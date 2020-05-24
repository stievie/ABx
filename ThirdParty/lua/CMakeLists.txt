project (lua C)

file(GLOB_RECURSE LUA_SOURCES      lua/*.c lua/*.h)

add_library(
    lua
    ${LUA_SOURCES}
)

if (MSVC)
    target_compile_options(lua PRIVATE /wd4244 /wd4702 /wd4324)
endif()
