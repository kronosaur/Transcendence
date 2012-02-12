@echo off
if "%1"=="" echo Copies source files to destination
if "%1"=="" echo.
if "%1"=="" echo CopyTo {destination}
if "%1"=="" echo.
if "%1"=="" echo e.g.: copyToArchives Transcendence096
if "%1"=="" goto :EOF

xcopy . "..\Archives\%1" /s/d/i/r/y /EXCLUDE:BackupExclude.txt
