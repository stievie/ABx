project (abserv CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

file(GLOB ABSERV_SOURCES
    abserv/*.cpp abserv/*.h
    abserv/actions/*.cpp abserv/actions/*.h
    abserv/conditions/*.cpp abserv/conditions/*.h
    abserv/filters/*.cpp abserv/filters/*.h
)

add_executable(
    abserv
    ${ABSERV_SOURCES}
)

target_precompile_headers(abserv PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abserv/stdafx.h)

if (ABX_SCENE_VIEWER)
  target_compile_definitions(abserv PRIVATE SCENE_VIEWER)
endif(ABX_SCENE_VIEWER)

target_link_libraries(abserv abscommon abcrypto absmath abai abipc abshared Detour DetourTileCache EASTL PugiXml)
if (ABX_SCENE_VIEWER)
  target_link_libraries(abserv GL freeglut_static glew)
endif(ABX_SCENE_VIEWER)
if (UNIX)
    target_link_libraries(abserv DeathHandler)
endif()

install(TARGETS abserv
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
