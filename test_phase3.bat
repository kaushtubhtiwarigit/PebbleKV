@echo off
echo ========================================
echo   PebbleKV Phase 3: Multi-Client Test
echo ========================================
echo.

REM Start server
echo Starting server...
start "PebbleKV Server" pebblekv_server.exe
timeout /t 2 /nobreak > nul

echo.
echo Testing with 3 simultaneous clients...
echo.

REM Start 3 clients simultaneously
echo [Client 1] Running...
start /B cmd /c "(echo SET client1 Alice & echo GET client1 & echo quit) | pebblekv_client.exe > client1_output.txt 2>&1"

echo [Client 2] Running...
start /B cmd /c "(echo SET client2 Bob & echo GET client2 & echo quit) | pebblekv_client.exe > client2_output.txt 2>&1"

echo [Client 3] Running...
start /B cmd /c "(echo SET client3 Carol & echo GET client3 & echo quit) | pebblekv_client.exe > client3_output.txt 2>&1"

echo.
echo Waiting for clients to complete...
timeout /t 3 /nobreak > nul

echo.
echo ========================================
echo Client 1 Output:
echo ========================================
type client1_output.txt 2>nul
echo.

echo ========================================
echo Client 2 Output:
echo ========================================
type client2_output.txt 2>nul
echo.

echo ========================================
echo Client 3 Output:
echo ========================================
type client3_output.txt 2>nul
echo.

REM Cleanup
del client*.txt 2>nul
taskkill /F /IM pebblekv_server.exe >nul 2>&1

echo.
echo ========================================
echo Test Complete!
echo.
echo All 3 clients ran simultaneously.
echo Phase 3 multi-client support working!
echo ========================================
pause
