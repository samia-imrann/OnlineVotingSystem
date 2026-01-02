@echo off
echo ========================================
echo   Online Voting System
echo ========================================

REM Check if server is already running
netstat -an | findstr ":8080" | findstr "LISTENING" >nul
if %errorlevel% equ 0 (
    echo Server is already running on port 8080
) else (
    echo Starting server...
    start /B server.exe
    timeout /t 2 /nobreak >nul
)

echo Opening web interface...
start http://localhost:8080

echo.
echo System is ready!
echo Press any key to stop the server...
echo ========================================
pause >nul
taskkill /f /im server.exe >nul 2>&1