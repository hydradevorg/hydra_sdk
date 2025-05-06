cmake_minimum_required(VERSION 3.10)
project(lmvs_examples VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Example executable
add_executable(lmvs_example lmvs_example.cpp)
target_link_libraries(lmvs_example lmvs)

# Installation
install(TARGETS lmvs_example
        RUNTIME DESTINATION bin)
