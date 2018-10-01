-- Creates two solutions in ./build directory
--  1. absall: Solution with all servers. Binaries go to ./Bin
--  2. abclient: Solution with the game client. Binaries go to ./abclient/bin

-- Requirements
--  * BOOST_DIR, BOOST_LIB_PATH environment variables

--------------------------------------------------------------------------------
-- 3rd Party -------------------------------------------------------------------
--------------------------------------------------------------------------------
workspace "abs3rd"
  configurations { "Debug", "Release" }
  location "build"
  targetdir ("Bin")
  warnings "Extra"
  if (_TARGET_OS == "windows") then
    platforms { "x32", "x64" }
    filter "platforms:x64"
      architecture "x86_64"
    filter "platforms:x32"
      architecture "x86"
  elseif (_TARGET_OS == "linux") then
    platforms { "armv7" }
    filter "platforms:armv7"
      architecture "armv7"
  end
  
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"
  filter "configurations:Rel*"
    defines { "NDEBUG" }
    flags { "LinkTimeOptimization" }
    optimize "Full"
  filter "configurations:RelNoProfiling"
    defines { "_NPROFILING" }
    
  project "abcrypto"
    kind "StaticLib"
    language "C"
    files { 
      "ThirdParty/abcrypto/*.c",
      "ThirdParty/abcrypto/*.h",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
    if (_TARGET_OS == "linux") then
      defines { "CC_TARGET_PLATFORM=5" }
    end
    filter { "action:gmake*", "toolset:gcc" }
      buildoptions { "-std=c11" }

  project "lua"
    kind "StaticLib"
    language "C"
    files { 
      "ThirdParty/lua/lua/*.c",
      "ThirdParty/lua/lua/*.h",
    }
    removefiles { 
      "ThirdParty/lua/lua/lua.c", 
      "ThirdParty/lua/lua/luac.c" 
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
    filter { "action:gmake*", "toolset:gcc" }
      buildoptions { "-std=c11" }

  project "sqlite3"
    kind "StaticLib"
    language "C"
    files { 
      "ThirdParty/sqlite3/*.c",
      "ThirdParty/sqlite3/*.h",
    }
    removefiles { 
      "ThirdParty/sqlite3/shell.c"
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
    filter { "action:gmake*", "toolset:gcc" }
      buildoptions { "-std=c11" }
    
--------------------------------------------------------------------------------
-- Server ----------------------------------------------------------------------
--------------------------------------------------------------------------------
workspace "absall"
  configurations { "Debug", "Release", "RelNoProfiling" }
  location "build"
  system ("windows")
  includedirs { ".", "abscommon/abscommon", "Include", "$(BOOST_DIR)" }
  libdirs { "Lib", "Lib/%{cfg.platform}/%{cfg.buildcfg}", "$(BOOST_LIB_PATH)" }
  targetdir ("Bin")
  cppdialect "C++14"
  warnings "Extra"

  if (_TARGET_OS == "windows") then
    platforms { "x64" }
  elseif (_TARGET_OS == "linux") then
    platforms { "x32", "x64", "armv7" }
  end
  filter "platforms:x64"
    architecture "x64"
  filter "platforms:x32"
    architecture "x32"
  if (_TARGET_OS == "linux") then
    filter { "system:linux", "platforms:armv7" }
      architecture "armv7"
  end
    
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"
  filter "configurations:Rel*"
    defines { "NDEBUG" }
    flags { "LinkTimeOptimization" }
    optimize "Full"
  filter "configurations:RelNoProfiling"
    defines { "_NPROFILING" }
  
  project "abscommon"
    kind "StaticLib"
    language "C++"
    links { "abcrypto", "lua" }
    dependson { "abcrypto", "lua" }
    files { 
      "abscommon/abscommon/*.cpp",
      "abscommon/abscommon/*.cxx",
      "abscommon/abscommon/*.c",
      "abscommon/abscommon/*.h",
      "abscommon/abscommon/*.hpp",
      "abscommon/abscommon/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abscommon/abscommon/stdafx.cpp"
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"

  project "absmath"
    kind "StaticLib"
    language "C++"
    files { 
      "absmath/absmath/*.cpp",
      "absmath/absmath/*.cxx",
      "absmath/absmath/*.c",
      "absmath/absmath/*.h",
      "absmath/absmath/*.hpp",
      "absmath/absmath/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "absmath/absmath/stdafx.cpp"
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
          
  project "abdata"
    kind "ConsoleApp"
    language "C++"
    files { 
      "abdata/abdata/*.cpp",
      "abdata/abdata/*.c",
      "abdata/abdata/*.cxx",
      "abdata/abdata/*.h",
      "abdata/abdata/*.hpp",
      "abdata/abdata/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    includedirs { "Include/pgsql" }
    links { "abscommon", "abcrypto", "lua", "sqlite3" }
    dependson { "abscommon", "abcrypto", "lua", "sqlite3" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abdata/abdata/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"
    
  project "abfile"
    kind "ConsoleApp"
    language "C++"
    files { 
      "abfile/abfile/*.cpp",
      "abfile/abfile/*.c",
      "abfile/abfile/*.cxx",
      "abfile/abfile/*.h",
      "abfile/abfile/*.hpp",
      "abfile/abfile/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "abcrypto", "lua" }
    dependson { "abscommon", "abcrypto", "lua" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abfile/abfile/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "ablogin"
    kind "ConsoleApp"
    language "C++"
    files { 
      "ablogin/ablogin/*.cpp",
      "ablogin/ablogin/*.c",
      "ablogin/ablogin/*.cxx",
      "ablogin/ablogin/*.h",
      "ablogin/ablogin/*.hpp",
      "ablogin/ablogin/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "abcrypto", "lua" }
    dependson { "abscommon", "abcrypto", "lua" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "ablogin/ablogin/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "ablb"
    kind "ConsoleApp"
    language "C++"
    files { 
      "ablb/ablb/*.cpp",
      "ablb/ablb/*.c",
      "ablb/ablb/*.cxx",
      "ablb/ablb/*.h",
      "ablb/ablb/*.hpp",
      "ablb/ablb/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "abcrypto", "lua" }
    dependson { "abscommon", "abcrypto", "lua" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "ablb/ablb/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "abmsgs"
    kind "ConsoleApp"
    language "C++"
    files { 
      "abmsgs/abmsgs/*.cpp",
      "abmsgs/abmsgs/*.c",
      "abmsgs/abmsgs/*.cxx",
      "abmsgs/abmsgs/*.h",
      "abmsgs/abmsgs/*.hpp",
      "abmsgs/abmsgs/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "abcrypto", "lua" }
    dependson { "abscommon", "abcrypto", "lua" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abmsgs/abmsgs/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  project "abserv"
    kind "ConsoleApp"
    language "C++"
    includedirs { "absmath/absmath" }
    files { 
      "abserv/abserv/*.cpp",
      "abserv/abserv/*.c",
      "abserv/abserv/*.cxx",
      "abserv/abserv/*.h",
      "abserv/abserv/*.hpp",
      "abserv/abserv/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "absmath", "abcrypto", "lua" }
    dependson { "abscommon", "absmath", "abcrypto", "lua" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abserv/abserv/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  -- Import utility. Binary import/bin
  project "import"
    kind "ConsoleApp"
    language "C++"
    targetdir ("import/bin")
    includedirs { "Include/stb", "absmath/absmath" }
    files { 
      "import/import/*.cpp",
      "import/import/*.c",
      "import/import/*.cxx",
      "import/import/*.h",
      "import/import/*.hpp",
      "import/import/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "absmath" }
    dependson { "abscommon", "absmath" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "import/import/stdafx.cpp"
    filter "configurations:Debug"
      targetsuffix "_d"

  -- Navigation mesh generator. Binary genavmesh/bin
  project "genavmesh"
    kind "ConsoleApp"
    language "C++"
    targetdir ("genavmesh/bin")
    includedirs {  "genavmesh/genavmesh/DebugUtils/Include", "genavmesh/genavmesh/Detour/Include", 
      "genavmesh/genavmesh/DetourCrowd/Include", "genavmesh/genavmesh/DetourTileCache/Include", 
      "genavmesh/genavmesh/Recast/Include", "Include/stb", "absmath/absmath" }
    files { 
      "genavmesh/genavmesh/**.cpp",
      "genavmesh/genavmesh/**.c",
      "genavmesh/genavmesh/**.cxx",
      "genavmesh/genavmesh/**.h",
      "genavmesh/genavmesh/**.hpp",
      "genavmesh/genavmesh/**.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    links { "abscommon", "absmath" }
    dependson { "abscommon", "absmath" }
    defines { 
      "_CONSOLE", 
      "_CRT_SECURE_NO_WARNINGS"     -- fopen()
    }
    filter "configurations:Debug"
      targetsuffix "_d"

--------------------------------------------------------------------------------
-- Client ----------------------------------------------------------------------
--------------------------------------------------------------------------------
workspace "abclient"
  configurations { "Debug", "Release", "RelWithSymbols" }
  location "build"
  includedirs { "Include" }
  libdirs { "Lib", "Lib/%{cfg.platform}/%{cfg.buildcfg}" }
  platforms { "x64" }
  filter { "platforms:x64" }
    architecture "x64"
  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"
  filter "configurations:RelWithSymbols"
    symbols "On"
    libdirs { "Lib/%{cfg.platform}/Release" }
  filter "configurations:Release or configurations:RelWithSymbols"
    defines { "NDEBUG" }
    flags { "LinkTimeOptimization" }
    optimize "Full"

  project "libabclient"
    kind "StaticLib"
    language "C++"
    targetdir "Lib/%{cfg.platform}/%{cfg.buildcfg}"
    defines { "ASIO_STANDALONE" }
    files { 
      "libabclient/libabclient/*.cpp",
      "libabclient/libabclient/*.c",
      "libabclient/libabclient/*.cxx",
      "libabclient/libabclient/*.h",
      "libabclient/libabclient/*.hpp",
      "libabclient/libabclient/*.hxx",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
    }
    warnings "Extra"
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "libabclient/libabclient/stdafx.cpp"

  project "abclient"
    kind "WindowedApp"
    includedirs { "libabclient/libabclient", "Include/Urho3D/ThirdParty" }
    defines { "URHO3D_MINIDUMPS", "URHO3D_FILEWATCHER", "URHO3D_PROFILING", "URHO3D_LOGGING", "URHO3D_THREADING", "URHO3D_ANGELSCRIPT", "URHO3D_LUA", "TOLUA_RELEASE", "URHO3D_NAVIGATION", "URHO3D_NETWORK", "URHO3D_PHYSICS", "URHO3D_URHO2D" }
    links { "libabclient", "dbghelp", "d3dcompiler", "d3d11", "dxgi", "dxguid", "winmm", "imm32", "version" }
    dependson { "libabclient" }
    language "C++"
    files { 
      "abclient/abclient/*.cpp",
      "abclient/abclient/*.c",
      "abclient/abclient/*.cxx",
      "abclient/abclient/*.h",
      "abclient/abclient/*.hpp",
      "abclient/abclient/*.hxx",
      "abclient/abclient/*.rc",
      "abclient/abclient/*.ico",
    }
    vpaths {
      ["Header Files"] = {"**.h", "**.hpp", "**.hxx"},
      ["Source Files"] = {"**.cpp", "**.c", "**.cxx"},
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
    filter "configurations:RelWithSymbols"
      targetsuffix "_r"
    filter "configurations:Release or configurations:RelWithSymbols"
      links { "Urho3D" }
