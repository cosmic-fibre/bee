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

# Utility rule file for libsham.

# Include the progress variables for this target.
include CMakeFiles/libsham.dir/progress.make

CMakeFiles/libsham: third_party/sham/libsham.a

third_party/sham/libsham.a: CMakeCache.txt
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Generating third_party/sham/libsham.a"
	cd /home/wahyu/bee165b/third_party/sham && $(MAKE) CFLAGS=" -fno-omit-frame-pointer -fno-stack-protector -fexceptions -funwind-tables -fopenmp -msse2" LDFLAGS=" " clean
	cd /home/wahyu/bee165b/third_party/sham && $(MAKE) CFLAGS=" -fno-omit-frame-pointer -fno-stack-protector -fexceptions -funwind-tables -fopenmp -msse2" LDFLAGS=" " static

libsham: CMakeFiles/libsham
libsham: third_party/sham/libsham.a
libsham: CMakeFiles/libsham.dir/build.make
.PHONY : libsham

# Rule to build all files generated by this target.
CMakeFiles/libsham.dir/build: libsham
.PHONY : CMakeFiles/libsham.dir/build

CMakeFiles/libsham.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/libsham.dir/cmake_clean.cmake
.PHONY : CMakeFiles/libsham.dir/clean

CMakeFiles/libsham.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b /home/wahyu/bee165b/CMakeFiles/libsham.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/libsham.dir/depend

