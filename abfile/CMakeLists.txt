project (abfile CXX)

file(GLOB ABFILE_SOURCES      abfile/*.cpp abfile/*.h)

add_executable(
    abfile
    ${ABFILE_SOURCES}
)

target_precompile_headers(abfile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abfile/stdafx.h)
set_property(TARGET abfile PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(abfile abscommon abcrypto PugiXml)
if (WIN32)
    include_directories(../Include/openssl-1.0)
    target_link_libraries(abfile libcrypto libssl, zlib)
else()
    target_link_libraries(abfile DeathHandler ssl crypto z)
endif()

install(TARGETS abfile
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
