cmake_minimum_required (VERSION 3.17)
project (abmatch CXX)

file(GLOB_RECURSE ABMATCH_SOURCES      abmatch/*.cpp abmatch/*.h)

add_executable(
    abmatch
    ${ABMATCH_SOURCES}
)

target_link_libraries(abmatch abscommon)
