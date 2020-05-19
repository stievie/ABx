cmake_minimum_required (VERSION 3.17)
project (less CXX)

file(GLOB_RECURSE LESS_SOURCES      src/*.cpp src/css/*.cpp src/less/*.cpp src/lessstylesheet/*.cpp src/stylesheet/*.cpp src/value/*.cpp
                                    include/less/*.h include/less/css/*.h include/less/less/*.h include/less/lessstylesheet/*.h include/less/stylesheet/*.h include/less/value/*.h)

add_library(
    less
    ${LESS_SOURCES}
)

target_include_directories(EASTL PUBLIC include/less)
