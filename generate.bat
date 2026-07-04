@echo off
setlocal

pushd "%~dp0" >nul

cmake --preset vs2022
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%
