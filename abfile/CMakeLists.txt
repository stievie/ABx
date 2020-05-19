cmake_minimum_required (VERSION 3.17)
project (abfile CXX)

file(GLOB_RECURSE ABFILE_SOURCES      abfile/*.cpp abfile/*.h)

add_executable(
    abfile
    ${ABFILE_SOURCES}
)

if (WIN32)
  include_directories(../Include/openssl-1.0)
endif()

target_link_libraries(abfile abscommon abcrypto lua PugiXml libeay32 ssleay32)
