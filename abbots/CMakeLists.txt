project (abbots CXX)

file(GLOB ABBOTS_SOURCES      abbots/*.cpp abbots/*.h)

add_executable(
    abbots
    ${ABBOTS_SOURCES}
)

target_precompile_headers(abbots PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abbots/stdafx.h)
set_property(TARGET abbots PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(abbots abscommon absmath libabclient)
if (WIN32)
    target_link_libraries(abbots libcrypto libssl)
else()
    target_link_libraries(abbots DeathHandler)
endif()

install(TARGETS abbots
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
