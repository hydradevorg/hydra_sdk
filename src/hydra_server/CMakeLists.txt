cmake_minimum_required(VERSION 3.16)
project(hydra_server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(json)
endif()

# Add cpp-httplib
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.1
)
FetchContent_MakeAvailable(httplib)

# Source files
set(HYDRA_SERVER_SOURCES
    api_server.cpp
    api_example.cpp
)

# Build the server library
add_library(hydra_server STATIC ${HYDRA_SERVER_SOURCES})

# Include directories
target_include_directories(hydra_server 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${httplib_SOURCE_DIR}
)

# Link libraries
target_link_libraries(hydra_server
    PUBLIC
        hydra_crypto
        hydra_vfs
    PRIVATE
        nlohmann_json::nlohmann_json
)

# Build the example executable
add_executable(api_server_example api_example.cpp api_server.cpp)
target_include_directories(api_server_example PRIVATE ${httplib_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(api_server_example PRIVATE hydra_crypto hydra_vfs nlohmann_json::nlohmann_json)

# Install
install(TARGETS hydra_server
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
