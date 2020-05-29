project (genavmesh CXX)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-reorder -Wimplicit-fallthrough=0)
endif()

file(GLOB GENAVMESH_SOURCES      genavmesh/*.cpp genavmesh/*.h)

add_executable(
    genavmesh
    ${GENAVMESH_SOURCES}
)

target_precompile_headers(genavmesh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/genavmesh/stdafx.h)

target_include_directories(genavmesh PRIVATE ../Include/stb ${assimp_INCLUDE_DIRS})
target_link_libraries(genavmesh absmath abscommon ${assimp_LIBRARIES} EASTL DebugUtils Recast Detour DetourTileCache)

install(TARGETS genavmesh
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
