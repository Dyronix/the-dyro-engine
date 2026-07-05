@echo off
setlocal enabledelayedexpansion

set "SCRIPT=%~nx0"

rem Default to Visual Studio 2026; -2022 selects the older toolset.
set "PRESET=vs2026"
set "TARGET_GEN=Visual Studio 18 2026"

if not "%~2"=="" (
    echo Usage: %SCRIPT% [-2022^|-2026]
    exit /b 1
)

if /i "%~1"=="-2026" (
    set "PRESET=vs2026"
    set "TARGET_GEN=Visual Studio 18 2026"
) else if /i "%~1"=="-2022" (
    set "PRESET=vs2022"
    set "TARGET_GEN=Visual Studio 17 2022"
) else if not "%~1"=="" (
    echo Usage: %SCRIPT% [-2022^|-2026]
    exit /b 1
)

rem The "Visual Studio 18 2026" generator was added in CMake 4.2; fail early
rem with a clear message instead of a cryptic CMake error on older versions.
if "%PRESET%"=="vs2026" (
    call :require_cmake_42
    if errorlevel 1 exit /b 1
)

rem CMake cannot reconfigure an existing cache with a different generator, so
rem wipe the disposable build/ tree when the cached generator does not match.
if exist "%~dp0build\CMakeCache.txt" (
    set "CACHED_GEN="
    for /f "tokens=2 delims==" %%a in ('findstr /b /c:"CMAKE_GENERATOR:INTERNAL=" "%~dp0build\CMakeCache.txt"') do set "CACHED_GEN=%%a"
    if not "!CACHED_GEN!"=="%TARGET_GEN%" (
        echo Switching generator ^(!CACHED_GEN! -^> %TARGET_GEN%^), cleaning build/
        rmdir /s /q "%~dp0build"
    )
)

pushd "%~dp0" >nul

cmake --preset %PRESET%
set "EXIT_CODE=%ERRORLEVEL%"

popd >nul
exit /b %EXIT_CODE%

:require_cmake_42
setlocal
set "CMAKE_VER="
for /f "tokens=3" %%v in ('cmake --version ^| findstr /b /c:"cmake version"') do set "CMAKE_VER=%%v"
if not defined CMAKE_VER (
    echo Could not determine CMake version; is CMake installed and on PATH?
    endlocal & exit /b 1
)
for /f "tokens=1,2 delims=." %%a in ("%CMAKE_VER%") do (
    set "CMAKE_MAJOR=%%a"
    set "CMAKE_MINOR=%%b"
)
set "CMAKE_OK="
if %CMAKE_MAJOR% GTR 4 set "CMAKE_OK=1"
if %CMAKE_MAJOR% EQU 4 if %CMAKE_MINOR% GEQ 2 set "CMAKE_OK=1"
if not defined CMAKE_OK (
    echo Visual Studio 2026 generation requires CMake 4.2 or newer ^(found %CMAKE_VER%^).
    echo Upgrade CMake, or run: %SCRIPT% -2022
    endlocal & exit /b 1
)
endlocal & exit /b 0
