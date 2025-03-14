cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(fizzbuzz LANGUAGES CXX)

# add a library to the project using the specified source files
add_library(fizzbuzz STATIC include/fizzbuzz.h src/fizzbuzz.cpp)

# PUBLIC -> targets that link to this target get that include directory
target_include_directories(fizzbuzz PUBLIC include)

