cmake_minimum_required(VERSION 3.16)
project(hydra_sdk VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# External dependencies
find_package(yaml-cpp REQUIRED)

# Find GMP
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARY NAMES gmp)
if(GMP_INCLUDE_DIR AND GMP_LIBRARY)
    message(STATUS "Found GMP: ${GMP_LIBRARY}")
    add_library(GMP::GMP UNKNOWN IMPORTED)
    set_target_properties(GMP::GMP PROPERTIES
        IMPORTED_LOCATION "${GMP_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
    )
else()
    message(FATAL_ERROR "GMP library not found")
endif()

# Add custom find modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
add_subdirectory(lib/blake3/c)


# Add subdirectories for all components
add_subdirectory(src/hydra_common)
add_subdirectory(src/hydra_math)
add_subdirectory(src/hydra_crypto) # Build hydra_crypto before hydra_vfs
add_subdirectory(src/hydra_vfs)
add_subdirectory(src/hydra_cli)
add_subdirectory(src/hydra_kernel)
add_subdirectory(src/hydra_server)
add_subdirectory(src/hydra_compression)
add_subdirectory(src/hydra_qzkp)
add_subdirectory(src/hydra_qtm)
add_subdirectory(src/lmvs) # Layered Matrix and Vector System
add_subdirectory(src/hydra_address) # Address generation system

# Add examples
add_subdirectory(examples/vfs_example)
add_subdirectory(examples/kernel_example)
add_subdirectory(examples/lmvs_example)
add_subdirectory(examples/lmvs_bigint_example)
add_subdirectory(examples/secure_vector_transport_example)
add_subdirectory(examples/p2p_vfs_example)
add_subdirectory(examples/p2p_secure_vfs_example)
add_subdirectory(examples/p2p_vfs_cat_example)
add_subdirectory(examples/qzkp_example)

# Address generation example
add_executable(address_gen_example examples/address_generation_example.cpp)
target_include_directories(address_gen_example PRIVATE include)
target_link_libraries(address_gen_example PRIVATE
    hydra_compression_tvc
    hydra_compression_ost
    hydra_address
    hydra_crypto
    hydra_math
    hydra_qzkp
    hydra_lmvs
    ${GMP_LIBRARY}
)

# Enable testing
enable_testing()

# Set the output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
