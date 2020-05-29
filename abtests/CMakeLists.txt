project (abtests CXX)

file(GLOB ABTESTS_SOURCES
    abtests/*.cpp abtests/*.h)

add_executable(
    abtests
    ${ABTESTS_SOURCES}
)

target_link_libraries(abtests abscommon absmath abai abipc tinyexpr)

add_test(ABxTestRuns abtests)
