cmake_minimum_required (VERSION 3.17)
project (DeathHandler CXX)

file(GLOB_RECURSE DEATHHANDLER_SOURCES      ./death_handler.cc ./death_handler.h)

add_library(
    DeathHandler
    ${DEATHHANDLER_SOURCES}
)
