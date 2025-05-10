cmake_minimum_required(VERSION 3.10)
project(lmvs_wasm VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

# Add cmake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Botan REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${OPENSSL_INCLUDE_DIR}
    ${BOTAN_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build_wasm/mock/include
)

# Source files
file(GLOB LMVS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Create library
add_library(lmvs_wasm STATIC
    ${LMVS_SOURCES}
)

# Link dependencies
target_link_libraries(lmvs_wasm
    PUBLIC
    ${OPENSSL_LIBRARIES}
    ${BOTAN_LIBRARY}
)

# Create an interface library called hydra_lmvs that links to lmvs
add_library(hydra_lmvs INTERFACE)
target_link_libraries(hydra_lmvs INTERFACE lmvs_wasm)
target_include_directories(hydra_lmvs INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Installation
install(TARGETS lmvs_wasm hydra_lmvs
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lmvs
        DESTINATION include)
