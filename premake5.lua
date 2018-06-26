-- https://github.com/lukaslaobeyer/libdrone/blob/master/premake5.lua

workspace "absall"
  configurations { "Debug", "Release" }
  location "build"
  includedirs { ".", "abscommon/abscommon", "Include", "$(BOOST_DIR)" }
  libdirs { "Lib", "Lib/%{cfg.platform}/%{cfg.buildcfg}", "$(BOOST_LIB_PATH)" }
  targetdir ("Bin")
  platforms { "x64" }
--  system "Windows"
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
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abscommon/abscommon/stdafx.cpp"
    filter "configurations:Debug"
      targetname "abscommon_d"
      
  project "abdata"
    kind "ConsoleApp"
    language "C++"
    files { "abdata/abdata/*.cpp", "abdata/abdata/*.h" }
    includedirs { "Include/pgsql", "Include/mysql" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abdata/abdata/stdafx.cpp"
    filter "configurations:Debug"
      targetname "abdata_d"
    
  project "abfile"
    kind "ConsoleApp"
    language "C++"
    files { "abfile/abfile/*.cpp", "abfile/abfile/*.h" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abfile/abfile/stdafx.cpp"
    filter "configurations:Debug"
      targetname "abfile_d"

  project "ablogin"
    kind "ConsoleApp"
    language "C++"
    files { "ablogin/ablogin/*.cpp", "ablogin/ablogin/*.h" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "ablogin/ablogin/stdafx.cpp"
    filter "configurations:Debug"
      targetname "ablogin_d"

  project "abmsgs"
    kind "ConsoleApp"
    language "C++"
    files { "abmsgs/abmsgs/*.cpp", "abmsgs/abmsgs/*.h" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abmsgs/abmsgs/stdafx.cpp"
    filter "configurations:Debug"
      targetname "abmsgs_d"

  project "abserv"
    kind "ConsoleApp"
    language "C++"
    files { "abserv/abserv/*.cpp", "abserv/abserv/*.h" }
    links { "abscommon" }
    dependson { "abscommon" }
    defines { "_CONSOLE" }
    pchheader "stdafx.h"
    filter "action:vs*"
      pchsource "abserv/abserv/stdafx.cpp"
    filter "configurations:Debug"
      targetname "abserv_d"
