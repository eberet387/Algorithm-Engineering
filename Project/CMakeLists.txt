cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(build_image_enhancer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF) # disable compiler specific extensions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# search for OpenMP, REQUIRED -> fatal error if not found
find_package(OpenMP REQUIRED)
add_subdirectory(catch)
add_subdirectory(stb)
add_subdirectory(image_enhancer)

# create executable with name image_enhancer
add_executable(main main.cpp)
add_executable(image_enhancer_tests catch_tests_image_enhancer.cpp)

# link to the imported target provided by the FindOpenMP module
target_link_libraries(main PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(main PUBLIC catch)
target_link_libraries(main PUBLIC stb)
target_link_libraries(main PUBLIC image_enhancer)

target_link_libraries(image_enhancer_tests PUBLIC catch)
target_link_libraries(image_enhancer_tests PUBLIC image_enhancer)
# Check the compiler being used
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang: Use -fopenmp
    target_compile_options(main PRIVATE -fopenmp)
    target_compile_options(main PRIVATE -O2 -march=native -mavx -ffast-math -std=c++17)
elseif(MSVC)
    target_compile_options(main PRIVATE /openmp)
    target_compile_options(main PRIVATE /O2)  # Equivalent to GCC's -O2 for optimization
    target_compile_options(main PRIVATE /arch:AVX)  # Enable AVX support
    target_compile_options(main PRIVATE /fp:fast)  # Equivalent to GCC's -ffast-math for floating-point math optimization
    target_compile_options(main PRIVATE /std:c++17)  # Enable C++17 standard

    # Disable runtime checks to avoid conflict with /O2
    target_compile_options(main PRIVATE /RTC-)
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
#add_executable(image_enhancer_tests catch_main.cpp catch_tests_fizzbuzz.cpp)

# link libraries fizzbuzz and catch to executable
#target_link_libraries(image_enhancer_tests PRIVATE fizzbuzz PRIVATE catch)

