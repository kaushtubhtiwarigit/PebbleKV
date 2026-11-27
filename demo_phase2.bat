@echo off
cls
echo ========================================
echo     PebbleKV Phase 2 Demo
echo     TCP Server with Network Access
echo ========================================
echo.
echo This demo will:
echo 1. Start the PebbleKV TCP server
echo 2. Connect a client and run commands
echo 3. Show the results
echo.
pause

echo.
echo [1/3] Starting PebbleKV server on port 6379...
echo.
start "PebbleKV Server" pebblekv_server.exe

echo Waiting for server to initialize...
timeout /t 2 /nobreak > nul

echo.
echo [2/3] Server is running! Connecting client...
echo.
timeout /t 1 /nobreak > nul

echo [3/3] Running demo commands:
echo.
(
echo SET username Alice
echo GET username
echo SET score 9999
echo GET score
echo SET message Hello from PebbleKV over TCP!
echo GET message
echo DELETE score
echo GET score
echo quit
) | pebblekv_client.exe

echo.
echo ========================================
echo Demo complete!
echo.
echo The client connected to the server over TCP,
echo sent commands, and received responses.
echo.
echo In a real scenario, clients could be on
echo different machines across a network.
echo ========================================
echo.
echo Stopping server...
taskkill /F /IM pebblekv_server.exe > nul 2>&1

echo.
echo To start the server manually: run_server.bat
echo To connect a client: run_client.bat
echo.
pause
