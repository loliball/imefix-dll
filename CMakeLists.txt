cmake_minimum_required(VERSION 3.27)
project(imefix)

set(CMAKE_CXX_STANDARD 17)

link_directories("lib.X64")

add_library(imefix SHARED library.cpp)
