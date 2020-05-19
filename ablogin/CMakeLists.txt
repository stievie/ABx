cmake_minimum_required (VERSION 3.17)
project (ablogin CXX)

file(GLOB_RECURSE ABLOGIN_SOURCES      ablogin/*.cpp ablogin/*.h)

add_executable(
    ablogin
    ${ABLOGIN_SOURCES}
)

target_link_libraries(ablogin abscommon)
