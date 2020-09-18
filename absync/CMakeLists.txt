project (absync CXX)

file(GLOB SOURCES absync/*.cpp absync/*.h)

add_library(
    absync
    ${SOURCES}
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(absync PUBLIC -Wno-unused-but-set-variable)
endif()

target_compile_definitions(absync PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT CPPHTTPLIB_ZLIB_SUPPORT)
target_link_libraries(absync abcrypto PugiXml z)
if (WIN32)
    target_include_directories(absync PUBLIC ${CMAKE_SOURCE_DIR}/Include/zlib)
    target_include_directories(absync PUBLIC ${CMAKE_SOURCE_DIR}/Include/openssl-1.0)
    target_link_libraries(absync libcrypto libssl)
else()
    target_link_libraries(absync ssl crypto rt dl)
endif()
target_include_directories(absync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
