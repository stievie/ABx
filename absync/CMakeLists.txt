project (absync CXX)

file(GLOB SOURCES absync/*.cpp absync/*.h)

add_library(
    absync
    ${SOURCES}
)

target_link_libraries(absync abcrypto)
target_include_directories(absync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
