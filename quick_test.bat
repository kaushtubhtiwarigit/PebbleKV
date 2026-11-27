@echo off
echo Starting PebbleKV server...
start /B pebblekv_server.exe

echo Waiting for server to start...
timeout /t 3 /nobreak > nul

echo.
echo Testing with client...
echo.

(
echo SET name Kaushtubh
echo GET name
echo SET age 25
echo GET age
echo DELETE age
echo GET age
echo quit
) | pebblekv_client.exe

echo.
echo Stopping server...
taskkill /F /IM pebblekv_server.exe > nul 2>&1

echo.
echo Test complete!
pause
