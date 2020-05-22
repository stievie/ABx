project (abscommon CXX)

file(GLOB ABSCOMMON_SOURCES      abscommon/*.cpp abscommon/*.h)

add_library(
    abscommon
    ${ABSCOMMON_SOURCES}
)

target_precompile_headers(abscommon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/abscommon/stdafx.h)
if (ABX_WRITE_MINIBUMP)
  target_compile_definitions(abscommon PUBLIC WRITE_MINIBUMP)
endif(ABX_WRITE_MINIBUMP)

target_link_libraries(abscommon abcrypto)
if (WIN32)
    target_link_libraries(abscommon lua)
else (UNIX)
    target_link_libraries(abscommon pthread lua5.3 uuid)
endif()
target_include_directories(abscommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
