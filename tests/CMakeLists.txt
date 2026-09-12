# Fetch GoogleTest (no system install needed).
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(leylib_tests
  NodeIdGeneratorTest.cpp
  GraphGenerationTest.cpp
  EdgeTest.cpp
  NodeOpsTest.cpp
  FlowTest.cpp
)

target_link_libraries(leylib_tests
  PRIVATE
    leylib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(leylib_tests)

add_executable(leylib_flow_tests EXCLUDE_FROM_ALL FlowTest.cpp)
target_link_libraries(leylib_flow_tests PRIVATE leylib GTest::gtest_main)
