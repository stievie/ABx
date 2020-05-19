cmake_minimum_required (VERSION 3.17)
project (dbgclient CXX)

file(GLOB_RECURSE DBGCLIENT_SOURCES      dbgclient/*.cpp dbgclient/*.h)

add_executable(
    dbgclient
    ${DBGCLIENT_SOURCES}
)

target_link_libraries(dbgclient abipc)
