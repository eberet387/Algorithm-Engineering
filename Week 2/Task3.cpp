#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  int num_steps = 100000000; // number of rectangles
  double width = 1.0 / double(num_steps); // width of a rectangle
  double sum = 0.0; // for summing up all heights of rectangles

  double start_time = omp_get_wtime(); // wall clock time in seconds
  omp_set_num_threads(16);
#pragma omp parallel
  {
    double local_sum = 0.0;
#pragma omp for
    for (int i = 0; i < num_steps; i++) {
    double x = (i + 0.5) * width; // midpoint
    local_sum = local_sum + (1.0 / (1.0 + x * x)); // add new height of a rectangle
    }

#pragma omp atomic
  sum += local_sum;  
}

  double pi = sum * 4 * width; // compute pi
  double run_time = omp_get_wtime() - start_time;

  cout << "pi with " << num_steps << " steps is " << setprecision(17)
       << pi << " in " << setprecision(6) << run_time << " seconds\n";
}

/*
before parallelization:
pi with 100000000 steps is 3.1415926535904264 in 0.247 seconds

after parallelization (4 Threads):
pi with 100000000 steps is 3.1415926535896825 in 0.0939999 seconds
*/