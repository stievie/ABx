project (abai CXX)

file(GLOB ABAI_SOURCES      abai/*.cpp abai/*.h)

add_library(
    abai
    ${ABAI_SOURCES}
)

target_precompile_headers(abai PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abai/stdafx.h)

target_include_directories(abai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
