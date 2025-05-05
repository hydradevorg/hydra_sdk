cmake_minimum_required(VERSION 3.16)
project(hydra_math VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find source files
file(GLOB_RECURSE HYDRA_MATH_SRC CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Remove problematic bigint.cpp file and use stub instead
list(FILTER HYDRA_MATH_SRC EXCLUDE REGEX ".*bigint\.cpp$")
list(APPEND HYDRA_MATH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/bigint_stub.cpp)

# Check if we found any source files
if(HYDRA_MATH_SRC STREQUAL "")
    message(STATUS "No hydra_math source files found, creating stub library")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/hydra_math_stub.cpp "// Stub file for hydra_math library\n")
    set(HYDRA_MATH_SRC ${CMAKE_CURRENT_BINARY_DIR}/hydra_math_stub.cpp)
endif()

# Create the library
add_library(hydra_math STATIC ${HYDRA_MATH_SRC})

# Include directories
target_include_directories(hydra_math PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/hydra_math
)

# Find GMP
find_package(GMP QUIET)
if(GMP_FOUND)
    target_link_libraries(hydra_math PUBLIC GMP::GMP)
else()
    message(STATUS "GMP not found, linking with gmp directly")
    target_link_libraries(hydra_math PUBLIC gmp)
    # Add GMP include directory if it exists in standard locations
    if(EXISTS /usr/include/gmp.h)
        target_include_directories(hydra_math PUBLIC /usr/include)
    elseif(EXISTS /usr/local/include/gmp.h)
        target_include_directories(hydra_math PUBLIC /usr/local/include)
    elseif(EXISTS /opt/homebrew/include/gmp.h)
        target_include_directories(hydra_math PUBLIC /opt/homebrew/include)
    else()
        message(WARNING "Could not find gmp.h, you may need to install GMP development files")
    endif()
endif()

# Find Eigen
find_package(Eigen3 QUIET)
if(Eigen3_FOUND)
    target_link_libraries(hydra_math PUBLIC Eigen3::Eigen)
else()
    # Check if Eigen is in lib/eigen
    if(EXISTS ${CMAKE_SOURCE_DIR}/lib/eigen/Eigen/Dense)
        target_include_directories(hydra_math PUBLIC ${CMAKE_SOURCE_DIR}/lib/eigen)
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/vendors/eigen/Eigen/Dense)
        target_include_directories(hydra_math PUBLIC ${CMAKE_SOURCE_DIR}/vendors/eigen)
    elseif(EXISTS /usr/include/eigen3/Eigen/Dense)
        target_include_directories(hydra_math PUBLIC /usr/include/eigen3)
    elseif(EXISTS /usr/local/include/eigen3/Eigen/Dense)
        target_include_directories(hydra_math PUBLIC /usr/local/include/eigen3)
    elseif(EXISTS /opt/homebrew/include/eigen3/Eigen/Dense)
        target_include_directories(hydra_math PUBLIC /opt/homebrew/include/eigen3)
    else()
        message(WARNING "Could not find Eigen, you may need to install Eigen3 development files")
    endif()
endif()

# Compile features
target_compile_features(hydra_math PUBLIC cxx_std_20)
