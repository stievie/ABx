project (abtests CXX)

file(GLOB ABTESTS_SOURCES
    abtests/*.cpp abtests/*.h)

add_executable(
    abtests
    ${ABTESTS_SOURCES}
)

target_link_libraries(abtests abscommon absmath abai abipc tinyexpr lz4)
if (WIN32)
    target_include_directories(abtests PRIVATE ${CMAKE_SOURCE_DIR}/Include/zlib)
    target_link_libraries(abtests zlib)
else()
    target_link_libraries(abtests z)
endif()

add_test(ABxTestRuns abtests)
