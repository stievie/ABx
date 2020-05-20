project (keygen CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE KEYGEN_SOURCES      keygen/*.cpp keygen/*.h)

add_executable(
    keygen
    ${KEYGEN_SOURCES}
)

target_precompile_headers(keygen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/keygen/stdafx.h)

target_link_libraries(keygen abscommon)
