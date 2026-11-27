@echo off
cls
echo ========================================
echo   PebbleKV Phase 5: TTL & LRU Test
echo ========================================
echo.

REM Clean up
taskkill /F /IM pebblekv_server.exe >nul 2>&1
del /Q data\*.wal 2>nul

echo [Test 1] Starting server...
start "PebbleKV Server" pebblekv_server.exe
timeout /t 2 /nobreak > nul

echo.
echo [Test 2] Testing TTL (Time-To-Live)...
(
echo SET session abc123
echo EXPIRE session 5
echo TTL session
echo GET session
echo quit
) | pebblekv_client.exe

echo.
echo Waiting 6 seconds for key to expire...
timeout /t 6 /nobreak > nul

echo.
echo [Test 3] Checking expired key...
(
echo GET session
echo TTL session
echo quit
) | pebblekv_client.exe

echo.
echo [Test 4] Testing LRU eviction (limit is 1000 keys)...
echo Adding 1005 keys...
(
for /L %%i in (1,1,1005) do (
    echo SET key%%i value%%i
)
echo GET key1
echo GET key1000
echo GET key1005
echo quit
) | pebblekv_client.exe

echo.
echo [Test 5] Cleanup
taskkill /F /IM pebblekv_server.exe >nul 2>&1

echo.
echo ========================================
echo Test Complete!
echo.
echo Expected behavior:
echo - TTL returns seconds remaining
echo - Expired keys return "(nil)"
echo - Old keys evicted when limit reached
echo ========================================
pause
