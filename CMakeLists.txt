cmake_minimum_required(VERSION 3.16)
project(brainf++k)

set(CMAKE_CXX_STANDARD 20)

add_executable(brainf++k main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native -mtune=native" )