project (libabclient CXX)

file(GLOB LIBABCLIENT_SOURCES      libabclient/*.cpp libabclient/*.h)

add_library(
    libabclient
    ${LIBABCLIENT_SOURCES}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(libabclient PUBLIC -Wno-unused-but-set-variable)
endif()

target_precompile_headers(libabclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libabclient/stdafx.h)

target_compile_definitions(libabclient PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT CPPHTTPLIB_ZLIB_SUPPORT)
target_link_libraries(libabclient lz4 z)
if (WIN32)
    target_include_directories(libabclient PUBLIC ${CMAKE_SOURCE_DIR}/Include/zlib)
    target_include_directories(libabclient PRIVATE ${CMAKE_SOURCE_DIR}/Include/openssl-1.0)
    target_link_libraries(libabclient libcrypto libssl)
else()
    target_link_libraries(libabclient ssl crypto rt dl)
endif()

target_include_directories(libabclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libabclient)
