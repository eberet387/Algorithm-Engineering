# Algorithm Engineering 2025

## 📂 Week 1-12 Folders  
Contains answers to the weekly tasks.

## 📂 Project Paper Folder  
Includes the **Project Paper** as a PDF.

## 📂 Project Folder  
Contains the **Image Enhancement Project**.

### 🛠️ Build Instructions  
1. Navigate to the Project folder:  
   ```sh
   cd Project
   ```
2. Generate build files using CMake:  
   ```sh
   cmake -B build .
   ```
3. Navigate to the build folder:  
   ```sh
   cd build
   ```
4. Build the project:  
   ```sh
   cmake --build .
   ```

### ⚠️ Troubleshooting  
- **CMake is using MSVC instead of GCC / Clang (MinGW)?**  
  Force CMake to use G++ by running:  
  ```sh
  cmake -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
  ```
  Or manually set the CMake env variables to a different compiler.

### 🚀 Usage  
Run the Image Enhancer with:  
```sh
./main <path_to_image> <parameters>
```
For help, use:  
```sh
./main -h
```
Run the unit tests with:
./catch_tests_image_enhancer
