project (lua C)

file(GLOB LUA_SOURCES lua/*.c lua/*.h)
# Remove programs we just need the library
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c)
list(REMOVE_ITEM LUA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lua/luac.c)

if (UNIX)
    add_definitions(-DLUA_USE_LINUX -DLUA_COMPAT_5_2)
endif()

add_library(
    lua
    ${LUA_SOURCES}
)

if (UNIX)
    target_link_libraries(lua dl readline)
endif()

if (MSVC)
    target_compile_options(lua PRIVATE /wd4244 /wd4702 /wd4324 /wd4310)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(lua PRIVATE -Wno-stringop-overflow)
endif()
