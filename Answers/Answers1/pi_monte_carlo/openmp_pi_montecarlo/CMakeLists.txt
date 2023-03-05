cmake_minimum_required(VERSION 3.23)
project(openmp_pi_montecarlo)

set(CMAKE_CXX_STANDARD 14)
set(GCC_COVERAGE_COMPILE_FLAGS "-fopenmp -Ofast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(openmp_pi_montecarlo main.cpp)
