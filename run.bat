@echo off
setlocal

set "CONFIG=Debug"
set "CONFIG_ARG=-debug"

if not "%~2"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

if /i "%~1"=="-debug" (
    set "CONFIG=Debug"
    set "CONFIG_ARG=-debug"
) else if /i "%~1"=="-release" (
    set "CONFIG=Release"
    set "CONFIG_ARG=-release"
) else if not "%~1"=="" (
    echo Usage: %~nx0 [-debug^|-release]
    exit /b 1
)

set "EXE_DIR=%~dp0build\%CONFIG%"
set "EXE_PATH=%EXE_DIR%\dyro_game.exe"

if not exist "%EXE_PATH%" (
    set "EXE_PATH=%EXE_DIR%\dyro_game_d.exe"
)

if not exist "%EXE_PATH%" (
    echo Could not find dyro_game.exe in "%EXE_DIR%".
    echo Run build.bat %CONFIG_ARG% first.
    exit /b 1
)

pushd "%EXE_DIR%" >nul

"%EXE_PATH%"
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%
