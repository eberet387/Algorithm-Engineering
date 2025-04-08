cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(image_enhancer_implementation LANGUAGES CXX)

# add a library to the project using the specified source files
add_library(image_enhancer_implementation INTERFACE)

target_include_directories(image_enhancer_implementation INTERFACE include)
target_compile_features(image_enhancer_implementation INTERFACE cxx_std_17)
