project (dbtool CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE DBTOOL_SOURCES
    dbtool/*.cpp
    dbtool/*.h
)

add_executable(
    dbtool
    ${DBTOOL_SOURCES}
)

target_link_libraries(dbtool abdb abscommon)
