@echo off

setlocal
:PROMPT
SET /P AREYOUSURE=Update file server root from /cygdrive/g/abx/file_root (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

"..\Tools\rsync\rsync" -av --delete /cygdrive/g/abx/file_root "./"

:END
endlocal
