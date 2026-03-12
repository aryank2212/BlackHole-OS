@echo off
echo ============================================
echo  BlackHole OS - Tool Installer (Admin)
echo ============================================
echo.

:: Self-elevate if not admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Requesting admin privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo Cleaning stale Chocolatey lock files...
del /f /q "C:\ProgramData\chocolatey\lib\000e9d50408ecdb0a6ea4a9e5f60951d1db925e0" 2>nul
del /f /q "C:\ProgramData\chocolatey\lib\fbdc0560aaa937379f34e7f76506ff3536773a52" 2>nul
del /f /q "C:\ProgramData\chocolatey\lib\59745d231a33fe9f41972f379684fce8f36b2392" 2>nul

echo.
echo Installing NASM...
choco install nasm -y
echo.
echo Installing MinGW (GCC)...
choco install mingw -y
echo.
echo Installing QEMU...
choco install qemu -y

echo.
echo ============================================
echo  ALL DONE! Close this window, restart your
echo  terminal, then run:
echo    cd d:\Github\Private\BlackHole_OS\build
echo    .\build.ps1 run
echo ============================================
pause
