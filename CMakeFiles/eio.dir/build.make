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
include CMakeFiles/eio.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/eio.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/eio.dir/flags.make

CMakeFiles/eio.dir/third_party/bee_eio.c.o: CMakeFiles/eio.dir/flags.make
CMakeFiles/eio.dir/third_party/bee_eio.c.o: third_party/bee_eio.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/eio.dir/third_party/bee_eio.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/eio.dir/third_party/bee_eio.c.o   -c /home/wahyu/bee165b/third_party/bee_eio.c

CMakeFiles/eio.dir/third_party/bee_eio.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eio.dir/third_party/bee_eio.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/third_party/bee_eio.c > CMakeFiles/eio.dir/third_party/bee_eio.c.i

CMakeFiles/eio.dir/third_party/bee_eio.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eio.dir/third_party/bee_eio.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/third_party/bee_eio.c -o CMakeFiles/eio.dir/third_party/bee_eio.c.s

CMakeFiles/eio.dir/third_party/bee_eio.c.o.requires:
.PHONY : CMakeFiles/eio.dir/third_party/bee_eio.c.o.requires

CMakeFiles/eio.dir/third_party/bee_eio.c.o.provides: CMakeFiles/eio.dir/third_party/bee_eio.c.o.requires
	$(MAKE) -f CMakeFiles/eio.dir/build.make CMakeFiles/eio.dir/third_party/bee_eio.c.o.provides.build
.PHONY : CMakeFiles/eio.dir/third_party/bee_eio.c.o.provides

CMakeFiles/eio.dir/third_party/bee_eio.c.o.provides.build: CMakeFiles/eio.dir/third_party/bee_eio.c.o

# Object files for target eio
eio_OBJECTS = \
"CMakeFiles/eio.dir/third_party/bee_eio.c.o"

# External object files for target eio
eio_EXTERNAL_OBJECTS =

libeio.a: CMakeFiles/eio.dir/third_party/bee_eio.c.o
libeio.a: CMakeFiles/eio.dir/build.make
libeio.a: CMakeFiles/eio.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library libeio.a"
	$(CMAKE_COMMAND) -P CMakeFiles/eio.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/eio.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/eio.dir/build: libeio.a
.PHONY : CMakeFiles/eio.dir/build

CMakeFiles/eio.dir/requires: CMakeFiles/eio.dir/third_party/bee_eio.c.o.requires
.PHONY : CMakeFiles/eio.dir/requires

CMakeFiles/eio.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/eio.dir/cmake_clean.cmake
.PHONY : CMakeFiles/eio.dir/clean

CMakeFiles/eio.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b/CMakeFiles/eio.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/eio.dir/depend

