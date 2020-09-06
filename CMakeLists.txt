cmake_minimum_required (VERSION 3.16)
project (ABx C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

# Set default config for Ninja Multi-Config. Possible values: Debug;Release;RelWithDebInfo
if (CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    if(NOT CMAKE_DEFAULT_BUILD_TYPE)
        set(CMAKE_DEFAULT_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    endif()
endif()

include(CTest)
find_package(Lua 5.3 EXACT)

if(LUA_FOUND)
    set(LUA_VERSION_SHORT "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}")
endif()

if (UNIX)
add_custom_target(run
    COMMAND ${CMAKE_SOURCE_DIR}/run
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_custom_target(stop
    COMMAND ${CMAKE_SOURCE_DIR}/stop
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
endif(UNIX)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    # Target Windows 7
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /Zc:__cplusplus /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /MP /utf-8")
elseif ((CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -march=native -fdiagnostics-color=always -fuse-ld=gold")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -march=native -fdiagnostics-color=always -fuse-ld=gold")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_EXECUTABLE})
endif(CCACHE_EXECUTABLE)

add_definitions(-DASIO_STANDALONE)

include_directories(${CMAKE_SOURCE_DIR}/Include)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/EASTL/test/packages/EABase/include/Common)
if (WIN32)
    link_directories(Lib/x64)
endif()

option (ABX_ASSERT "Enable assert() in Release mode" TRUE)
option (ABX_SCENE_VIEWER "Compile Game Server with Scene viewer" FALSE)
option (ABX_PGSQL "Use PostgreSQL Database backend" TRUE)
# The other DB backends probably won't work
#option (ABX_MYSQL "Use MySQL Database backend" FALSE)
#option (ABX_ODBC "Use OODBC Database backend" FALSE)
#option (ABX_SQLITE "Use SQLite Database backend" FALSE)
if (WIN32)
    option (ABX_WINDOWS_SERVICE "Build Windows Service executeables" FALSE)
endif()
option (ABX_NO_PROFILIING "No Profiling" FALSE)
option (ABX_WRITE_MINIBUMP "Write Minidump and Stacktrace in case of crash" TRUE)
option (ABX_BUILD_TOOLS "Build Tools (may require assimp to be installed)" FALSE)
option (ABX_BUILD_CLIENT "Build Game Client (requires Urho3D to be installed)" FALSE)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-DNOMINMAX)
endif()

if (ABX_ASSERT)
    add_definitions(-DSA_ASSERT)
endif(ABX_ASSERT)
if (ABX_WINSERVICE)
    add_definitions(-DWIN_SERVICE)
endif(ABX_WINSERVICE)
if (ABX_NO_PROFILIING)
    add_definitions(-DNPROFILING)
endif(ABX_NO_PROFILIING)

# Libraries
add_subdirectory(ThirdParty)
add_subdirectory(abai)
add_subdirectory(abdb)
add_subdirectory(abipc)
add_subdirectory(abscommon)
add_subdirectory(abshared)
add_subdirectory(absmath)
add_subdirectory(libabclient)

# Servers
add_subdirectory(abdata)
add_subdirectory(abfile)
add_subdirectory(ablb)
add_subdirectory(ablogin)
add_subdirectory(abmsgs)
add_subdirectory(abmatch)
add_subdirectory(absadmin)
add_subdirectory(abserv)

# Tools
add_subdirectory(abbots)
add_subdirectory(dbgclient)
add_subdirectory(dbtool)
add_subdirectory(keygen)
if(ABX_BUILD_TOOLS)
    include(Findassimp)
    if (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
        add_subdirectory(import)
        add_subdirectory(genavmesh)
    else()
        message(WARNING "Can not find assimp, Tools are not built")
    endif()
endif()
if(BUILD_TESTING)
    add_subdirectory(abtests)
endif()

# Client
if(ABX_BUILD_CLIENT)
    add_subdirectory(abclient)
endif()
