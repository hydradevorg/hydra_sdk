# This file is included from the main CMakeLists.txt
# It's not meant to be built separately

# Example executables
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example hydra_vfs hydra_crypto)

add_executable(secure_file_manager secure_file_manager_updated.cpp)
target_link_libraries(secure_file_manager hydra_vfs hydra_crypto)

# Instructions for running
message(STATUS "After building, run examples with:")
message(STATUS "  ./simple_example")
message(STATUS "  ./secure_file_manager")
