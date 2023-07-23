# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.25)

project ("elker")

add_subdirectory(extern)
add_subdirectory ("elker")
