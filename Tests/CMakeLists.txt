cmake_minimum_required (VERSION 3.17)
project (Tests CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE TESTS_SOURCES      Tests/*.cpp Tests/*.h)

add_executable(
    Tests
    ${TESTS_SOURCES}
)

target_link_libraries(Tests abscommon absmath abai abipc tinyexpr)

add_test(ABxTestRuns Tests)
