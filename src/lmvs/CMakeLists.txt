cmake_minimum_required(VERSION 3.10)
project(lmvs VERSION 0.1.0 LANGUAGES CXX C)

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
find_package(GMP REQUIRED)

# Define GMP target if not defined
if(NOT TARGET GMP::GMP)
    add_library(GMP::GMP UNKNOWN IMPORTED)
    set_target_properties(GMP::GMP PROPERTIES
        IMPORTED_LOCATION "${GMP_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIRS}"
    )
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../build_wasm/mock/include
)

# Source files
file(GLOB LMVS_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

file(GLOB LMVS_SECURITY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/security/*.cpp"
)

file(GLOB LMVS_P2P_VFS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/p2p_vfs/*.cpp"
)

# Create library
add_library(lmvs STATIC
    ${LMVS_CORE_SOURCES}
    ${LMVS_SECURITY_SOURCES}
    ${LMVS_P2P_VFS_SOURCES}
)

# Link dependencies
target_link_libraries(lmvs
    PUBLIC
    ${OPENSSL_LIBRARIES}
    GMP::GMP
)

# Create an interface library called hydra_lmvs that links to lmvs
add_library(hydra_lmvs INTERFACE)
target_link_libraries(hydra_lmvs INTERFACE lmvs)
target_include_directories(hydra_lmvs INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/lmvs
)

# Installation
install(TARGETS lmvs hydra_lmvs
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../include/lmvs
        DESTINATION include)
