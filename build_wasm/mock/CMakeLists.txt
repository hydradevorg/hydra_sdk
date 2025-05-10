cmake_minimum_required(VERSION 3.10)
project(gmp_mock VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the source files
add_library(gmp STATIC
    include/gmp_mock.cpp
)

# Set include directories
target_include_directories(gmp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install the library
install(TARGETS gmp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES include/gmp.h
    DESTINATION include
)
