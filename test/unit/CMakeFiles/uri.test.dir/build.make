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
include test/unit/CMakeFiles/uri.test.dir/depend.make

# Include the progress variables for this target.
include test/unit/CMakeFiles/uri.test.dir/progress.make

# Include the compile flags for this target's objects.
include test/unit/CMakeFiles/uri.test.dir/flags.make

test/unit/CMakeFiles/uri.test.dir/uri.c.o: test/unit/CMakeFiles/uri.test.dir/flags.make
test/unit/CMakeFiles/uri.test.dir/uri.c.o: test/unit/uri.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/uri.test.dir/uri.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/uri.test.dir/uri.c.o   -c /home/wahyu/bee165b/test/unit/uri.c

test/unit/CMakeFiles/uri.test.dir/uri.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uri.test.dir/uri.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/unit/uri.c > CMakeFiles/uri.test.dir/uri.c.i

test/unit/CMakeFiles/uri.test.dir/uri.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uri.test.dir/uri.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/unit/uri.c -o CMakeFiles/uri.test.dir/uri.c.s

test/unit/CMakeFiles/uri.test.dir/uri.c.o.requires:
.PHONY : test/unit/CMakeFiles/uri.test.dir/uri.c.o.requires

test/unit/CMakeFiles/uri.test.dir/uri.c.o.provides: test/unit/CMakeFiles/uri.test.dir/uri.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/uri.test.dir/build.make test/unit/CMakeFiles/uri.test.dir/uri.c.o.provides.build
.PHONY : test/unit/CMakeFiles/uri.test.dir/uri.c.o.provides

test/unit/CMakeFiles/uri.test.dir/uri.c.o.provides.build: test/unit/CMakeFiles/uri.test.dir/uri.c.o

test/unit/CMakeFiles/uri.test.dir/unit.c.o: test/unit/CMakeFiles/uri.test.dir/flags.make
test/unit/CMakeFiles/uri.test.dir/unit.c.o: test/unit/unit.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/uri.test.dir/unit.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/uri.test.dir/unit.c.o   -c /home/wahyu/bee165b/test/unit/unit.c

test/unit/CMakeFiles/uri.test.dir/unit.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uri.test.dir/unit.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/unit/unit.c > CMakeFiles/uri.test.dir/unit.c.i

test/unit/CMakeFiles/uri.test.dir/unit.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uri.test.dir/unit.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/unit/unit.c -o CMakeFiles/uri.test.dir/unit.c.s

test/unit/CMakeFiles/uri.test.dir/unit.c.o.requires:
.PHONY : test/unit/CMakeFiles/uri.test.dir/unit.c.o.requires

test/unit/CMakeFiles/uri.test.dir/unit.c.o.provides: test/unit/CMakeFiles/uri.test.dir/unit.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/uri.test.dir/build.make test/unit/CMakeFiles/uri.test.dir/unit.c.o.provides.build
.PHONY : test/unit/CMakeFiles/uri.test.dir/unit.c.o.provides

test/unit/CMakeFiles/uri.test.dir/unit.c.o.provides.build: test/unit/CMakeFiles/uri.test.dir/unit.c.o

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o: test/unit/CMakeFiles/uri.test.dir/flags.make
test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o: src/uri.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/uri.test.dir/__/__/src/uri.c.o   -c /home/wahyu/bee165b/src/uri.c

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/uri.test.dir/__/__/src/uri.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/src/uri.c > CMakeFiles/uri.test.dir/__/__/src/uri.c.i

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/uri.test.dir/__/__/src/uri.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/src/uri.c -o CMakeFiles/uri.test.dir/__/__/src/uri.c.s

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.requires:
.PHONY : test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.requires

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.provides: test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/uri.test.dir/build.make test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.provides.build
.PHONY : test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.provides

test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.provides.build: test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o

# Object files for target uri.test
uri_test_OBJECTS = \
"CMakeFiles/uri.test.dir/uri.c.o" \
"CMakeFiles/uri.test.dir/unit.c.o" \
"CMakeFiles/uri.test.dir/__/__/src/uri.c.o"

# External object files for target uri.test
uri_test_EXTERNAL_OBJECTS =

test/unit/uri.test: test/unit/CMakeFiles/uri.test.dir/uri.c.o
test/unit/uri.test: test/unit/CMakeFiles/uri.test.dir/unit.c.o
test/unit/uri.test: test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o
test/unit/uri.test: test/unit/CMakeFiles/uri.test.dir/build.make
test/unit/uri.test: test/unit/CMakeFiles/uri.test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable uri.test"
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/uri.test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/unit/CMakeFiles/uri.test.dir/build: test/unit/uri.test
.PHONY : test/unit/CMakeFiles/uri.test.dir/build

test/unit/CMakeFiles/uri.test.dir/requires: test/unit/CMakeFiles/uri.test.dir/uri.c.o.requires
test/unit/CMakeFiles/uri.test.dir/requires: test/unit/CMakeFiles/uri.test.dir/unit.c.o.requires
test/unit/CMakeFiles/uri.test.dir/requires: test/unit/CMakeFiles/uri.test.dir/__/__/src/uri.c.o.requires
.PHONY : test/unit/CMakeFiles/uri.test.dir/requires

test/unit/CMakeFiles/uri.test.dir/clean:
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -P CMakeFiles/uri.test.dir/cmake_clean.cmake
.PHONY : test/unit/CMakeFiles/uri.test.dir/clean

test/unit/CMakeFiles/uri.test.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b/test/unit/CMakeFiles/uri.test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/unit/CMakeFiles/uri.test.dir/depend

