@echo off
set VENVFILE=%1
set HOMEPATH=%~2

echo home = %HOMEPATH%> %VENVFILE%
echo include-system-site-packages = false>> %VENVFILE%

