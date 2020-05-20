project (abdata CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABDATA_SOURCES      abdata/*.cpp abdata/*.h)

add_executable(
    abdata
    ${ABDATA_SOURCES}
)

target_precompile_headers(abdata PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abdata/stdafx.h)

target_link_libraries(abdata abdb abscommon EASTL)
if (UNIX)
    target_link_libraries(abdata DeathHandler)
endif()
