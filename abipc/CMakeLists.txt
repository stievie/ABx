cmake_minimum_required (VERSION 3.17)
project (abipc CXX)

file(GLOB_RECURSE ABIPC_SOURCES      abipc/*.cpp abipc/*.h)

add_library(
    abipc
    ${ABIPC_SOURCES}
)

target_include_directories(abipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
