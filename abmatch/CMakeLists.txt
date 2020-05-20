project (abmatch CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABMATCH_SOURCES      abmatch/*.cpp abmatch/*.h)

add_executable(
    abmatch
    ${ABMATCH_SOURCES}
)

target_link_libraries(abmatch abscommon EASTL)
if (UNIX)
    target_link_libraries(abmatch DeathHandler)
endif()
