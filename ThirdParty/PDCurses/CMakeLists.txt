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

if (MSVC)
    target_compile_options(PDCurses PRIVATE /wd4100 /wd4244 /wd4245 /wd4267 /wd4996)
endif()

target_include_directories(PDCurses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
