# CMakeList.txt : CMake project for elker, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (elker "elker.cpp" "elker.hpp" "skin.hpp" "skin.cpp" "tradeup.hpp" "tradeup.cpp" "log.cpp" "base64.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET elker PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(elker PUBLIC "../extern/eigen")
target_link_libraries(elker PRIVATE spdlog::spdlog)

# TODO: Add tests and install targets if needed.
