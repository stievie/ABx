cmake_minimum_required (VERSION 3.17)
project (absmath CXX)

file(GLOB_RECURSE ABSMATH_SOURCES      absmath/*.cpp absmath/*.h)

add_library(
    absmath
    ${ABSMATH_SOURCES}
)

target_link_libraries(absmath EASTL)

target_include_directories(absmath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
