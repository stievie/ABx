@echo off

cd Bin

rem Start these only if they are not running
tasklist /nh /fi "imagename eq abdata.exe" | find /i "abdata.exe" > nul || (start abdata.exe)
timeout /t 1
tasklist /nh /fi "imagename eq abmsgs.exe" | find /i "abmsgs.exe" > nul || (start abmsgs.exe)
timeout /t 1
tasklist /nh /fi "imagename eq abfile.exe" | find /i "abfile.exe" > nul || (start abfile.exe)
tasklist /nh /fi "imagename eq ablogin.exe" | find /i "ablogin.exe" > nul || (start ablogin.exe)

tasklist /nh /fi "imagename eq abserv.exe" | find /i "abserv.exe" > nul || (start abserv.bat)

rem start abserv.bat
rem start abserv.bat -conf abserv2.lua
rem start abserv.exe -conf abserv3.lua

cd ..

