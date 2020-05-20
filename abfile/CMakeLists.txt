project (abfile CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABFILE_SOURCES      abfile/*.cpp abfile/*.h)

add_executable(
    abfile
    ${ABFILE_SOURCES}
)

target_link_libraries(abfile abscommon abcrypto lua PugiXml)
if (WIN32)
 	include_directories(../Include/openssl-1.0)
	target_link_libraries(abfile libeay32 ssleay32)
else()
	target_link_libraries(abfile DeathHandler PugiXml dl ssl crypto)
endif()
