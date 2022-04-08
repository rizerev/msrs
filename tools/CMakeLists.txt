cmake_minimum_required(VERSION 3.16)
project(toolset)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-static -Wall -Wextra -Wpedantic -Wconversion)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(shared/include/)

add_subdirectory(msrs_ls)
add_subdirectory(msrs_sample)
add_subdirectory(msrs_detect)

target_link_libraries(msrs_ls -static -static-libgcc -static-libstdc++)
target_link_libraries(msrs_sample -static -static-libgcc -static-libstdc++)
target_link_libraries(msrs_detect -static -static-libgcc -static-libstdc++)
