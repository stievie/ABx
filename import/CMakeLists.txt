project (import CXX)

include_directories(../Include/stb)

file(GLOB IMPORT_SOURCES      import/*.cpp import/*.h)

add_executable(
    import
    ${IMPORT_SOURCES}
)

target_precompile_headers(import PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/import/stdafx.h)

target_include_directories(import PRIVATE ${assimp_INCLUDE_DIRS})
target_link_libraries(import absmath abscommon ${assimp_LIBRARIES} EASTL)

install(TARGETS import
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)
