cmake_minimum_required(VERSION 3.16)
project(msrs_sample)

include_directories(include)
add_executable(msrs_sample src/msrs_sample.cpp src/msr_cpu_file.cpp)