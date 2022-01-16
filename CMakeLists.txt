cmake_minimum_required(VERSION 3.13)
project(CompilerCode)

set(CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS " -Wall")

add_executable(CompilerCode main.cpp CompilerHeaders.h Lexer.cpp Lexer.h Parser.cpp Parser.h SymbolTable.cpp SymbolTable.h)