cmake_minimum_required (VERSION 3.17)
project (absmath CXX)

include_directories(../Include/DirectXMath)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
endif()

file(GLOB_RECURSE ABSMATH_SOURCES
    absmath/*.cpp
    absmath/*.h
)

add_library(
    absmath
    ${ABSMATH_SOURCES}
)

target_link_libraries(absmath EASTL)

target_include_directories(absmath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(absmath PUBLIC ../Include/DirectXMath)
