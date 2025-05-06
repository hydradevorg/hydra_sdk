cmake_minimum_required(VERSION 3.16)
project(kernel_example VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Add executable
add_executable(kernel_example kernel_example.cpp)

# Include directories
target_include_directories(kernel_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(kernel_example
    PRIVATE
        hydra_vfs
        hydra_kernel
        Threads::Threads
)

# Platform-specific dependencies
if(WIN32)
    target_link_libraries(kernel_example PRIVATE ws2_32)
endif()

# Set output directory
set_target_properties(kernel_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
