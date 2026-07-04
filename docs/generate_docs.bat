@echo off
rem Regenerates the html documentation in docs/html from the engine headers
rem and the guide pages in docs/pages. Requires doxygen on the PATH:
rem https://www.doxygen.nl/download.html

pushd "%~dp0"

where doxygen >nul 2>nul
if errorlevel 1 (
    echo doxygen not found - install it from https://www.doxygen.nl/download.html
    popd
    exit /b 1
)

doxygen Doxyfile

popd
