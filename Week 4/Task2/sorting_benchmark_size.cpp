#include <iostream>
#include <vector>
#include <limits>
#include <omp.h>
#include <parallel/algorithm>
#include <fstream>

// compute the average of two integers without overflow
inline int64_t average(int64_t a, int64_t b) {
    return (a & b) + ((a ^ b) >> 1);
}

// partitioning function for quicksort
inline int64_t
partition(int64_t *arr, int64_t left, int64_t right, int64_t pivot, int64_t &smallest, int64_t &biggest) {
    int64_t *left_ptr = &arr[left];
    int64_t *right_ptr = &arr[right];
    while (left_ptr < right_ptr) {
        smallest = (*left_ptr < smallest) ? *left_ptr : smallest;
        biggest = (*left_ptr > biggest) ? *left_ptr : biggest;
        if (*left_ptr > pivot) {
            --right_ptr;
            std::swap(*left_ptr, *right_ptr);
        } else {
            ++left_ptr;
        }
    }
    return left_ptr - arr;
}

inline void insertion_sort(int64_t *arr, int64_t left, int64_t right) {
    for (int64_t i = left + 1; i <= right; i++) {
        int64_t key = arr[i];
        int64_t j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// the core recursive quicksort function
void qs_core(int64_t *arr, int64_t left, int64_t right, const int64_t pivot) {
    if (right - left < 32) {
        insertion_sort(arr, left, right);
        return;
    }

    int64_t smallest = std::numeric_limits<int64_t>::max();
    int64_t biggest = std::numeric_limits<int64_t>::min();
    int64_t bound = partition(arr, left, right + 1, pivot, smallest, biggest);

    if (smallest == biggest)
        return;

#pragma omp task final(bound - left < 10000)
    qs_core(arr, left, bound - 1, average(smallest, pivot));
    qs_core(arr, bound, right, average(pivot, biggest));
}

// wrapper for the quicksort function
void min_max_quicksort(int64_t *arr, int64_t n, int num_threads = omp_get_max_threads()) {
#pragma omp parallel num_threads(num_threads)
#pragma omp single nowait
    qs_core(arr, 0, n - 1, n > 0 ? arr[average(0, n - 1)] : 0);
}

// class for generating random numbers
class Xoroshiro128Plus {
    uint64_t state[2]{};

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

public:
    explicit Xoroshiro128Plus(uint64_t seed = 0) {
        state[0] = (12345678901234567 + seed) | 0b1001000010000001000100101000000110010010100000011001001010000001ULL;
        state[1] = (98765432109876543 + seed) | 0b0100000011001100100000011001001010000000100100101000000110010010ULL;
        for(int i = 0; i < 10; i++){operator()();}
    }

    uint64_t operator()() {
        const uint64_t s0 = state[0];
        uint64_t s1 = state[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        state[1] = rotl(s1, 37);
        return result;
    }
};

bool verify_qs_correctness(int64_t size) {
    Xoroshiro128Plus generator(size);
    std::vector<int64_t> data(size);
    for (int64_t i = 0; i < size; ++i) {
        data[i] = generator();
    }
    std::vector<int64_t> data_copy = data;  // Duplicate for std::sort
    min_max_quicksort(&data[0], size);
    std::sort(data_copy.begin(), data_copy.end());
    return data == data_copy;  // check if arrays are equal
}

int main() {
    // test correctness of min_max_quicksort
    const std::vector<int64_t> sizes = {0, 1, 23, 133, 1777, 57462, 786453};
    for (const auto &size: sizes) {
        if (!verify_qs_correctness(size)) {
            std::cout << "min_max_quicksort is incorrect for size " << size << "!\n";
            return 1;
        }
    }

    // threads used by min_max_quicksort and __gnu_parallel::sort
    omp_set_num_threads(16); // 12500H has 12 cores, but through hyperthreading supports up to 16 threads

    std::ofstream file("output.csv");
    file << "size,std:sort_time,min_max_quicksort_time,min_max_quicksort_speedup,__gnu_parallel_time,__gnu_parallel_speedup\n";

    for (int i = 0; i <= 30; i++) {
        int64_t SIZE = 100000 * pow(10, i/9);
        
        SIZE = SIZE * ((i % 9) + 1);
        
        std::vector<int64_t> data(SIZE);

        uint64_t seed = 3;
        Xoroshiro128Plus generator(seed);

        for (int64_t i = 0; i < SIZE; ++i) {
            data[i] = (int64_t) generator();
        }

        std::vector<int64_t> data_copy = data; // duplicate data for std:sort
        std::vector<int64_t> data_copy_min_max = data;  // duplicate data for min_max_quicksort
        std::vector<int64_t> data_copy_parallel = data;  // duplicate data for __gnu_parallel::sort

        // measure std::sort
        double start = omp_get_wtime();
        std::sort(data_copy.begin(), data_copy.end());
        double end = omp_get_wtime();
        double time_std_sort = end - start;

        // measure min_max_quicksort
        start = omp_get_wtime();
        min_max_quicksort(&data_copy_min_max[0], SIZE);
        end = omp_get_wtime();
        double time_min_max_quicksort = end - start;

        // measure __gnu_parallel::sort
        start = omp_get_wtime();
        __gnu_parallel::sort(data_copy_parallel.begin(), data_copy_parallel.end());
        end = omp_get_wtime();
        double time_gnu_parallel = end - start;

        std::cout << SIZE << "," << time_std_sort << "," << time_min_max_quicksort << "," << time_std_sort / time_min_max_quicksort
             << "," << time_gnu_parallel << "," << time_std_sort / time_gnu_parallel << "\n";

        file << SIZE << "," << time_std_sort << "," << time_min_max_quicksort << "," << time_std_sort / time_min_max_quicksort
             << "," << time_gnu_parallel << "," << time_std_sort / time_gnu_parallel << "\n";
    }

    file.close();
   
}