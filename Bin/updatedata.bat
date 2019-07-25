@echo off

setlocal
:PROMPT
SET /P AREYOUSURE=Update data directory from /cygdrive/g/abx/data (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

"..\Tools\rsync\rsync" -av --delete /cygdrive/g/abx/data "./"

:END
endlocal
