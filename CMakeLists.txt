cmake_minimum_required(VERSION 3.31)

project(leylinesim)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()

set(LEYLINE_WARNINGS -Wall -Wextra -Wpedantic)

add_executable(leyexe)
target_sources(leyexe
  PRIVATE
./main.cpp
)

add_library(leylib)
target_sources(leylib
  PRIVATE
  ./leylib/LeyNode.cpp
  ./leylib/LeyGraph.cpp
  ./leylib/LeySource.cpp
  ./leylib/FlowRndGenerator.cpp

  PUBLIC
  FILE_SET leylibheaders
  TYPE HEADERS
  BASE_DIRS ./leylib/include/
  FILES
    ./leylib/include/leylib/LeyNode.h
    ./leylib/include/leylib/LeyGraph.h
    ./leylib/include/leylib/LeySource.h
    ./leylib/include/leylib/FlowRndGenerator.h
)

target_link_libraries(leyexe
PRIVATE
leylib
)

target_compile_options(leyexe  PRIVATE ${LEYLINE_WARNINGS})
target_compile_options(leylib  PRIVATE ${LEYLINE_WARNINGS})
