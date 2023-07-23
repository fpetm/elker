# CMakeList.txt : CMake project for elker, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (elker "elker.cpp" "elker.hpp" "skin.hpp" "collections.hpp" "tradeup.hpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET elker PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(elker PUBLIC "../extern/eigen-3.4.0")

# TODO: Add tests and install targets if needed.
