# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/CompilerCode.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/CompilerCode.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/CompilerCode.dir/flags.make

CMakeFiles/CompilerCode.dir/main.cpp.o: CMakeFiles/CompilerCode.dir/flags.make
CMakeFiles/CompilerCode.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/CompilerCode.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CompilerCode.dir/main.cpp.o -c "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/main.cpp"

CMakeFiles/CompilerCode.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CompilerCode.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/main.cpp" > CMakeFiles/CompilerCode.dir/main.cpp.i

CMakeFiles/CompilerCode.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CompilerCode.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/main.cpp" -o CMakeFiles/CompilerCode.dir/main.cpp.s

CMakeFiles/CompilerCode.dir/Lexer.cpp.o: CMakeFiles/CompilerCode.dir/flags.make
CMakeFiles/CompilerCode.dir/Lexer.cpp.o: ../Lexer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/CompilerCode.dir/Lexer.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CompilerCode.dir/Lexer.cpp.o -c "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Lexer.cpp"

CMakeFiles/CompilerCode.dir/Lexer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CompilerCode.dir/Lexer.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Lexer.cpp" > CMakeFiles/CompilerCode.dir/Lexer.cpp.i

CMakeFiles/CompilerCode.dir/Lexer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CompilerCode.dir/Lexer.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Lexer.cpp" -o CMakeFiles/CompilerCode.dir/Lexer.cpp.s

CMakeFiles/CompilerCode.dir/Parser.cpp.o: CMakeFiles/CompilerCode.dir/flags.make
CMakeFiles/CompilerCode.dir/Parser.cpp.o: ../Parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/CompilerCode.dir/Parser.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CompilerCode.dir/Parser.cpp.o -c "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Parser.cpp"

CMakeFiles/CompilerCode.dir/Parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CompilerCode.dir/Parser.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Parser.cpp" > CMakeFiles/CompilerCode.dir/Parser.cpp.i

CMakeFiles/CompilerCode.dir/Parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CompilerCode.dir/Parser.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/Parser.cpp" -o CMakeFiles/CompilerCode.dir/Parser.cpp.s

CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o: CMakeFiles/CompilerCode.dir/flags.make
CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o: ../SymbolTable.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o -c "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/SymbolTable.cpp"

CMakeFiles/CompilerCode.dir/SymbolTable.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CompilerCode.dir/SymbolTable.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/SymbolTable.cpp" > CMakeFiles/CompilerCode.dir/SymbolTable.cpp.i

CMakeFiles/CompilerCode.dir/SymbolTable.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CompilerCode.dir/SymbolTable.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/SymbolTable.cpp" -o CMakeFiles/CompilerCode.dir/SymbolTable.cpp.s

# Object files for target CompilerCode
CompilerCode_OBJECTS = \
"CMakeFiles/CompilerCode.dir/main.cpp.o" \
"CMakeFiles/CompilerCode.dir/Lexer.cpp.o" \
"CMakeFiles/CompilerCode.dir/Parser.cpp.o" \
"CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o"

# External object files for target CompilerCode
CompilerCode_EXTERNAL_OBJECTS =

CompilerCode: CMakeFiles/CompilerCode.dir/main.cpp.o
CompilerCode: CMakeFiles/CompilerCode.dir/Lexer.cpp.o
CompilerCode: CMakeFiles/CompilerCode.dir/Parser.cpp.o
CompilerCode: CMakeFiles/CompilerCode.dir/SymbolTable.cpp.o
CompilerCode: CMakeFiles/CompilerCode.dir/build.make
CompilerCode: CMakeFiles/CompilerCode.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable CompilerCode"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CompilerCode.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/CompilerCode.dir/build: CompilerCode

.PHONY : CMakeFiles/CompilerCode.dir/build

CMakeFiles/CompilerCode.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/CompilerCode.dir/cmake_clean.cmake
.PHONY : CMakeFiles/CompilerCode.dir/clean

CMakeFiles/CompilerCode.dir/depend:
	cd "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode" "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode" "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug" "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug" "/Users/MAS/Documents/Second Year Semester Two/COMP2932 - Compiler Design and Construction/CompilerCode/cmake-build-debug/CMakeFiles/CompilerCode.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/CompilerCode.dir/depend
