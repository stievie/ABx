project (PDCurses C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE PDCURSES_SOURCES
    common/*.c common/*.h
    pdcurses/*.c pdcurses/*.h
    wincon/*.c wincon/*.h
)

add_library(
    PDCurses
    ${PDCURSES_SOURCES}
)

target_include_directories(PDCurses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
