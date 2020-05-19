cmake_minimum_required (VERSION 3.17)
project (ABx C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    # Target Windows 7
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-D_UNICODE)
    add_definitions(-DUNICODE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /W4 /MP /Zc:__cplusplus /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c++17 /W4 /MP /utf-8")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c17 -Wall -Wextra -march=native")
endif(MSVC)

add_definitions(-DASIO_STANDALONE)

include_directories(Include)
include_directories(ThirdParty/EASTL/test/packages/EABase/include/Common)
if (WIN32)
    link_directories(Lib/x64)
endif()

option (ABX_ASSERT "Enable assert() in Release mode" TRUE)
option (ABX_SCENE_VIEWER "Compile with Scene viewer" FALSE)
option (ABX_PGSQL "Use PostreSQL Database backend" TRUE)
option (ABX_MYSQL "Use MySQL Database backend" FALSE)
option (ABX_ODBC "Use OODBC Database backend" FALSE)
option (ABX_SQLITE "Use SQLite Database backend" FALSE)

if (ABX_ASSERT)
  add_definitions(-DSA_ASSERT)
endif(ABX_ASSERT)

# Libraries
add_subdirectory(ThirdParty)
add_subdirectory(abai)
add_subdirectory(abdb)
add_subdirectory(abipc)
add_subdirectory(abscommon)
add_subdirectory(abshared)
add_subdirectory(absmath)

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
#add_subdirectory(dbgclient)
add_subdirectory(dbtool)
add_subdirectory(keygen)
add_subdirectory(Tests)

# Client
# TODO:
