# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop

# Include any dependencies generated for this target.
include src/CMakeFiles/OpenGL.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/OpenGL.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/OpenGL.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/OpenGL.dir/flags.make

src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o: src/CMakeFiles/OpenGL.dir/flags.make
src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o: /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_api.cpp
src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o: src/CMakeFiles/OpenGL.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o -MF CMakeFiles/OpenGL.dir/opengl_api.cpp.o.d -o CMakeFiles/OpenGL.dir/opengl_api.cpp.o -c /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_api.cpp

src/CMakeFiles/OpenGL.dir/opengl_api.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenGL.dir/opengl_api.cpp.i"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_api.cpp > CMakeFiles/OpenGL.dir/opengl_api.cpp.i

src/CMakeFiles/OpenGL.dir/opengl_api.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenGL.dir/opengl_api.cpp.s"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_api.cpp -o CMakeFiles/OpenGL.dir/opengl_api.cpp.s

src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o: src/CMakeFiles/OpenGL.dir/flags.make
src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o: /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_texture.cpp
src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o: src/CMakeFiles/OpenGL.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o -MF CMakeFiles/OpenGL.dir/opengl_texture.cpp.o.d -o CMakeFiles/OpenGL.dir/opengl_texture.cpp.o -c /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_texture.cpp

src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenGL.dir/opengl_texture.cpp.i"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_texture.cpp > CMakeFiles/OpenGL.dir/opengl_texture.cpp.i

src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenGL.dir/opengl_texture.cpp.s"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && /usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src/opengl_texture.cpp -o CMakeFiles/OpenGL.dir/opengl_texture.cpp.s

# Object files for target OpenGL
OpenGL_OBJECTS = \
"CMakeFiles/OpenGL.dir/opengl_api.cpp.o" \
"CMakeFiles/OpenGL.dir/opengl_texture.cpp.o"

# External object files for target OpenGL
OpenGL_EXTERNAL_OBJECTS =

src/libOpenGL.so: src/CMakeFiles/OpenGL.dir/opengl_api.cpp.o
src/libOpenGL.so: src/CMakeFiles/OpenGL.dir/opengl_texture.cpp.o
src/libOpenGL.so: src/CMakeFiles/OpenGL.dir/build.make
src/libOpenGL.so: src/CMakeFiles/OpenGL.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX shared library libOpenGL.so"
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/OpenGL.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/OpenGL.dir/build: src/libOpenGL.so
.PHONY : src/CMakeFiles/OpenGL.dir/build

src/CMakeFiles/OpenGL.dir/clean:
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src && $(CMAKE_COMMAND) -P CMakeFiles/OpenGL.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/OpenGL.dir/clean

src/CMakeFiles/OpenGL.dir/depend:
	cd /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/src /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src /home/programier/Projects/C++/GameEngine/src/Engine/src/OpenGL/build-Debug-Desktop/src/CMakeFiles/OpenGL.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/OpenGL.dir/depend
