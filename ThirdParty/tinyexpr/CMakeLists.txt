project (tinyexpr C)

file(GLOB_RECURSE TINYEXPR_SOURCES      ./tinyexpr.c ./tinyexpr.h)

add_library(
    tinyexpr
    ${TINYEXPR_SOURCES}
)
