add_subdirectory("${CMAKE_SOURCE_DIR}/extern/spdlog" SYSTEM)
add_subdirectory("${CMAKE_SOURCE_DIR}/extern/Catch2")
set(SKIP_BUILD_TEST ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/extern/libpqxx" SYSTEM)
