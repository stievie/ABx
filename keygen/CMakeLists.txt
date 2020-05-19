cmake_minimum_required (VERSION 3.17)
project (keygen CXX)

file(GLOB_RECURSE KEYGEN_SOURCES      keygen/*.cpp keygen/*.h)

add_executable(
    keygen
    ${KEYGEN_SOURCES}
)

target_link_libraries(keygen abscommon)
