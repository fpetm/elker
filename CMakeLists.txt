# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.25)

project ("elker"
	VERSION 1.0.0 
	DESCRIPTION ""
	LANGUAGES C CXX)

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

execute_process(COMMAND git log -1 --format=%h
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                OUTPUT_VARIABLE GIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE)

add_custom_target(AlwaysCheckGit COMMAND ${CMAKE_COMMAND}
    -DRUN_CHECK_GIT_VERSION=1
    -Dpre_configure_dir=${pre_configure_dir}
    -Dpost_configure_file=${post_configure_dir}
    -DGIT_HASH_CACHE=${GIT_HASH_CACHE}
    -P ${CURRENT_LIST_DIR}/CheckGit.cmake
    BYPRODUCTS ${post_configure_file})

add_subdirectory(extern)
add_subdirectory(motek)
add_subdirectory(elker)

if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

# Adding the tests:
include(CTest)

if(BUILD_TESTING)
  message(AUTHOR_WARNING "Building Tests")
  add_subdirectory(test)
endif()
