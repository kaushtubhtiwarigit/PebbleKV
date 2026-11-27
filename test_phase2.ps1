# Test script for PebbleKV Phase 2 - TCP Server
Write-Host "`n=== PebbleKV Phase 2: TCP Server Test ===`n" -ForegroundColor Cyan

# Check if executables exist
if (-not (Test-Path "pebblekv_server.exe")) {
    Write-Host "❌ pebblekv_server.exe not found. Run build_server.bat first." -ForegroundColor Red
    exit 1
}

if (-not (Test-Path "pebblekv_client.exe")) {
    Write-Host "❌ pebblekv_client.exe not found. Run build_server.bat first." -ForegroundColor Red
    exit 1
}

Write-Host "✅ Found server and client executables`n" -ForegroundColor Green

# Kill any existing server process
Get-Process | Where-Object {$_.ProcessName -like "pebblekv_server"} | Stop-Process -Force -ErrorAction SilentlyContinue

# Start server in background
Write-Host "Starting PebbleKV server on port 6379..." -ForegroundColor Cyan
$serverProcess = Start-Process -FilePath ".\pebblekv_server.exe" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 2

if ($serverProcess.HasExited) {
    Write-Host "❌ Server failed to start!" -ForegroundColor Red
    exit 1
}

Write-Host "✅ Server started (PID: $($serverProcess.Id))`n" -ForegroundColor Green

try {
    # Test commands
    $testCommands = @"
SET user Alice
GET user
SET score 100
GET score
SET message Hello from PebbleKV
GET message
DELETE score
GET score
DELETE nonexistent
SET user Bob
GET user
quit
"@

    Write-Host "Running test commands through TCP client..." -ForegroundColor Cyan
    Write-Host "Commands:`n$testCommands`n" -ForegroundColor Gray

    $output = $testCommands | .\pebblekv_client.exe 2>&1 | Out-String

    Write-Host "--- Output ---" -ForegroundColor Cyan
    Write-Host $output -ForegroundColor White

    # Validation
    $tests = @(
        @{Name="Client connected to server"; Pattern="Connected to PebbleKV"; Pass=$output -match "Connected to PebbleKV"},
        @{Name="SET command works"; Pattern="OK"; Pass=$output -match "OK"},
        @{Name="GET existing key"; Pattern="Alice"; Pass=$output -match "Alice"},
        @{Name="GET number value"; Pattern="100"; Pass=$output -match "100"},
        @{Name="GET multi-word value"; Pattern="Hello from PebbleKV"; Pass=$output -match "Hello from PebbleKV"},
        @{Name="DELETE returns OK"; Pattern="OK"; Pass=$output -match "OK"},
        @{Name="GET deleted key returns nil"; Pattern="\(nil\)"; Pass=$output -match "\(nil\)"},
        @{Name="Update existing key"; Pattern="Bob"; Pass=$output -match "Bob"},
        @{Name="Graceful disconnect"; Pattern="Goodbye"; Pass=$output -match "Goodbye"}
    )

    Write-Host "`n--- Test Results ---" -ForegroundColor Cyan
    $passed = 0
    $failed = 0

    foreach ($test in $tests) {
        if ($test.Pass) {
            Write-Host "✅ $($test.Name)" -ForegroundColor Green
            $passed++
        } else {
            Write-Host "❌ $($test.Name)" -ForegroundColor Red
            $failed++
        }
    }

    Write-Host "`n--- Summary ---" -ForegroundColor Cyan
    Write-Host "Passed: $passed / $($tests.Count)" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Yellow" })
    Write-Host "Failed: $failed / $($tests.Count)" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Red" })

    if ($failed -eq 0) {
        Write-Host "`n🎉 All tests passed! Phase 2 is complete.`n" -ForegroundColor Green
        Write-Host "The TCP server is working correctly!" -ForegroundColor Green
    } else {
        Write-Host "`n⚠️  Some tests failed. Please review the output.`n" -ForegroundColor Yellow
    }
}
finally {
    # Clean up: stop server
    Write-Host "`nStopping server..." -ForegroundColor Cyan
    Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    Write-Host "✅ Server stopped`n" -ForegroundColor Green
}
