@echo off

setlocal
:PROMPT
SET /P AREYOUSURE=Update admin directory from /cygdrive/g/abx/admin (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

"..\Tools\rsync\rsync" -av --delete /cygdrive/g/abx/admin "./"

:END
endlocal
