cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(cpp_gpt_duplicate LANGUAGES CXX CUDA)

# Assuming CUDA Toolkit is installed properly
find_package(CUDAToolkit REQUIRED)

# Set Torch directory and find the package
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/libtorch")
set(CMAKE_CUDA_COMPILER "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.8")
set(CMAKE_CUDA_ARCHITECTURES 60 61 62 70 72 75 86)
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# Include directories for CUDA and LibTorch
include_directories(${CUDAToolkit_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})

# Add the executable
add_executable(cpp_gpt_duplicate cpp_gpt/main.cpp)  # Ensure all source files are listed here

# Link libraries
target_link_libraries(cpp_gpt_duplicate "${TORCH_LIBRARIES}" CUDA::cudart)

# Specify C++ standard
set_property(TARGET cpp_gpt_duplicate PROPERTY CXX_STANDARD 17)

if (MSVC)
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  add_custom_command(TARGET cpp_gpt_duplicate
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     $<TARGET_FILE_DIR:cpp_gpt_duplicate>)
endif (MSVC)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()
