project (genavmesh CXX)

if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-reorder -Wimplicit-fallthrough=0)
endif()
include_directories(../Include/stb)

file(GLOB GENAVMESH_SOURCES      genavmesh/*.cpp genavmesh/*.h)

add_executable(
    genavmesh
    ${GENAVMESH_SOURCES}
)

target_precompile_headers(import PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/genavmesh/stdafx.h)

target_link_libraries(genavmesh absmath abscommon assimp EASTL DebugUtils Recast Detour DetourTileCache)

install(TARGETS genavmesh
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
