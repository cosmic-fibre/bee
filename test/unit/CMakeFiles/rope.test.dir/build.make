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
include test/unit/CMakeFiles/rope.test.dir/depend.make

# Include the progress variables for this target.
include test/unit/CMakeFiles/rope.test.dir/progress.make

# Include the compile flags for this target's objects.
include test/unit/CMakeFiles/rope.test.dir/flags.make

test/unit/CMakeFiles/rope.test.dir/rope.c.o: test/unit/CMakeFiles/rope.test.dir/flags.make
test/unit/CMakeFiles/rope.test.dir/rope.c.o: test/unit/rope.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/rope.test.dir/rope.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/rope.test.dir/rope.c.o   -c /home/wahyu/bee165b/test/unit/rope.c

test/unit/CMakeFiles/rope.test.dir/rope.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rope.test.dir/rope.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/unit/rope.c > CMakeFiles/rope.test.dir/rope.c.i

test/unit/CMakeFiles/rope.test.dir/rope.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rope.test.dir/rope.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/unit/rope.c -o CMakeFiles/rope.test.dir/rope.c.s

test/unit/CMakeFiles/rope.test.dir/rope.c.o.requires:
.PHONY : test/unit/CMakeFiles/rope.test.dir/rope.c.o.requires

test/unit/CMakeFiles/rope.test.dir/rope.c.o.provides: test/unit/CMakeFiles/rope.test.dir/rope.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/rope.test.dir/build.make test/unit/CMakeFiles/rope.test.dir/rope.c.o.provides.build
.PHONY : test/unit/CMakeFiles/rope.test.dir/rope.c.o.provides

test/unit/CMakeFiles/rope.test.dir/rope.c.o.provides.build: test/unit/CMakeFiles/rope.test.dir/rope.c.o

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o: test/unit/CMakeFiles/rope.test.dir/flags.make
test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o: src/lib/salad/rope.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o   -c /home/wahyu/bee165b/src/lib/salad/rope.c

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/src/lib/salad/rope.c > CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.i

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/src/lib/salad/rope.c -o CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.s

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.requires:
.PHONY : test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.requires

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.provides: test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/rope.test.dir/build.make test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.provides.build
.PHONY : test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.provides

test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.provides.build: test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o

# Object files for target rope.test
rope_test_OBJECTS = \
"CMakeFiles/rope.test.dir/rope.c.o" \
"CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o"

# External object files for target rope.test
rope_test_EXTERNAL_OBJECTS =

test/unit/rope.test: test/unit/CMakeFiles/rope.test.dir/rope.c.o
test/unit/rope.test: test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o
test/unit/rope.test: test/unit/CMakeFiles/rope.test.dir/build.make
test/unit/rope.test: test/unit/CMakeFiles/rope.test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable rope.test"
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rope.test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/unit/CMakeFiles/rope.test.dir/build: test/unit/rope.test
.PHONY : test/unit/CMakeFiles/rope.test.dir/build

test/unit/CMakeFiles/rope.test.dir/requires: test/unit/CMakeFiles/rope.test.dir/rope.c.o.requires
test/unit/CMakeFiles/rope.test.dir/requires: test/unit/CMakeFiles/rope.test.dir/__/__/src/lib/salad/rope.c.o.requires
.PHONY : test/unit/CMakeFiles/rope.test.dir/requires

test/unit/CMakeFiles/rope.test.dir/clean:
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -P CMakeFiles/rope.test.dir/cmake_clean.cmake
.PHONY : test/unit/CMakeFiles/rope.test.dir/clean

test/unit/CMakeFiles/rope.test.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b/test/unit/CMakeFiles/rope.test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/unit/CMakeFiles/rope.test.dir/depend

