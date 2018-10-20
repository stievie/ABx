@echo off

rem Copies a backup to the local PostreSQL server

rem DROP all existing tables
FOR /f "tokens=2 delims=|" %%G IN ('c:\PROGRA~1\POSTGR~1\11\bin\psql.exe --host localhost --username postgres --command="\dt" forgottenwars') DO (
   c:\PROGRA~1\POSTGR~1\11\bin\psql.exe --host localhost --username postgres --command="DROP table if exists %%G cascade" forgottenwars
   echo table %%G dropped
)

rem Get latest backup directory
for /f "delims=" %%a in ('dir /b /ad-h /od "i:\bak\pgsql\*"') do set "latestDir=%%~a"

rem Import last backup
c:\PROGRA~1\POSTGR~1\11\bin\psql.exe -U postgres -d forgottenwars -f i:\bak\pgsql\%latestDir%\forgottenwars.sql
