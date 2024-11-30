#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  unsigned int seed = 0;
  default_random_engine re{seed};
  uniform_real_distribution<double> zero_to_one{0.0, 1.0};

  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine
  omp_set_num_threads(4);

#pragma omp parallel 
{
  int num_threads = omp_get_num_threads();
  int local_counter = 0;
      // compute n/num_threads points per thread and test if they lie within the first quadrant of a unit circle
    for (int i = 0; i < n; i += num_threads) {
      auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
      auto y = zero_to_one(re); // generate random number between 0.0 and 1.0

      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        local_counter++;
      }
    }

#pragma omp atomic 
    counter+= local_counter;
  }

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; 
}

/*
int n = 100000000;

non-parallelized code:
  pi: 3.14176
  run_time: 25.691 s
  n: 100000000

parallelized code: (2 threads)
  pi: 3.14168
  run_time: 15.003 s
  n: 100000000

parallelized code: (4 threads)
  pi: 3.13846
  run_time: 10.744 s
  n: 100000000
*/