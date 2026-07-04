@echo off
setlocal

set "INDEX=%~dp0docs\html\index.html"

if not exist "%INDEX%" (
    echo Documentation not found at "%INDEX%".
    echo Run docs\generate_docs.bat first.
    exit /b 1
)

start "" "%INDEX%"
exit /b 0
