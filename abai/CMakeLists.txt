project (abai CXX)

file(GLOB_RECURSE ABAI_SOURCES      abai/*.cpp abai/*.h)

add_library(
    abai
    ${ABAI_SOURCES}
)

target_include_directories(abai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
