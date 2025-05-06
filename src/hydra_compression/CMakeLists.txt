cmake_minimum_required(VERSION 3.20)
project(hydra_compression LANGUAGES CXX)

# C++ standard and AppleClang fix
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
  add_compile_options(-stdlib=libc++)
  add_link_options(-stdlib=libc++)
endif()

# Visibility default for shared libs
add_compile_definitions(HYDRA_COMPRESSION_API_EXPORT)
add_compile_options(-fvisibility=hidden)

# === TVC SHARED LIBRARY ===
file(GLOB_RECURSE TVC_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tvc/*.cpp
)
if(TVC_SRC STREQUAL "")
  message(STATUS "No TVC source files found, creating stub library")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tvc_stub.cpp "// Stub file for TVC library\n")
  set(TVC_SRC ${CMAKE_CURRENT_BINARY_DIR}/tvc_stub.cpp)
endif()

add_library(hydra_compression_tvc SHARED ${TVC_SRC})
target_include_directories(hydra_compression_tvc PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/hydra_compression
)
target_compile_features(hydra_compression_tvc PUBLIC cxx_std_20)

# === OST SHARED LIBRARY ===
file(GLOB_RECURSE OST_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ost/*.cpp
)
if(OST_SRC STREQUAL "")
  message(STATUS "No OST source files found, creating stub library")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ost_stub.cpp "// Stub file for OST library\n")
  set(OST_SRC ${CMAKE_CURRENT_BINARY_DIR}/ost_stub.cpp)
endif()

add_library(hydra_compression_ost SHARED ${OST_SRC})
target_include_directories(hydra_compression_ost PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/hydra_compression
)
target_compile_features(hydra_compression_ost PUBLIC cxx_std_20)

# === COMBINED INTERFACE LIBRARY ===
# Create an interface library that combines both TVC and OST
add_library(hydra_compression INTERFACE)
target_link_libraries(hydra_compression INTERFACE
    hydra_compression_tvc
    hydra_compression_ost
)
target_include_directories(hydra_compression INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/hydra_compression
)

# === TESTING ===
enable_testing()
find_package(GTest QUIET)

if(GTest_FOUND)
  # Add the TVC implementation source files directly to the test executable
  add_executable(tvc_test tests/tvc_test.cpp ${TVC_SRC})
  target_include_directories(tvc_test PRIVATE ${CMAKE_SOURCE_DIR}/include)
  target_link_libraries(tvc_test PRIVATE GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(tvc_test)
else()
  message(STATUS "GTest not found, skipping tests for hydra_compression")
endif()
