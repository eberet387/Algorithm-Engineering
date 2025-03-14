cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(quantpng LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler specific extensions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# search for OpenMP, REQUIRED -> fatal error if not found
find_package(OpenMP REQUIRED)
add_subdirectory(lodepng)
add_subdirectory(quantize)
# create executable with name quantpng
add_executable(quantpng main.cpp)

# link to the imported target provided by the FindOpenMP module
target_link_libraries(quantpng PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(quantpng PUBLIC lodepng)
target_link_libraries(quantpng PUBLIC quantize)