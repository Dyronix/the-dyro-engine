@echo off
setlocal

set "PRESET=debug"

if not "%~2"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

if /i "%~1"=="-debug" (
    set "PRESET=debug"
) else if /i "%~1"=="-release" (
    set "PRESET=release"
) else if not "%~1"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

pushd "%~dp0" >nul

cmake --build --preset %PRESET%
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%
