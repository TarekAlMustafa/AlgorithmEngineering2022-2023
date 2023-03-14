cmake_minimum_required(VERSION 3.23)
project(ImageEnhancerProject)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler specific extensions
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -std=c++14 -fopenmp -ffast-math -march=native")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)



add_executable(ImageEnhancerProject main.cpp)
