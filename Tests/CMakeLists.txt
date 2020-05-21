project (Tests CXX)

file(GLOB TESTS_SOURCES      Tests/*.cpp Tests/*.h)

add_executable(
    Tests
    ${TESTS_SOURCES}
)

target_link_libraries(Tests abscommon absmath abai abipc tinyexpr)

add_test(ABxTestRuns Tests)
