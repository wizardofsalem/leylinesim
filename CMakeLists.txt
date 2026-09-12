cmake_minimum_required(VERSION 3.31)

project(leylinesim)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()

set(LEYLINE_WARNINGS -Wall -Wextra -Wpedantic)

option(LEYLINE_BUILD_TESTS "Build unit tests" ON)

option(LEYLINE_BUILD_BENCHMARKS "Build benchmarks" OFF)

add_executable(leyexe)
target_sources(leyexe
  PRIVATE
  ./src/exe/main.cpp
)
target_include_directories(leyexe PRIVATE ./include/exe)

add_library(leylib)
target_sources(leylib
  PRIVATE
  ./src/lib/LeyGraph.cpp
  ./src/lib/FlowRndGenerator.cpp
  ./src/lib/NodeIdGenerator.cpp

  PUBLIC
  FILE_SET leylibheaders
  TYPE HEADERS
  BASE_DIRS ./include/lib/
  FILES
    ./include/lib/leylib/LeyNode.h
    ./include/lib/leylib/LeyLine.h
    ./include/lib/leylib/LeyGraph.h
    ./include/lib/leylib/NodeHandle.h
    ./include/lib/leylib/FlowRndGenerator.h
    ./include/lib/leylib/NodeIdGenerator.h
)

target_link_libraries(leyexe
PRIVATE
leylib
)

target_compile_features(leylib PUBLIC cxx_std_20)

target_compile_options(leyexe  PRIVATE ${LEYLINE_WARNINGS})
target_compile_options(leylib  PRIVATE ${LEYLINE_WARNINGS})

if(LEYLINE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(LEYLINE_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
