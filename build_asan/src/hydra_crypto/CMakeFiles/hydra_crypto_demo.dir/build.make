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
CMAKE_BINARY_DIR = /Volumes/BIGCODE/hydra_sdk/build_asan

# Include any dependencies generated for this target.
include src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/compiler_depend.make

# Include the progress variables for this target.
include src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/progress.make

# Include the compile flags for this target's objects.
include src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/flags.make

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/codegen:
.PHONY : src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/codegen

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o: src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/flags.make
src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o: /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/main.cpp
src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o: src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Volumes/BIGCODE/hydra_sdk/build_asan/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o"
	cd /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o -MF CMakeFiles/hydra_crypto_demo.dir/main.cpp.o.d -o CMakeFiles/hydra_crypto_demo.dir/main.cpp.o -c /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/main.cpp

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/hydra_crypto_demo.dir/main.cpp.i"
	cd /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/main.cpp > CMakeFiles/hydra_crypto_demo.dir/main.cpp.i

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/hydra_crypto_demo.dir/main.cpp.s"
	cd /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto && /Library/Developer/CommandLineTools/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto/main.cpp -o CMakeFiles/hydra_crypto_demo.dir/main.cpp.s

# Object files for target hydra_crypto_demo
hydra_crypto_demo_OBJECTS = \
"CMakeFiles/hydra_crypto_demo.dir/main.cpp.o"

# External object files for target hydra_crypto_demo
hydra_crypto_demo_EXTERNAL_OBJECTS =

src/hydra_crypto/hydra_crypto_demo: src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/main.cpp.o
src/hydra_crypto/hydra_crypto_demo: src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/build.make
src/hydra_crypto/hydra_crypto_demo: src/hydra_crypto/libhydra_crypto.a
src/hydra_crypto/hydra_crypto_demo: /usr/local/lib/libbotan-3.7.dylib
src/hydra_crypto/hydra_crypto_demo: /usr/local/Cellar/openssl@3/3.4.1/lib/libssl.dylib
src/hydra_crypto/hydra_crypto_demo: /usr/local/Cellar/openssl@3/3.4.1/lib/libcrypto.dylib
src/hydra_crypto/hydra_crypto_demo: /usr/local/lib/libgmp.dylib
src/hydra_crypto/hydra_crypto_demo: lib/blake3/c/libblake3.a
src/hydra_crypto/hydra_crypto_demo: src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Volumes/BIGCODE/hydra_sdk/build_asan/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable hydra_crypto_demo"
	cd /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hydra_crypto_demo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/build: src/hydra_crypto/hydra_crypto_demo
.PHONY : src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/build

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/clean:
	cd /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto && $(CMAKE_COMMAND) -P CMakeFiles/hydra_crypto_demo.dir/cmake_clean.cmake
.PHONY : src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/clean

src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/depend:
	cd /Volumes/BIGCODE/hydra_sdk/build_asan && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Volumes/BIGCODE/hydra_sdk /Volumes/BIGCODE/hydra_sdk/src/hydra_crypto /Volumes/BIGCODE/hydra_sdk/build_asan /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto /Volumes/BIGCODE/hydra_sdk/build_asan/src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : src/hydra_crypto/CMakeFiles/hydra_crypto_demo.dir/depend

