project (libabclient CXX)

file(GLOB_RECURSE LIBABCLIENT_SOURCES      libabclient/*.cpp libabclient/*.h)

add_library(
    libabclient
    ${LIBABCLIENT_SOURCES}
)

target_include_directories(libabclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libabclient)
