#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

double zero_to_one(unsigned int *seed) {
    *seed = (510397509 * (*seed) + 820750875) % (1 << 24);
    return ((double) (*seed)) / (1 << 24);
}

int main() {
  // default_random_engine re{seed};
  // uniform_real_distribution<double> zero_to_one{0.0, 1.0};
  
  int n = 100000000; // number of points to generate
  int counter = 0; // counter for points lying in the first quadrant of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine
  omp_set_num_threads(4);
  int hits = 0;

#pragma omp parallel 
{
  unsigned int seed = omp_get_thread_num() * 7452;
    #pragma omp for reduction(+:hits)
    for (int i = 0; i < n; i++) {
      auto x = zero_to_one(&seed); // generate random number between 0.0 and 1.0
      auto y = zero_to_one(&seed); // generate random number between 0.0 and 1.0

      if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
        hits++;
      }
    }
}
  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(hits) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; 
}
/*
int n = 100000000;

parallelized code from Week 1: (4 threads)
  pi: 3.13846
  run_time: 10.744 s
  n: 100000000

Changes made:
-> we do not use a random library for the computation of numbers between 0 to 1,
   each thread has its own seed now 
Why this was bad before:
   the random library is not thread safe, as after each random call, a hidden 
   state has to be updated for all threads, making this new approach way faster

-> using #pragma omp for to make the loop distribution easier 
Why this was bad before:
   if n % num_threads was not equal to 0, the result would have been wrong
   as there would be more for-loop calls than neccessary (more noticable on
   smaller n's)

-> using reduction(+:hits) instead of atomic variable updating
Why this was bad before:
   the performance gain isn't that noticable, but it results in cleaner code
   and less competition for the global hits variable, as reduction(+:hits)
   uses a more optimized approach of tree based reduction, which can be
   parallelized more efficiently (1:2:4:...:num_threads) instead of a
   single 1:n reduction where each thread has to wait until the variable
   has been updated 
   source: https://stackoverflow.com/questions/54186268/why-should-i-use-a-reduction-rather-than-an-atomic-variable
   
parallelized code now: (4 threads)
  pi: 3.14153
  run_time: 0.302 s
  n: 100000000
*/