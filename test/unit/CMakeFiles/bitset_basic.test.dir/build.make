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
include test/unit/CMakeFiles/bitset_basic.test.dir/depend.make

# Include the progress variables for this target.
include test/unit/CMakeFiles/bitset_basic.test.dir/progress.make

# Include the compile flags for this target's objects.
include test/unit/CMakeFiles/bitset_basic.test.dir/flags.make

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o: test/unit/CMakeFiles/bitset_basic.test.dir/flags.make
test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o: test/unit/bitset_basic.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o   -c /home/wahyu/bee165b/test/unit/bitset_basic.c

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bitset_basic.test.dir/bitset_basic.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/unit/bitset_basic.c > CMakeFiles/bitset_basic.test.dir/bitset_basic.c.i

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bitset_basic.test.dir/bitset_basic.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/unit/bitset_basic.c -o CMakeFiles/bitset_basic.test.dir/bitset_basic.c.s

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.requires:
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.requires

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.provides: test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/bitset_basic.test.dir/build.make test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.provides.build
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.provides

test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.provides.build: test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o

# Object files for target bitset_basic.test
bitset_basic_test_OBJECTS = \
"CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o"

# External object files for target bitset_basic.test
bitset_basic_test_EXTERNAL_OBJECTS =

test/unit/bitset_basic.test: test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o
test/unit/bitset_basic.test: test/unit/CMakeFiles/bitset_basic.test.dir/build.make
test/unit/bitset_basic.test: src/lib/bitset/libbitset.a
test/unit/bitset_basic.test: src/lib/bit/libbit.a
test/unit/bitset_basic.test: test/unit/CMakeFiles/bitset_basic.test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable bitset_basic.test"
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bitset_basic.test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/unit/CMakeFiles/bitset_basic.test.dir/build: test/unit/bitset_basic.test
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/build

test/unit/CMakeFiles/bitset_basic.test.dir/requires: test/unit/CMakeFiles/bitset_basic.test.dir/bitset_basic.c.o.requires
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/requires

test/unit/CMakeFiles/bitset_basic.test.dir/clean:
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -P CMakeFiles/bitset_basic.test.dir/cmake_clean.cmake
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/clean

test/unit/CMakeFiles/bitset_basic.test.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b/test/unit/CMakeFiles/bitset_basic.test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/unit/CMakeFiles/bitset_basic.test.dir/depend

