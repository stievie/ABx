project (absadmin CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABSADMIN_SOURCES      absadmin/*.cpp absadmin/*.h)

add_executable(
    absadmin
    ${ABSADMIN_SOURCES}
)

target_link_libraries(absadmin abscommon less PugiXml)
if (WIN32)
    include_directories(../Include/openssl-1.0)
    target_link_libraries(absadmin libeay32 ssleay32)
else()
    target_link_libraries(absadmin DeathHandler ssl crypto)
endif()
