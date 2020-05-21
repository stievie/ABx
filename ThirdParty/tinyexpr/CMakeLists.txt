project (tinyexpr C)

file(GLOB TINYEXPR_SOURCES      ./tinyexpr.c ./tinyexpr.h)

add_library(
    tinyexpr
    ${TINYEXPR_SOURCES}
)
