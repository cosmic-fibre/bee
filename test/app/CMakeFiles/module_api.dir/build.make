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
include test/app/CMakeFiles/module_api.dir/depend.make

# Include the progress variables for this target.
include test/app/CMakeFiles/module_api.dir/progress.make

# Include the compile flags for this target's objects.
include test/app/CMakeFiles/module_api.dir/flags.make

test/app/CMakeFiles/module_api.dir/module_api.c.o: test/app/CMakeFiles/module_api.dir/flags.make
test/app/CMakeFiles/module_api.dir/module_api.c.o: test/app/module_api.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/app/CMakeFiles/module_api.dir/module_api.c.o"
	cd /home/wahyu/bee165b/test/app && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/module_api.dir/module_api.c.o   -c /home/wahyu/bee165b/test/app/module_api.c

test/app/CMakeFiles/module_api.dir/module_api.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/module_api.dir/module_api.c.i"
	cd /home/wahyu/bee165b/test/app && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/app/module_api.c > CMakeFiles/module_api.dir/module_api.c.i

test/app/CMakeFiles/module_api.dir/module_api.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/module_api.dir/module_api.c.s"
	cd /home/wahyu/bee165b/test/app && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/app/module_api.c -o CMakeFiles/module_api.dir/module_api.c.s

test/app/CMakeFiles/module_api.dir/module_api.c.o.requires:
.PHONY : test/app/CMakeFiles/module_api.dir/module_api.c.o.requires

test/app/CMakeFiles/module_api.dir/module_api.c.o.provides: test/app/CMakeFiles/module_api.dir/module_api.c.o.requires
	$(MAKE) -f test/app/CMakeFiles/module_api.dir/build.make test/app/CMakeFiles/module_api.dir/module_api.c.o.provides.build
.PHONY : test/app/CMakeFiles/module_api.dir/module_api.c.o.provides

test/app/CMakeFiles/module_api.dir/module_api.c.o.provides.build: test/app/CMakeFiles/module_api.dir/module_api.c.o

# Object files for target module_api
module_api_OBJECTS = \
"CMakeFiles/module_api.dir/module_api.c.o"

# External object files for target module_api
module_api_EXTERNAL_OBJECTS =

test/app/module_api.so: test/app/CMakeFiles/module_api.dir/module_api.c.o
test/app/module_api.so: test/app/CMakeFiles/module_api.dir/build.make
test/app/module_api.so: test/app/CMakeFiles/module_api.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library module_api.so"
	cd /home/wahyu/bee165b/test/app && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/module_api.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/app/CMakeFiles/module_api.dir/build: test/app/module_api.so
.PHONY : test/app/CMakeFiles/module_api.dir/build

test/app/CMakeFiles/module_api.dir/requires: test/app/CMakeFiles/module_api.dir/module_api.c.o.requires
.PHONY : test/app/CMakeFiles/module_api.dir/requires

test/app/CMakeFiles/module_api.dir/clean:
	cd /home/wahyu/bee165b/test/app && $(CMAKE_COMMAND) -P CMakeFiles/module_api.dir/cmake_clean.cmake
.PHONY : test/app/CMakeFiles/module_api.dir/clean

test/app/CMakeFiles/module_api.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/test/app /home/wahyu/bee165b /home/wahyu/bee165b/test/app /home/wahyu/bee165b/test/app/CMakeFiles/module_api.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/app/CMakeFiles/module_api.dir/depend

