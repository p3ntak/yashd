# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.8

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /opt/clion-2017.2.2/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion-2017.2.2/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/matt/CLionProjects/SystemsProjects/yashd

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/yashd.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/yashd.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/yashd.dir/flags.make

CMakeFiles/yashd.dir/yashdclient.c.o: CMakeFiles/yashd.dir/flags.make
CMakeFiles/yashd.dir/yashdclient.c.o: ../yashdclient.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/yashd.dir/yashdclient.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/yashd.dir/yashdclient.c.o   -c /home/matt/CLionProjects/SystemsProjects/yashd/yashdclient.c

CMakeFiles/yashd.dir/yashdclient.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/yashd.dir/yashdclient.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/matt/CLionProjects/SystemsProjects/yashd/yashdclient.c > CMakeFiles/yashd.dir/yashdclient.c.i

CMakeFiles/yashd.dir/yashdclient.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/yashd.dir/yashdclient.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/matt/CLionProjects/SystemsProjects/yashd/yashdclient.c -o CMakeFiles/yashd.dir/yashdclient.c.s

CMakeFiles/yashd.dir/yashdclient.c.o.requires:

.PHONY : CMakeFiles/yashd.dir/yashdclient.c.o.requires

CMakeFiles/yashd.dir/yashdclient.c.o.provides: CMakeFiles/yashd.dir/yashdclient.c.o.requires
	$(MAKE) -f CMakeFiles/yashd.dir/build.make CMakeFiles/yashd.dir/yashdclient.c.o.provides.build
.PHONY : CMakeFiles/yashd.dir/yashdclient.c.o.provides

CMakeFiles/yashd.dir/yashdclient.c.o.provides.build: CMakeFiles/yashd.dir/yashdclient.c.o


CMakeFiles/yashd.dir/yashdhost.c.o: CMakeFiles/yashd.dir/flags.make
CMakeFiles/yashd.dir/yashdhost.c.o: ../yashdhost.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/yashd.dir/yashdhost.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/yashd.dir/yashdhost.c.o   -c /home/matt/CLionProjects/SystemsProjects/yashd/yashdhost.c

CMakeFiles/yashd.dir/yashdhost.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/yashd.dir/yashdhost.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/matt/CLionProjects/SystemsProjects/yashd/yashdhost.c > CMakeFiles/yashd.dir/yashdhost.c.i

CMakeFiles/yashd.dir/yashdhost.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/yashd.dir/yashdhost.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/matt/CLionProjects/SystemsProjects/yashd/yashdhost.c -o CMakeFiles/yashd.dir/yashdhost.c.s

CMakeFiles/yashd.dir/yashdhost.c.o.requires:

.PHONY : CMakeFiles/yashd.dir/yashdhost.c.o.requires

CMakeFiles/yashd.dir/yashdhost.c.o.provides: CMakeFiles/yashd.dir/yashdhost.c.o.requires
	$(MAKE) -f CMakeFiles/yashd.dir/build.make CMakeFiles/yashd.dir/yashdhost.c.o.provides.build
.PHONY : CMakeFiles/yashd.dir/yashdhost.c.o.provides

CMakeFiles/yashd.dir/yashdhost.c.o.provides.build: CMakeFiles/yashd.dir/yashdhost.c.o


CMakeFiles/yashd.dir/yash.c.o: CMakeFiles/yashd.dir/flags.make
CMakeFiles/yashd.dir/yash.c.o: ../yash.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/yashd.dir/yash.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/yashd.dir/yash.c.o   -c /home/matt/CLionProjects/SystemsProjects/yashd/yash.c

CMakeFiles/yashd.dir/yash.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/yashd.dir/yash.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/matt/CLionProjects/SystemsProjects/yashd/yash.c > CMakeFiles/yashd.dir/yash.c.i

CMakeFiles/yashd.dir/yash.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/yashd.dir/yash.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/matt/CLionProjects/SystemsProjects/yashd/yash.c -o CMakeFiles/yashd.dir/yash.c.s

CMakeFiles/yashd.dir/yash.c.o.requires:

.PHONY : CMakeFiles/yashd.dir/yash.c.o.requires

CMakeFiles/yashd.dir/yash.c.o.provides: CMakeFiles/yashd.dir/yash.c.o.requires
	$(MAKE) -f CMakeFiles/yashd.dir/build.make CMakeFiles/yashd.dir/yash.c.o.provides.build
.PHONY : CMakeFiles/yashd.dir/yash.c.o.provides

CMakeFiles/yashd.dir/yash.c.o.provides.build: CMakeFiles/yashd.dir/yash.c.o


# Object files for target yashd
yashd_OBJECTS = \
"CMakeFiles/yashd.dir/yashdclient.c.o" \
"CMakeFiles/yashd.dir/yashdhost.c.o" \
"CMakeFiles/yashd.dir/yash.c.o"

# External object files for target yashd
yashd_EXTERNAL_OBJECTS =

yashd: CMakeFiles/yashd.dir/yashdclient.c.o
yashd: CMakeFiles/yashd.dir/yashdhost.c.o
yashd: CMakeFiles/yashd.dir/yash.c.o
yashd: CMakeFiles/yashd.dir/build.make
yashd: CMakeFiles/yashd.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable yashd"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/yashd.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/yashd.dir/build: yashd

.PHONY : CMakeFiles/yashd.dir/build

CMakeFiles/yashd.dir/requires: CMakeFiles/yashd.dir/yashdclient.c.o.requires
CMakeFiles/yashd.dir/requires: CMakeFiles/yashd.dir/yashdhost.c.o.requires
CMakeFiles/yashd.dir/requires: CMakeFiles/yashd.dir/yash.c.o.requires

.PHONY : CMakeFiles/yashd.dir/requires

CMakeFiles/yashd.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/yashd.dir/cmake_clean.cmake
.PHONY : CMakeFiles/yashd.dir/clean

CMakeFiles/yashd.dir/depend:
	cd /home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/matt/CLionProjects/SystemsProjects/yashd /home/matt/CLionProjects/SystemsProjects/yashd /home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug /home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug /home/matt/CLionProjects/SystemsProjects/yashd/cmake-build-debug/CMakeFiles/yashd.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/yashd.dir/depend

