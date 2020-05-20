project (libabclient CXX)

file(GLOB_RECURSE LIBABCLIENT_SOURCES      libabclient/*.cpp libabclient/*.h)

add_library(
    libabclient
    ${LIBABCLIENT_SOURCES}
)

target_precompile_headers(libabclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libabclient/stdafx.h)

target_include_directories(libabclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libabclient)
