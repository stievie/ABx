cmake_minimum_required (VERSION 3.17)
project (ablb CXX)

file(GLOB_RECURSE ABLB_SOURCES      ablb/*.cpp ablb/*.h)

add_executable(
    ablb
    ${ABLB_SOURCES}
)

target_link_libraries(ablb abscommon)
