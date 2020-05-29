@echo off

rem generic Game server
mode con:cols=150 lines=50
call abserv.exe -conf abserv.lua -id 00000000-0000-0000-0000-000000000000 -name generic -port 0 -autoterm
