project (libabclient CXX)

file(GLOB LIBABCLIENT_SOURCES      libabclient/*.cpp libabclient/*.h)

add_library(
    libabclient
    ${LIBABCLIENT_SOURCES}
)

target_precompile_headers(libabclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libabclient/stdafx.h)

target_link_libraries(libabclient lz4)
if (WIN32)
    target_include_directories(libabclient PRIVATE ../Include/openssl-1.0)
    target_link_libraries(libabclient libcrypto libssl)
else()
    target_link_libraries(libabclient ssl crypto rt dl)
endif()

target_include_directories(libabclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libabclient)
