# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.25)

project ("elker")

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)

elker_setup_options()

elker_global_options()

elker_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

target_compile_features(elker_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(elker::elker_options ALIAS elker_options)
add_library(elker::elker_warnings ALIAS elker_warnings)


add_subdirectory(extern)
add_subdirectory ("elker")
