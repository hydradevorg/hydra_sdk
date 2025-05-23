# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Volumes/BIGCODE/hydra_sdk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Volumes/BIGCODE/hydra_sdk/build

# Include any dependencies generated for this target.
include examples/lmvs_example/CMakeFiles/lmvs_example.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/lmvs_example/CMakeFiles/lmvs_example.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/lmvs_example/CMakeFiles/lmvs_example.dir/progress.make

# Include the compile flags for this target's objects.
include examples/lmvs_example/CMakeFiles/lmvs_example.dir/flags.make

examples/lmvs_example/CMakeFiles/lmvs_example.dir/codegen:
.PHONY : examples/lmvs_example/CMakeFiles/lmvs_example.dir/codegen

examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o: examples/lmvs_example/CMakeFiles/lmvs_example.dir/flags.make
examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o: /Volumes/BIGCODE/hydra_sdk/examples/lmvs_example.cpp
examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o: examples/lmvs_example/CMakeFiles/lmvs_example.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Volumes/BIGCODE/hydra_sdk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o"
	cd /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o -MF CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o.d -o CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o -c /Volumes/BIGCODE/hydra_sdk/examples/lmvs_example.cpp

examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.i"
	cd /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Volumes/BIGCODE/hydra_sdk/examples/lmvs_example.cpp > CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.i

examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.s"
	cd /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Volumes/BIGCODE/hydra_sdk/examples/lmvs_example.cpp -o CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.s

# Object files for target lmvs_example
lmvs_example_OBJECTS = \
"CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o"

# External object files for target lmvs_example
lmvs_example_EXTERNAL_OBJECTS =

examples/lmvs_example/lmvs_example: examples/lmvs_example/CMakeFiles/lmvs_example.dir/__/lmvs_example.cpp.o
examples/lmvs_example/lmvs_example: examples/lmvs_example/CMakeFiles/lmvs_example.dir/build.make
examples/lmvs_example/lmvs_example: src/lmvs/liblmvs.a
examples/lmvs_example/lmvs_example: src/hydra_math/libhydra_math.a
examples/lmvs_example/lmvs_example: src/hydra_vfs/libhydra_vfs.a
examples/lmvs_example/lmvs_example: src/hydra_crypto/libhydra_crypto.a
examples/lmvs_example/lmvs_example: /usr/local/lib/libbotan-3.7.dylib
examples/lmvs_example/lmvs_example: /usr/local/Cellar/openssl@3/3.4.1/lib/libssl.dylib
examples/lmvs_example/lmvs_example: /usr/local/Cellar/openssl@3/3.4.1/lib/libcrypto.dylib
examples/lmvs_example/lmvs_example: /usr/local/lib/libgmp.dylib
examples/lmvs_example/lmvs_example: lib/blake3/c/libblake3.a
examples/lmvs_example/lmvs_example: /usr/local/Cellar/openssl@3/3.4.1/lib/libcrypto.dylib
examples/lmvs_example/lmvs_example: src/hydra_qtm/libhydra_qtm.dylib
examples/lmvs_example/lmvs_example: /usr/local/Cellar/openssl@3/3.4.1/lib/libssl.dylib
examples/lmvs_example/lmvs_example: /usr/local/Cellar/openssl@3/3.4.1/lib/libcrypto.dylib
examples/lmvs_example/lmvs_example: examples/lmvs_example/CMakeFiles/lmvs_example.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Volumes/BIGCODE/hydra_sdk/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable lmvs_example"
	cd /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lmvs_example.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/lmvs_example/CMakeFiles/lmvs_example.dir/build: examples/lmvs_example/lmvs_example
.PHONY : examples/lmvs_example/CMakeFiles/lmvs_example.dir/build

examples/lmvs_example/CMakeFiles/lmvs_example.dir/clean:
	cd /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example && $(CMAKE_COMMAND) -P CMakeFiles/lmvs_example.dir/cmake_clean.cmake
.PHONY : examples/lmvs_example/CMakeFiles/lmvs_example.dir/clean

examples/lmvs_example/CMakeFiles/lmvs_example.dir/depend:
	cd /Volumes/BIGCODE/hydra_sdk/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Volumes/BIGCODE/hydra_sdk /Volumes/BIGCODE/hydra_sdk/examples/lmvs_example /Volumes/BIGCODE/hydra_sdk/build /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example /Volumes/BIGCODE/hydra_sdk/build/examples/lmvs_example/CMakeFiles/lmvs_example.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : examples/lmvs_example/CMakeFiles/lmvs_example.dir/depend

