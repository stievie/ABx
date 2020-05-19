cmake_minimum_required (VERSION 3.17)
project (absadmin CXX)

file(GLOB_RECURSE ABSADMIN_SOURCES      absadmin/*.cpp absadmin/*.h)

add_executable(
    absadmin
    ${ABSADMIN_SOURCES}
)

if (WIN32)
  include_directories(../Include/openssl-1.0)
endif()

target_link_libraries(absadmin abscommon abcrypto lua less PugiXml libeay32 ssleay32)
