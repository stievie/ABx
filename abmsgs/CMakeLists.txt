project (abmsgs CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABSMGS_SOURCES      abmsgs/*.cpp abmsgs/*.h)

add_executable(
    abmsgs
    ${ABSMGS_SOURCES}
)

target_precompile_headers(abmsgs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abmsgs/stdafx.h)

target_link_libraries(abmsgs abscommon EASTL)
if (UNIX)
    target_link_libraries(abmsgs DeathHandler)
endif()
