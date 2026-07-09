@echo off
setlocal

set "INDEX=%~dp0docs\html\index.html"

if not exist "%INDEX%" (
    echo Documentation not found at "%INDEX%".
    echo Read it online at https://dyronix.github.io/the-dyro-engine/
    echo or run docs\generate_docs.bat to build it locally first.
    exit /b 1
)

start "" "%INDEX%"
exit /b 0
