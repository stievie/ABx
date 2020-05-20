project (lua C)

file(GLOB_RECURSE LUA_SOURCES      lua/*.c lua/*.h)

add_library(
    lua
    ${LUA_SOURCES}
)
