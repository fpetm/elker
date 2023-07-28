configure_file("config.hpp.in" "${CMAKE_BINARY_DIR}/configured_files/include/internal_use_only/config.hpp" ESCAPE_QUOTES)

add_executable (elker "elker.cpp" "elker.hpp" "skin.hpp" "skin.cpp" "tradeup.hpp" "tradeup.cpp" "log.cpp" "base64.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET elker PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(elker 
  PRIVATE elker::elker_options
          elker::elker_warnings)

target_include_directories(elker PUBLIC "../extern/eigen" SYSTEM)
target_link_libraries(elker PRIVATE spdlog::spdlog)
target_include_directories(elker PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
