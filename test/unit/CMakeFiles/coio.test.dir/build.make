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
include test/unit/CMakeFiles/coio.test.dir/depend.make

# Include the progress variables for this target.
include test/unit/CMakeFiles/coio.test.dir/progress.make

# Include the compile flags for this target's objects.
include test/unit/CMakeFiles/coio.test.dir/flags.make

test/unit/CMakeFiles/coio.test.dir/coio.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/coio.cc.o: test/unit/coio.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/coio.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/coio.cc.o -c /home/wahyu/bee165b/test/unit/coio.cc

test/unit/CMakeFiles/coio.test.dir/coio.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/coio.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/test/unit/coio.cc > CMakeFiles/coio.test.dir/coio.cc.i

test/unit/CMakeFiles/coio.test.dir/coio.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/coio.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/test/unit/coio.cc -o CMakeFiles/coio.test.dir/coio.cc.s

test/unit/CMakeFiles/coio.test.dir/coio.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/coio.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/coio.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/coio.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/coio.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/coio.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/coio.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/coio.cc.o

test/unit/CMakeFiles/coio.test.dir/unit.c.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/unit.c.o: test/unit/unit.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/coio.test.dir/unit.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/coio.test.dir/unit.c.o   -c /home/wahyu/bee165b/test/unit/unit.c

test/unit/CMakeFiles/coio.test.dir/unit.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coio.test.dir/unit.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/test/unit/unit.c > CMakeFiles/coio.test.dir/unit.c.i

test/unit/CMakeFiles/coio.test.dir/unit.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coio.test.dir/unit.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/test/unit/unit.c -o CMakeFiles/coio.test.dir/unit.c.s

test/unit/CMakeFiles/coio.test.dir/unit.c.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/unit.c.o.requires

test/unit/CMakeFiles/coio.test.dir/unit.c.o.provides: test/unit/CMakeFiles/coio.test.dir/unit.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/unit.c.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/unit.c.o.provides

test/unit/CMakeFiles/coio.test.dir/unit.c.o.provides.build: test/unit/CMakeFiles/coio.test.dir/unit.c.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o: src/sio.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/sio.cc.o -c /home/wahyu/bee165b/src/sio.cc

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/__/__/src/sio.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/sio.cc > CMakeFiles/coio.test.dir/__/__/src/sio.cc.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/__/__/src/sio.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/sio.cc -o CMakeFiles/coio.test.dir/__/__/src/sio.cc.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o: src/evio.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/evio.cc.o -c /home/wahyu/bee165b/src/evio.cc

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/__/__/src/evio.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/evio.cc > CMakeFiles/coio.test.dir/__/__/src/evio.cc.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/__/__/src/evio.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/evio.cc -o CMakeFiles/coio.test.dir/__/__/src/evio.cc.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o: src/coio.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/coio.cc.o -c /home/wahyu/bee165b/src/coio.cc

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/__/__/src/coio.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/coio.cc > CMakeFiles/coio.test.dir/__/__/src/coio.cc.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/__/__/src/coio.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/coio.cc -o CMakeFiles/coio.test.dir/__/__/src/coio.cc.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o: src/coeio.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_6)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o -c /home/wahyu/bee165b/src/coeio.cc

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/__/__/src/coeio.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/coeio.cc > CMakeFiles/coio.test.dir/__/__/src/coeio.cc.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/__/__/src/coeio.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/coeio.cc -o CMakeFiles/coio.test.dir/__/__/src/coeio.cc.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o: src/uri.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_7)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/uri.c.o   -c /home/wahyu/bee165b/src/uri.c

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coio.test.dir/__/__/src/uri.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/src/uri.c > CMakeFiles/coio.test.dir/__/__/src/uri.c.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coio.test.dir/__/__/src/uri.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/src/uri.c -o CMakeFiles/coio.test.dir/__/__/src/uri.c.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o: src/fio.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_8)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/fio.c.o   -c /home/wahyu/bee165b/src/fio.c

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coio.test.dir/__/__/src/fio.c.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/wahyu/bee165b/src/fio.c > CMakeFiles/coio.test.dir/__/__/src/fio.c.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coio.test.dir/__/__/src/fio.c.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/wahyu/bee165b/src/fio.c -o CMakeFiles/coio.test.dir/__/__/src/fio.c.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o: test/unit/CMakeFiles/coio.test.dir/flags.make
test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o: src/iobuf.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/wahyu/bee165b/CMakeFiles $(CMAKE_PROGRESS_9)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o -c /home/wahyu/bee165b/src/iobuf.cc

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.i"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/wahyu/bee165b/src/iobuf.cc > CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.i

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.s"
	cd /home/wahyu/bee165b/test/unit && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/wahyu/bee165b/src/iobuf.cc -o CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.s

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.requires:
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.requires

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.provides: test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.requires
	$(MAKE) -f test/unit/CMakeFiles/coio.test.dir/build.make test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.provides.build
.PHONY : test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.provides

test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.provides.build: test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o

# Object files for target coio.test
coio_test_OBJECTS = \
"CMakeFiles/coio.test.dir/coio.cc.o" \
"CMakeFiles/coio.test.dir/unit.c.o" \
"CMakeFiles/coio.test.dir/__/__/src/sio.cc.o" \
"CMakeFiles/coio.test.dir/__/__/src/evio.cc.o" \
"CMakeFiles/coio.test.dir/__/__/src/coio.cc.o" \
"CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o" \
"CMakeFiles/coio.test.dir/__/__/src/uri.c.o" \
"CMakeFiles/coio.test.dir/__/__/src/fio.c.o" \
"CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o"

# External object files for target coio.test
coio_test_EXTERNAL_OBJECTS =

test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/coio.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/unit.c.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/build.make
test/unit/coio.test: src/libcore.a
test/unit/coio.test: libeio.a
test/unit/coio.test: src/lib/bit/libbit.a
test/unit/coio.test: src/lib/salad/libsalad.so
test/unit/coio.test: src/lib/small/libsmall.so
test/unit/coio.test: libev.a
test/unit/coio.test: libcoro.a
test/unit/coio.test: test/unit/CMakeFiles/coio.test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable coio.test"
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/coio.test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/unit/CMakeFiles/coio.test.dir/build: test/unit/coio.test
.PHONY : test/unit/CMakeFiles/coio.test.dir/build

test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/coio.cc.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/unit.c.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/sio.cc.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/evio.cc.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/coio.cc.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/coeio.cc.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/uri.c.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/fio.c.o.requires
test/unit/CMakeFiles/coio.test.dir/requires: test/unit/CMakeFiles/coio.test.dir/__/__/src/iobuf.cc.o.requires
.PHONY : test/unit/CMakeFiles/coio.test.dir/requires

test/unit/CMakeFiles/coio.test.dir/clean:
	cd /home/wahyu/bee165b/test/unit && $(CMAKE_COMMAND) -P CMakeFiles/coio.test.dir/cmake_clean.cmake
.PHONY : test/unit/CMakeFiles/coio.test.dir/clean

test/unit/CMakeFiles/coio.test.dir/depend:
	cd /home/wahyu/bee165b && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b /home/wahyu/bee165b/test/unit /home/wahyu/bee165b/test/unit/CMakeFiles/coio.test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/unit/CMakeFiles/coio.test.dir/depend

