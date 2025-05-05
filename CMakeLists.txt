cmake_minimum_required(VERSION 3.16)
project(hydra_sdk VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External dependencies
find_package(yaml-cpp REQUIRED)

# Add custom find modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
add_subdirectory(lib/blake3)


# Add subdirectories for all components
add_subdirectory(src/hydra_common)
add_subdirectory(src/hydra_math)
add_subdirectory(src/hydra_vfs)
add_subdirectory(src/hydra_crypto)
add_subdirectory(src/hydra_cli)
add_subdirectory(src/hydra_kernel)
add_subdirectory(src/hydra_server)
add_subdirectory(src/hydra_compression)
add_subdirectory(src/hydra_qtm_vm)

# Add examples
add_subdirectory(examples/vfs_example)
add_subdirectory(examples/kernel_example)

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
