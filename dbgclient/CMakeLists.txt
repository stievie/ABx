cmake_minimum_required (VERSION 3.17)
project (dbgclient CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Bin)

file(GLOB_RECURSE DBGCLIENT_SOURCES      dbgclient/*.cpp dbgclient/*.h)

add_executable(
    dbgclient
    ${DBGCLIENT_SOURCES}
)

target_link_libraries(dbgclient abscommon abipc abai )
if (UNIX)
    target_link_libraries(dbgclient ncurses panel)
elseif(WIN32)
    target_link_libraries(dbgclient PDCurses)
endif()
