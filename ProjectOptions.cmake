include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

macro(elker_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(elker_setup_options)
  option(elker_ENABLE_COVERAGE "Enable coverage reporting" OFF)

  elker_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR elker_PACKAGING_MAINTAINER_MODE)
    option(elker_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(elker_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(elker_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(elker_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(elker_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(elker_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(elker_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(elker_ENABLE_PCH "Enable precompiled headers" OFF)
    option(elker_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(elker_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(elker_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(elker_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(elker_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(elker_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(elker_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(elker_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(elker_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(elker_ENABLE_PCH "Enable precompiled headers" OFF)
    option(elker_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      elker_ENABLE_IPO
      elker_WARNINGS_AS_ERRORS
      elker_ENABLE_USER_LINKER
      elker_ENABLE_SANITIZER_ADDRESS
      elker_ENABLE_SANITIZER_LEAK
      elker_ENABLE_SANITIZER_UNDEFINED
      elker_ENABLE_SANITIZER_THREAD
      elker_ENABLE_SANITIZER_MEMORY
      elker_ENABLE_UNITY_BUILD
      elker_ENABLE_CLANG_TIDY
      elker_ENABLE_CPPCHECK
      elker_ENABLE_COVERAGE
      elker_ENABLE_PCH
      elker_ENABLE_CACHE)
  endif()
endmacro()

macro(elker_global_options)
  if(elker_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    elker_enable_ipo()
  endif()

  elker_supports_sanitizers()
endmacro()

macro(elker_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(elker_warnings INTERFACE)
  add_library(elker_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  elker_set_project_warnings(
    elker_warnings
    ${elker_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(elker_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(elker_options)
  endif()

  include(cmake/Sanitizers.cmake)
  elker_enable_sanitizers(
    elker_options
    ${elker_ENABLE_SANITIZER_ADDRESS}
    ${elker_ENABLE_SANITIZER_LEAK}
    ${elker_ENABLE_SANITIZER_UNDEFINED}
    ${elker_ENABLE_SANITIZER_THREAD}
    ${elker_ENABLE_SANITIZER_MEMORY})

  set_target_properties(elker_options PROPERTIES UNITY_BUILD ${elker_ENABLE_UNITY_BUILD})

  if(elker_ENABLE_PCH)
    target_precompile_headers(
      elker_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(elker_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    elker_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(elker_ENABLE_CLANG_TIDY)
    elker_enable_clang_tidy(elker_options ${elker_WARNINGS_AS_ERRORS})
  endif()

  if(elker_ENABLE_CPPCHECK)
    elker_enable_cppcheck(${elker_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(elker_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    elker_enable_coverage(elker_options)
  endif()

  if(elker_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(myproject_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()
endmacro()
