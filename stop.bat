@echo off

taskkill /IM abserv.exe
taskkill /IM abmatch.exe
taskkill /IM ablogin.exe
taskkill /IM absadmin.exe
taskkill /IM abfile.exe
timeout /T 1 /nobreak >nul
taskkill /IM abmsgs.exe
timeout /T 1 /nobreak >nul
taskkill /IM abdata.exe
