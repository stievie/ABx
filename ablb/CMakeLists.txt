project (ablb CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE ABLB_SOURCES      ablb/*.cpp ablb/*.h)

add_executable(
    ablb
    ${ABLB_SOURCES}
)

target_link_libraries(ablb abscommon)
if (UNIX)
	target_link_libraries(ablb DeathHandler dl)
endif()
