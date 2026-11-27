@echo off
cls
echo ========================================
echo   PebbleKV Phase 4: Persistence Test
echo ========================================
echo.

REM Clean up old data
del /Q data\*.wal 2>nul

echo [Step 1] Starting server and adding data...
start "PebbleKV Server" pebblekv_server.exe
timeout /t 2 /nobreak > nul

echo.
echo Adding test data: Alice, Bob, Carol
(
echo SET user1 Alice
echo SET user2 Bob
echo SET user3 Carol
echo GET user1
echo GET user2
echo GET user3
echo quit
) | pebblekv_client.exe

echo.
echo [Step 2] Stopping server (simulating crash)...
timeout /t 1 /nobreak > nul
taskkill /F /IM pebblekv_server.exe >nul 2>&1
timeout /t 1 /nobreak > nul

echo.
echo [Step 3] Restarting server (should replay WAL)...
start "PebbleKV Server" pebblekv_server.exe
timeout /t 2 /nobreak > nul

echo.
echo [Step 4] Checking if data survived restart...
(
echo GET user1
echo GET user2
echo GET user3
echo quit
) | pebblekv_client.exe

echo.
echo [Step 5] Cleanup
taskkill /F /IM pebblekv_server.exe >nul 2>&1

echo.
echo ========================================
echo Test Complete!
echo.
echo If you saw Alice, Bob, and Carol after
echo the restart, persistence is working!
echo ========================================
pause
