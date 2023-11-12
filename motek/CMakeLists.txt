add_library(motek STATIC log.cpp skin.cpp tradeup.cpp calculator.cpp combinatorics.cpp)

target_link_libraries(motek 
  PRIVATE elker::elker_options
          elker::elker_warnings)
target_include_directories(motek PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(motek SYSTEM PUBLIC  "../extern/eigen")
target_link_libraries(motek PUBLIC spdlog::spdlog)
target_link_libraries(motek PUBLIC pqxx)

add_library(elker::motek ALIAS motek)
