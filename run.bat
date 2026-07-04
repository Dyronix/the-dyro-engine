@echo off
setlocal

set "CONFIG=Debug"
set "CONFIG_ARG=-debug"
set "GAME=dyro_game"

:parse_args
if "%~1"=="" goto args_done

if /i "%~1"=="-debug" (
    set "CONFIG=Debug"
    set "CONFIG_ARG=-debug"
    shift
    goto parse_args
)

if /i "%~1"=="-release" (
    set "CONFIG=Release"
    set "CONFIG_ARG=-release"
    shift
    goto parse_args
)

rem cmd.exe splits "-game=name" into two args at the "=", so accept both
rem the quoted form ("-game=name" as a single %1) and the unquoted one.
if /i "%~1"=="-game" (
    if "%~2"=="" (
        echo Usage: %~nx0 [-debug^|-release] [-game=name]
        exit /b 1
    )
    set "GAME=%~2"
    shift
    shift
    goto parse_args
)

set "ARG=%~1"
if /i "%ARG:~0,6%"=="-game=" (
    set "GAME=%ARG:~6%"
    shift
    goto parse_args
)

echo Usage: %~nx0 [-debug^|-release] [-game=name]
exit /b 1

:args_done

set "EXE_DIR=%~dp0build\%CONFIG%"
set "EXE_PATH=%EXE_DIR%\%GAME%.exe"

if not exist "%EXE_PATH%" (
    set "EXE_PATH=%EXE_DIR%\%GAME%_d.exe"
)

if not exist "%EXE_PATH%" (
    echo Could not find %GAME%.exe in "%EXE_DIR%".
    echo Run build.bat %CONFIG_ARG% first.
    exit /b 1
)

pushd "%EXE_DIR%" >nul

"%EXE_PATH%"
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%
