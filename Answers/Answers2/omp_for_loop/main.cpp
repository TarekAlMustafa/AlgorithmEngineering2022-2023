#include <iostream>
#include <omp.h>
#include <random>
using namespace std;
int main() {
    int seed = 0;
    default_random_engine re{seed};
    uniform_real_distribution<double> zero_to_one{0.0, 1.0};
    int n = 100000000; // amount of points to generate
    int counter = 0; // counter for points in the first quarter of a unit circle
    int counter_local = 0; // we actually have to move this local counter to up here compared to the previous program
    omp_set_num_threads(2);
    auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine


#pragma omp parallel
    {
#pragma omp for // i thought the omp for would save more space than it actually does, however it saves a lot of trouble in the for loop definition so thats nice
        for (int i = 0; i < n; ++i) {
            auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
            auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
            if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
                ++counter_local; // increment locally
            }
        }
#pragma omp atomic
        counter += counter_local; // add local counters together
        // I tried skipping this step and removing the local counter, program output becomes pi/num_threads
        };

/*#pragma omp parallel
    {
        int num_threads = omp_get_num_threads(); //for loop instances
        int thread_id = omp_get_thread_num();
        int counter_local = 0; // local counter for every thread
        for (int i = thread_id; i < n; i += num_threads) {
            auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
            auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
            if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
                ++counter_local; // increment locally
            }
        }
#pragma omp atomic
            counter += counter_local; // add local counters together
        };
        */

// we get the same 'counter' variable from the for loop above so we can keep the code down here the same
    auto run_time = omp_get_wtime() - start_time;
    auto pi = 4 * (double(counter) / n);
    cout << "pi: " << pi << endl;
    cout << "run_time: " << run_time << " s" << endl;
    cout << "n: " << n << endl; }