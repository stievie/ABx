project (fw CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Bullet will emit many warnings with -Wextra
    string(REPLACE "-Wextra" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    add_compile_options(-Wno-maybe-uninitialized -Wno-unused-parameter)
elseif(MSVC)
endif()

file(GLOB FW_SOURCES
    abclient/*.cpp abclient/*.h)

add_executable(
    fw
    ${FW_SOURCES}
)

if (URHO3D_INCLUDE_DIRS)
    target_include_directories(fw PRIVATE ${URHO3D_INCLUDE_DIRS})
else()
    target_include_directories(fw PRIVATE ${CMAKE_SOURCE_DIR}/Include/Urho3D)
    target_include_directories(fw PRIVATE ${CMAKE_SOURCE_DIR}/Include/Urho3D/ThirdParty)
    target_include_directories(fw PRIVATE ${CMAKE_SOURCE_DIR}/Include/Urho3D/ThirdParty/SDL)
endif()
target_link_directories(fw PRIVATE ${CMAKE_SOURCE_DIR}/Lib/x64)
target_link_directories(fw PRIVATE ${CMAKE_SOURCE_DIR}/Lib)

set_property(TARGET fw PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_precompile_headers(fw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abclient/stdafx.h)

target_link_libraries(fw libabclient abshared abcrypto tinyexpr)
if (URHO3D_LIBRARIES)
    target_link_libraries(fw ${URHO3D_LIBRARIES})
else()
    target_link_libraries(fw Urho3D)
endif()

if (WIN32)
    target_link_libraries(fw libeay32 ssleay32)
else()
    target_link_libraries(fw pthread GL)
endif()

install(TARGETS fw
    RUNTIME DESTINATION abclient/bin
    COMPONENT runtime
)
