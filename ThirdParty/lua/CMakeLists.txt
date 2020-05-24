project (lua C)

file(GLOB LUA_SOURCES      lua/*.c lua/*.h)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/luac.c)

message(${LUA_SOURCES})
add_library(
    lua
    ${LUA_SOURCES}
)

if (MSVC)
    target_compile_options(lua PRIVATE /wd4244 /wd4702 /wd4324 /wd4310)
endif()
