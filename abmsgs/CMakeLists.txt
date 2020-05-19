cmake_minimum_required (VERSION 3.17)
project (abmsgs CXX)

file(GLOB_RECURSE ABSMGS_SOURCES      abmsgs/*.cpp abmsgs/*.h)

add_executable(
    abmsgs
    ${ABSMGS_SOURCES}
)

target_link_libraries(abmsgs abscommon)
