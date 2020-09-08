project (absync CXX)

file(GLOB SOURCES absync/*.cpp absync/*.h)

add_library(
    absync
    ${SOURCES}
)

target_link_libraries(absync abcrypto)
if (WIN32)
    target_include_directories(absync PUBLIC ../Include/openssl-1.0)
    target_link_libraries(absync libeay32 ssleay32)
else()
    target_link_libraries(absync ssl crypto rt dl)
endif()
target_include_directories(absync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
