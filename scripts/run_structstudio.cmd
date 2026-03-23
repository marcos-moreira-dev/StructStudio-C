@echo off
rem StructStudio C
rem --------------
rem Wrapper simple para ejecutar el runner PowerShell con doble clic.

set SCRIPT_DIR=%~dp0
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%run_structstudio.ps1" %*
