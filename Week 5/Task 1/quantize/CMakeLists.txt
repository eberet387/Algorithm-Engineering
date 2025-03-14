cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(quantize LANGUAGES C)

set(IMAGE_SOURCES blur.c blur.h kmeans.c kmeans.h 
    libimagequant.c libimagequant.h mediancut.c mediancut.h 
    mempool.c mempool.h nearest.c nearest.h pam.c pam.h)

add_library(quantize STATIC ${IMAGE_SOURCES})

# PUBLIC -> targets that link to this directory get that include directory
target_include_directories(quantize PUBLIC ./)