cmake_minimum_required (VERSION 3.17)
project (abfile CXX)

file(GLOB_RECURSE ABFILE_SOURCES      abfile/*.cpp abfile/*.h)

add_executable(
    abfile
    ${ABFILE_SOURCES}
)

target_link_libraries(abfile abscommon)
