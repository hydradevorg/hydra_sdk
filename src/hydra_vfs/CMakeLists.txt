cmake_minimum_required(VERSION 3.10)
project(hydra_sks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Source files for the hydra_vfs library
set(HYDRA_VFS_SOURCES
    src/vfs.cpp
    src/memory_vfs.cpp
    src/persistent_vfs.cpp
    src/encrypted_vfs.cpp
    src/container_vfs.cpp
    src/container_vfs_factory.cpp
    src/container_utils.cpp
    src/container_metadata.cpp
    src/resource_monitor.cpp
    src/macos_security_module.cpp
    src/container_file.cpp
    src/path_utils.cpp
    src/container_path_handler.cpp
    src/container_vfs_fixes.cpp
)

# Create the static hydra_vfs library
add_library(hydra_vfs STATIC ${HYDRA_VFS_SOURCES})
target_include_directories(hydra_vfs PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendors
)
target_link_libraries(hydra_vfs PUBLIC OpenSSL::Crypto hydra_crypto)

# Create the shared hydra_vfs library
add_library(hydra_vfs_shared SHARED ${HYDRA_VFS_SOURCES})
target_include_directories(hydra_vfs_shared PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendors
)
target_link_libraries(hydra_vfs_shared PUBLIC OpenSSL::Crypto hydra_crypto)
set_target_properties(hydra_vfs_shared PROPERTIES 
    OUTPUT_NAME hydra_vfs
    VERSION 1.0.0
    SOVERSION 1
)

# Main executable
add_executable(hydra_sks main.cpp)
target_link_libraries(hydra_sks PRIVATE hydra_vfs)

# Tests executable
add_executable(vfs_tests tests/vfs_tests.cpp)
target_link_libraries(vfs_tests PRIVATE hydra_vfs)
