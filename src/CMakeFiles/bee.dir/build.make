# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wahyu/bee165b

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wahyu/bee165b

# Include any dependencies generated for this target.
include src/CMakeFiles/bee.dir/depend.make

# Include the progress variables for this target.
include src/CMakeFiles/bee.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/bee.dir/flags.make

src/CMakeFiles/bee.dir/main.cc.o: src/CMakeFiles/bee.dir/flags.make
src/CMakeFiles/bee.dir/main.cc.o: src/main.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/CMakeFiles/bee.dir/main.cc.o"
	cd /home/wahyu/bee165b/src && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/bee.dir/main.cc.o -c /home/wahyu/bee165b/src/main.cc

src/CMakeFiles/bee.dir/main.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bee.dir/main.cc.i"
	cd /home/wahyu/bee165b/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/main.cc > CMakeFiles/bee.dir/main.cc.i

src/CMakeFiles/bee.dir/main.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bee.dir/main.cc.s"
	cd /home/wahyu/bee165b/src && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/main.cc -o CMakeFiles/bee.dir/main.cc.s

src/CMakeFiles/bee.dir/main.cc.o.requires:
.PHONY : src/CMakeFiles/bee.dir/main.cc.o.requires

src/CMakeFiles/bee.dir/main.cc.o.provides: src/CMakeFiles/bee.dir/main.cc.o.requires
	$(MAKE) -f src/CMakeFiles/bee.dir/build.make src/CMakeFiles/bee.dir/main.cc.o.provides.build
.PHONY : src/CMakeFiles/bee.dir/main.cc.o.provides

src/CMakeFiles/bee.dir/main.cc.o.provides.build: src/CMakeFiles/bee.dir/main.cc.o

# Object files for target bee
bee_OBJECTS = \
"CMakeFiles/bee.dir/main.cc.o"

# External object files for target bee
bee_EXTERNAL_OBJECTS =

src/bee: src/CMakeFiles/bee.dir/main.cc.o
src/bee: src/CMakeFiles/bee.dir/build.make
src/bee: src/db/libdb.so
src/bee: src/libserver.a
src/bee: src/libcore.a
src/bee: libmisc.a
src/bee: src/lib/bitset/libbitset.a
src/bee: src/lib/libmsgpuck.a
src/bee: libeio.a
src/bee: libgopt.a
src/bee: libcjson.a
src/bee: libyaml.a
src/bee: /usr/lib/x86_64-linux-gnu/libreadline.so
src/bee: /usr/lib/x86_64-linux-gnu/libtermcap.so
src/bee: third_party/luajit/src/libluajit.a
src/bee: third_party/sham/libsham.a
src/bee: src/lib/salad/libsalad.so
src/bee: src/lib/small/libsmall.so
src/bee: libev.a
src/bee: libcoro.a
src/bee: src/lib/bit/libbit.a
src/bee: src/CMakeFiles/bee.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable bee"
	cd /home/wahyu/bee165b/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bee.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/bee.dir/build: src/bee
.PHONY : src/CMakeFiles/bee.dir/build

src/CMakeFiles/bee.dir/requires: src/CMakeFiles/bee.dir/main.cc.o.requires
.PHONY : src/CMakeFiles/bee.dir/requires

src/CMakeFiles/bee.dir/clean:
	cd /home/wahyu/bee165b/src && $(CMAKE_COMMAND) -P CMakeFiles/bee.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/bee.dir/clean

src/CMakeFiles/bee.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/src /home/wahyu/bee165b /home/wahyu/bee165b/src /home/wahyu/bee165b/src/CMakeFiles/bee.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/bee.dir/depend

