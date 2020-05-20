project (abshared CXX)

file(GLOB_RECURSE ABSHARED_SOURCES      abshared/*.cpp abshared/*.h)

add_library(
    abshared
    ${ABSHARED_SOURCES}
)

target_include_directories(abshared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
