cmake_minimum_required(VERSION 3.17)
project(Compile)

set(CMAKE_CXX_STANDARD 14)

add_executable(Compile main.cpp Parsing.h Lexical.h Generator.h Interpreter.h)