cmake_minimum_required (VERSION 3.17)
project (tinyexpr C)

file(GLOB_RECURSE TINYEXPR_SOURCES      ./tinyexpr.c ./tinyexpr.h)

add_library(
    tinyexpr
    ${TINYEXPR_SOURCES}
)
