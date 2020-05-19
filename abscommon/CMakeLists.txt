cmake_minimum_required (VERSION 3.17)
project (abscommon CXX)

file(GLOB_RECURSE ABSCOMMON_SOURCES      abscommon/*.cpp abscommon/*.h)

add_library(
    abscommon
    ${ABSCOMMON_SOURCES}
)

target_include_directories(abscommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
