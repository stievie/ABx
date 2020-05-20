project (ablogin CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABLOGIN_SOURCES      ablogin/*.cpp ablogin/*.h)

add_executable(
    ablogin
    ${ABLOGIN_SOURCES}
)

target_link_libraries(ablogin abscommon abshared)
if (UNIX)
	target_link_libraries(ablogin DeathHandler dl)
endif()
