cmake_minimum_required (VERSION 3.17)
project (import CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

include_directories(../Include/stb)

file(GLOB_RECURSE IMPORT_SOURCES      import/*.cpp import/*.h)

add_executable(
    import
    ${IMPORT_SOURCES}
)

target_link_libraries(import absmath abscommon assimp EASTL)
