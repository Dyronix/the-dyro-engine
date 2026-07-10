@echo off
rem Regenerates the html documentation in docs/html. Three pieces are
rem assembled into one tree that mirrors what is published to GitHub Pages:
rem
rem   docs/html/index.html      <- landing page   (docs/site, hand-written)
rem   docs/html/about.html      <- about page      (docs/site, hand-written)
rem   docs/html/assets/         <- front-site css and icons (docs/site)
rem   docs/html/guides/         <- guide pages     (docs/pages/*.md via build_guides.py)
rem   docs/html/reference/      <- Doxygen API reference (the engine headers)
rem
rem Requires doxygen (https://www.doxygen.nl/download.html) for the reference
rem and python 3 (https://www.python.org/downloads/) for the guide pages.

pushd "%~dp0"

where doxygen >nul 2>nul
if errorlevel 1 (
    echo doxygen not found - install it from https://www.doxygen.nl/download.html
    popd
    exit /b 1
)

where python >nul 2>nul
if errorlevel 1 (
    echo python not found - install python 3 from https://www.python.org/downloads/
    popd
    exit /b 1
)

rem 1. The code reference, from the engine headers -> docs/html/reference
doxygen Doxyfile
if errorlevel 1 (
    popd
    exit /b 1
)

rem 2. The guide pages, from docs/pages/*.md -> docs/html/guides
python build_guides.py
if errorlevel 1 (
    popd
    exit /b 1
)

rem 3. Overlay the hand-written landing, about page and shared assets.
xcopy "site" "html" /E /I /Y >nul

popd
