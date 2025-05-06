cmake_minimum_required(VERSION 3.16)
project(hydra_cli VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find yaml-cpp for Docker-like container configuration
find_package(yaml-cpp REQUIRED)
if(NOT yaml-cpp_FOUND)
    message(STATUS "yaml-cpp not found via find_package, trying pkg-config")
    # Try to use pkg-config as a fallback
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(YAML_CPP yaml-cpp)
        if(YAML_CPP_FOUND)
            message(STATUS "Found yaml-cpp via pkg-config: ${YAML_CPP_LIBRARIES}")
        else()
            message(FATAL_ERROR "yaml-cpp library not found. Please install it using your package manager.")
        endif()
    else()
        message(FATAL_ERROR "Neither find_package nor pkg-config could find yaml-cpp. Please install it using your package manager.")
    endif()
endif()

# Add the yaml-cpp include directories
include_directories(${YAML_CPP_INCLUDE_DIRS})

# Source files for the CLI
set(HYDRA_CLI_SOURCES
    src/hydra_cli.cpp
    src/vfs_commands_container.cpp
    src/vfs_commands_list.cpp
    src/vfs_commands_put.cpp
    src/vfs_commands_cat.cpp
    src/vfs_commands_get.cpp
    src/vfs_commands_mkdir.cpp
    src/vfs_commands_stats.cpp
    src/vfs_commands_remove.cpp
    src/crypto_commands_sign.cpp
    src/crypto_commands_keygen.cpp
    src/crypto_commands_verify.cpp
    src/crypto_commands_decrypt.cpp
    src/crypto_commands_encrypt.cpp
    src/crypto_commands_keyinfo.cpp
    src/kernel_commands.cpp
    src/kernel_commands_run.cpp
)

# Headers for the CLI
set(HYDRA_CLI_HEADERS
    include/commands/command.h
    include/commands/vfs_commands.h
    include/commands/crypto_commands.h
    include/commands/kernel_commands.h
)

# Build the CLI executable
add_executable(hydra-cli ${HYDRA_CLI_SOURCES} ${HYDRA_CLI_HEADERS})

# Include directories
target_include_directories(hydra-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(hydra-cli
    PRIVATE
        hydra_vfs
        hydra_crypto
        hydra_kernel
        /usr/local/Cellar/yaml-cpp/0.8.0/lib/libyaml-cpp.dylib
)

# Install the CLI executable
install(TARGETS hydra-cli DESTINATION bin)

# Make the hydra-cli available at the top level build directory for easier testing
set_target_properties(hydra-cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
