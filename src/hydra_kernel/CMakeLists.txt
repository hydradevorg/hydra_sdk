cmake_minimum_required(VERSION 3.16)
project(hydra_kernel VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Source files
set(HYDRA_KERNEL_SOURCES
${CMAKE_CURRENT_SOURCE_DIR}/src/process.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/kernel.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/network.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/ipc.cpp
)

# Headers
set(HYDRA_KERNEL_HEADERS
${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_kernel/process.h
${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_kernel/kernel.h
${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_kernel/network.h
${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_kernel/ipc.h
)

# Library target
add_library(hydra_kernel ${HYDRA_KERNEL_SOURCES} ${HYDRA_KERNEL_HEADERS})

# Include directories
target_include_directories(hydra_kernel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(hydra_kernel
    PUBLIC
        hydra_vfs
        hydra_common
        Threads::Threads
)

# Platform-specific dependencies
if(WIN32)
    target_link_libraries(hydra_kernel PUBLIC ws2_32)
endif()

# Install targets
install(TARGETS hydra_kernel
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HYDRA_KERNEL_HEADERS}
    DESTINATION include/hydra_kernel
)

# Set output directory
set_target_properties(hydra_kernel PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
