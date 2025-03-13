cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(image_enhancer LANGUAGES CXX)

add_library(image_enhancer INTERFACE)
target_include_directories(image_enhancer INTERFACE include)
target_compile_features(image_enhancer INTERFACE cxx_std_17)
