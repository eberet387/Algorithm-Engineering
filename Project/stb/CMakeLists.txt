cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(stb LANGUAGES CXX)

# add a library to the project using the specified source files
add_library(stb INTERFACE)

target_include_directories(stb INTERFACE include)
target_compile_features(stb INTERFACE cxx_std_17)