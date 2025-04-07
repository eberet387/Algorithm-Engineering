cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(image_enhancer_implementation LANGUAGES CXX)

set(IMAGE_ENHANCER_SOURCES include/implementation.cpp include/implementation.hpp)

add_library(image_enhancer_implementation STATIC ${IMAGE_ENHANCER_SOURCES})

target_include_directories(image_enhancer_implementation INTERFACE include)
target_compile_features(image_enhancer_implementation INTERFACE cxx_std_17)
