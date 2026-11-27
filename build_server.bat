@echo off
echo Building PebbleKV Server...
g++ -std=c++14 -Wall -Wextra -I.\include -o pebblekv_server.exe .\src\main.cpp .\src\server.cpp .\src\database.cpp .\src\command_parser.cpp .\src\thread_compat.cpp .\src\wal.cpp .\src\lru_cache.cpp -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo Server build successful!
    echo.
    echo Building PebbleKV Client...
    g++ -std=c++14 -Wall -Wextra -o pebblekv_client.exe .\src\client.cpp -lws2_32
    if %ERRORLEVEL% EQU 0 (
        echo Client build successful!
        echo.
        echo Run server: pebblekv_server.exe
        echo Run client: pebblekv_client.exe
    ) else (
        echo Client build failed!
    )
) else (
    echo Server build failed!
)
pause
