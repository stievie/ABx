project (abshared CXX)

file(GLOB ABSHARED_SOURCES      abshared/*.cpp abshared/*.h)

add_library(
    abshared
    ${ABSHARED_SOURCES}
)

target_precompile_headers(abshared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abshared/stdafx.h)

target_include_directories(abshared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
