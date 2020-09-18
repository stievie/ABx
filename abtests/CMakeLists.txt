project (abtests CXX)

file(GLOB ABTESTS_SOURCES
    abtests/*.cpp abtests/*.h)

add_executable(
    abtests
    ${ABTESTS_SOURCES}
)

if (WIN32)
    target_include_directories(abtests PRIVATE ${CMAKE_SOURCE_DIR}/Include/zlib)
endif()
target_link_libraries(abtests abscommon absmath abai abipc tinyexpr lz4 zlib)

add_test(ABxTestRuns abtests)
