project (abipc CXX)

file(GLOB_RECURSE ABIPC_SOURCES      abipc/*.cpp abipc/*.h)

add_library(
    abipc
    ${ABIPC_SOURCES}
)

target_precompile_headers(abipc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abipc/stdafx.h)

target_include_directories(abipc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
