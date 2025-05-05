cmake_minimum_required(VERSION 3.16)
project(hydra_qtm_vm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for source files
file(GLOB LIB_SRC CONFIGURE_DEPENDS src/*.cpp)

if(LIB_SRC STREQUAL "")
  # No source files found, create a stub file
  message(STATUS "No quantum simulator source files found, creating stub library")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/quantum_sim_stub.cpp "// Stub file for quantum simulator library\n")
  set(LIB_SRC ${CMAKE_CURRENT_BINARY_DIR}/quantum_sim_stub.cpp)
endif()

# Create the library
add_library(quantum_sim STATIC ${LIB_SRC})
target_include_directories(quantum_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

# Try to find Eigen
find_package(Eigen3 QUIET)
if(Eigen3_FOUND)
  target_link_libraries(quantum_sim PUBLIC Eigen3::Eigen)
else()
  # Fallback to local eigen if available
  if(EXISTS ${CMAKE_SOURCE_DIR}/lib/eigen)
    target_include_directories(quantum_sim PUBLIC ${CMAKE_SOURCE_DIR}/lib/eigen)
  else()
    message(STATUS "Eigen not found, some quantum simulator features may be limited")
  endif()
endif()

target_compile_features(quantum_sim PUBLIC cxx_std_20)
