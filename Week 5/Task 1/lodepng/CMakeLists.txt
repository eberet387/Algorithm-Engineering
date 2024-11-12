cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(lodepng LANGUAGES CXX)

set(LODEPNG_SOURCES lodepng.cpp lodepng.h)

add_library(lodepng STATIC ${LODEPNG_SOURCES})

# PUBLIC -> targets that link to this directory get that include directory
target_include_directories(lodepng PUBLIC ./)