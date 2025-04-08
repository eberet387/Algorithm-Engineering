cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(build_image_enhancer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler specific extensions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# search for OpenMP, REQUIRED -> fatal error if not found
find_package(OpenMP REQUIRED)
add_subdirectory(catch)
add_subdirectory(stb)
add_subdirectory(image_enhancer_implementation)

# create executable with name image_enhancer
add_executable(image_enhancer image_enhancer.cpp)
add_executable(image_enhancer_tests catch_tests_image_enhancer.cpp)

# link to the imported target provided by the FindOpenMP module
target_link_libraries(image_enhancer PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(image_enhancer PUBLIC catch)
target_link_libraries(image_enhancer PUBLIC stb)
target_link_libraries(image_enhancer PUBLIC image_enhancer_implementation)

target_link_libraries(image_enhancer_tests PUBLIC catch)
target_link_libraries(image_enhancer_tests PUBLIC image_enhancer_implementation)
# Check the compiler being used
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(image_enhancer PUBLIC -fopenmp -O2 -march=native -ffast-math -std=c++17)
elseif(MSVC)
    target_compile_options(image_enhancer PUBLIC /openmp)
    target_compile_options(image_enhancer PUBLIC /O2)  # Equivalent to GCC's -O2 for optimization
    target_compile_options(image_enhancer PUBLIC /arch:AVX)  # Enable AVX support
    target_compile_options(image_enhancer PUBLIC /fp:fast)  # Equivalent to GCC's -ffast-math for floating-point math optimization
    target_compile_options(image_enhancer PUBLIC /std:c++17)  # Enable C++17 standard

    # Disable runtime checks to avoid conflict with /O2
    target_compile_options(image_enhancer PUBLIC /RTC-)
endif()

add_custom_target(copy_tests_folder ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_BINARY_DIR}/tests
)

add_custom_target(copy_samples_folder ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/samples
    ${CMAKE_BINARY_DIR}/samples
)

add_custom_target(copy_output_folder ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/output
    ${CMAKE_BINARY_DIR}/output
)
# create executable with all tests
#add_executable(image_enhancer_tests catch_image_enhancer.cpp)


