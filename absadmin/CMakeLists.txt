project (absadmin CXX)

file(GLOB ABSADMIN_SOURCES      absadmin/*.cpp absadmin/*.h)

add_executable(
    absadmin
    ${ABSADMIN_SOURCES}
)

target_precompile_headers(absadmin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/absadmin/stdafx.h)
set_property(TARGET absadmin PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(absadmin abscommon less PugiXml)
if (WIN32)
    include_directories(../Include/openssl-1.0)
    target_link_libraries(absadmin libcrypto libssl)
else()
    target_link_libraries(absadmin DeathHandler ssl crypto)
endif()

install(TARGETS absadmin
    RUNTIME DESTINATION bin
    COMPONENT runtime
)
