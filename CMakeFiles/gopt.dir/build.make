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
include CMakeFiles/gopt.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/gopt.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/gopt.dir/flags.make

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o: CMakeFiles/gopt.dir/flags.make
CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o: third_party/gopt/gopt.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o   -c /home/wahyu/bee165b/third_party/gopt/gopt.c

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gopt.dir/third_party/gopt/gopt.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/third_party/gopt/gopt.c > CMakeFiles/gopt.dir/third_party/gopt/gopt.c.i

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gopt.dir/third_party/gopt/gopt.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/third_party/gopt/gopt.c -o CMakeFiles/gopt.dir/third_party/gopt/gopt.c.s

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.requires:
.PHONY : CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.requires

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.provides: CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.requires
	$(MAKE) -f CMakeFiles/gopt.dir/build.make CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.provides.build
.PHONY : CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.provides

CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.provides.build: CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o

# Object files for target gopt
gopt_OBJECTS = \
"CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o"

# External object files for target gopt
gopt_EXTERNAL_OBJECTS =

libgopt.a: CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o
libgopt.a: CMakeFiles/gopt.dir/build.make
libgopt.a: CMakeFiles/gopt.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library libgopt.a"
	$(CMAKE_COMMAND) -P CMakeFiles/gopt.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gopt.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/gopt.dir/build: libgopt.a
.PHONY : CMakeFiles/gopt.dir/build

CMakeFiles/gopt.dir/requires: CMakeFiles/gopt.dir/third_party/gopt/gopt.c.o.requires
.PHONY : CMakeFiles/gopt.dir/requires

CMakeFiles/gopt.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/gopt.dir/cmake_clean.cmake
.PHONY : CMakeFiles/gopt.dir/clean

CMakeFiles/gopt.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b/CMakeFiles/gopt.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/gopt.dir/depend

