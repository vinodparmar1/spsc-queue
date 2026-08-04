
cmake_minimum_required(VERSION 3.16)

project(spsc_queue VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# usage test
add_executable(spscq_test src/main.cpp)
target_include_directories(spscq_test PRIVATE include)
target_link_libraries(spscq_test PRIVATE pthread)

#benchmark test
find_package(benchmark REQUIRED)
add_executable(bench_spscq benchmarks/bench_spsc.cpp)
target_include_directories(bench_spscq PRIVATE include)
target_link_libraries(bench_spscq PRIVATE pthread benchmark::benchmark)
