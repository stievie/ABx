project (abserv CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

# For stdafx.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/abserv)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

file(GLOB_RECURSE ABSERV_SOURCES
    abserv/*.cpp abserv/*.h
    abserv/actions/*.cpp abserv/actions/*.h
    abserv/conditions/*.cpp abserv/conditions/*.h
    abserv/filters/*.cpp abserv/filters/*.h
)

add_executable(
    abserv
    ${ABSERV_SOURCES}
)

if (ABX_SCENE_VIEWER)
  target_compile_definitions(abserv PRIVATE SCENE_VIEWER)
endif(ABX_SCENE_VIEWER)

target_link_libraries(abserv abscommon abcrypto absmath abai abipc abshared Detour DetourTileCache EASTL PugiXml)
if (UNIX)
    target_link_libraries(abserv DeathHandler)
endif()
