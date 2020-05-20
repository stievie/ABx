project (fw CXX)

include_directories(../Include/Urho3D)
include_directories(../Include/Urho3D/ThirdParty)
link_directories(../Lib/x64)
link_directories(../Lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Bullet will emit many warnings with -Wextra
    string(REPLACE "-Wextra" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    add_compile_options(-Wno-maybe-uninitialized -Wno-unused-parameter)
elseif(MSVC)
endif()

file(GLOB_RECURSE FW_SOURCES      abclient/*.cpp abclient/*.h)

add_executable(
    fw
    ${FW_SOURCES}
)

target_precompile_headers(fw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abclient/stdafx.h)

target_link_libraries(fw libabclient abshared abcrypto tinyexpr lz4 Urho3D)

if (UNIX)
    target_link_libraries(fw pthread GL ssl crypto rt dl)
endif()
