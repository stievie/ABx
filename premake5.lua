-- Creates two solutions in ./build directory
--  1. absall: Solution with all servers. Binaries go to ./Bin
--  2. abclient: Solution with the game client. Binaries go to ./abclient/bin

-- Requirements
--  * BOOST_DIR, BOOST_LIB_PATH environment variables

--------------------------------------------------------------------------------
-- Server ----------------------------------------------------------------------
--------------------------------------------------------------------------------
workspace "absall"
  configurations { "Debug", "Release" }
  location "build"
  includedirs { ".", "abscommon/abscommon", "Include", "$(BOOST_DIR)" }
  libdirs { "Lib", "Lib/%{cfg.platform}/%{cfg.buildcfg}", "$(BOOST_LIB_PATH)" }
  targetdir ("Bin")
  platforms { "x64" }
  warnings "Extra"
  filter { "platforms:x64" }
    architecture "x64"
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"
  filter "configurations:Release"
    defines { "NDEBUG" }
    flags { "LinkTimeOptimization" }
    optimize "Full"
  
  project "abscommon"
    kind "StaticLib"
    language "C++"
    files { "abscommon/abscommon/*.cpp", "abscommon/abscommon/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abscommon/abscommon/stdafx.cpp"
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
      
  project "abdata"
    kind "ConsoleApp"
    language "C++"
    files { "abdata/abdata/*.cpp", "abdata/abdata/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    includedirs { "Include/pgsql", "Include/mysql" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abdata/abdata/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"
    
  project "abfile"
    kind "ConsoleApp"
    language "C++"
    files { "abfile/abfile/*.cpp", "abfile/abfile/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abfile/abfile/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "ablogin"
    kind "ConsoleApp"
    language "C++"
    files { "ablogin/ablogin/*.cpp", "ablogin/ablogin/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "ablogin/ablogin/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "abmsgs"
    kind "ConsoleApp"
    language "C++"
    files { "abmsgs/abmsgs/*.cpp", "abmsgs/abmsgs/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abmsgs/abmsgs/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "abserv"
    kind "ConsoleApp"
    language "C++"
    files { "abserv/abserv/*.cpp", "abserv/abserv/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abserv/abserv/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

--------------------------------------------------------------------------------
-- Client ----------------------------------------------------------------------
--------------------------------------------------------------------------------
workspace "abclient"
  configurations { "Debug", "Release" }
  location "build"
  includedirs { "Include" }
  libdirs { "Lib", "Lib/%{cfg.platform}/%{cfg.buildcfg}" }
  platforms { "x64" }
  filter { "platforms:x64" }
    architecture "x64"
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"
  filter "configurations:Release"
    defines { "NDEBUG" }
    flags { "LinkTimeOptimization" }
    optimize "Full"

  project "libabclient"
    kind "StaticLib"
    language "C++"
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
    defines { "ASIO_STANDALONE" }
    files { "libabclient/libabclient/*.cpp", "libabclient/libabclient/*.h" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
    }
    warnings "Extra"
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "libabclient/libabclient/stdafx.cpp"

  project "abclient"
    kind "WindowedApp"
    includedirs { "libabclient/libabclient", "Include/Box2D", "Include/Bullet", "Include/Urho3D/ThirdParty", "Include/Urho3D/ThirdParty/Bullet", "Include/Urho3D/ThirdParty/Lua" }
    defines { "URHO3D_MINIDUMPS", "URHO3D_FILEWATCHER", "URHO3D_PROFILING", "URHO3D_LOGGING", "URHO3D_THREADING", "URHO3D_ANGELSCRIPT", "URHO3D_LUA", "TOLUA_RELEASE", "URHO3D_NAVIGATION", "URHO3D_NETWORK", "URHO3D_PHYSICS", "URHO3D_URHO2D" }
    links { "libabclient", "dbghelp", "d3dcompiler", "d3d11", "dxgi", "dxguid", "winmm", "imm32", "version" }
    dependson { "libabclient" }
    language "C++"
    files { "abclient/abclient/*.cpp", "abclient/abclient/*.h", "abclient/abclient/*.rc", "abclient/abclient/*.ico" }
    vpaths {
      ["Header Files"] = {"**.h"},
      ["Source Files"] = {"**.cpp"},
      ["Resource Files"] = {"**.rc", "**.rc2", "**.manifest", "**.bmp", "**.cur", "**.ico"},
    }
    pchheader "stdafx.h"
    targetname "fw"
    filter "action:vs*"
      pchsource "abclient/abclient/stdafx.cpp"
    targetdir "abclient/bin"
    filter "configurations:Debug"
      targetsuffix "_d"
      links { "Urho3D_d" }
    filter "configurations:Release"
      links { "Urho3D" }
