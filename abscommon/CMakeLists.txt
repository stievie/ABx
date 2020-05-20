project (abscommon CXX)

file(GLOB_RECURSE ABSCOMMON_SOURCES      abscommon/*.cpp abscommon/*.h)

add_library(
    abscommon
    ${ABSCOMMON_SOURCES}
)

target_link_libraries(abscommon lua abcrypto)
if (UNIX)
	target_link_libraries(abscommon pthread uuid)
endif()
target_include_directories(abscommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
