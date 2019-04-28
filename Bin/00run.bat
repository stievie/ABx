@echo off

rem Start these only if they are not running
tasklist /nh /fi "imagename eq abdata.exe" | find /i "abdata.exe" > nul || (start abdata.bat)
sleep 1
tasklist /nh /fi "imagename eq abmsgs.exe" | find /i "abmsgs.exe" > nul || (start abmsgs.exe)
tasklist /nh /fi "imagename eq abfile.exe" | find /i "abfile.exe" > nul || (start abfile.exe)
tasklist /nh /fi "imagename eq ablogin.exe" | find /i "ablogin.exe" > nul || (start ablogin.exe)

start abserv.bat
rem start abserv.bat -conf abserv2.lua
rem start abserv.exe -conf abserv3.lua
