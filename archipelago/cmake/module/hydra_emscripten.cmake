# hydra_emscripten.cmake
# Emscripten toolchain file for Hydra SDK

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compiler
set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

# Where is the target environment
set(CMAKE_FIND_ROOT_PATH $ENV{EMSDK}/upstream/emscripten)

# Search for programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Specific flags for WebAssembly compilation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s USE_ES6_IMPORT_META=0 -s EXPORT_ES6=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORT_NAME=HydraModule")

# Add support for C++20 features
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Disable exceptions by default to reduce code size
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
