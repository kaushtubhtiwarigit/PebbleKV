# Test script for PebbleKV Phase 1
Write-Host "`n=== PebbleKV Phase 1 Functional Test ===`n" -ForegroundColor Cyan

# Check if executable exists
if (-not (Test-Path "pebblekv.exe")) {
    Write-Host "❌ pebblekv.exe not found. Please compile first:" -ForegroundColor Red
    Write-Host "   g++ -std=c++14 -Wall -Wextra -I.\include -o pebblekv.exe .\src\main.cpp .\src\database.cpp .\src\command_parser.cpp" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ Found pebblekv.exe`n" -ForegroundColor Green

# Test commands
$testCommands = @"
SET name Kaushtubh
GET name
SET age 25
GET age
SET city San Francisco with spaces
GET city
DELETE age
GET age
DELETE nonexistent
SET name John
GET name
quit
"@

Write-Host "Running test commands..." -ForegroundColor Cyan
Write-Host "Commands:`n$testCommands`n" -ForegroundColor Gray

$output = $testCommands | .\pebblekv.exe

Write-Host "`n--- Output ---" -ForegroundColor Cyan
Write-Host $output -ForegroundColor White

# Simple validation
$tests = @(
    @{Name="SET command"; Pattern="OK"; Pass=$output -match "OK"},
    @{Name="GET existing key"; Pattern="Kaushtubh"; Pass=$output -match "Kaushtubh"},
    @{Name="GET with number"; Pattern="25"; Pass=$output -match "25"},
    @{Name="GET multi-word value"; Pattern="San Francisco with spaces"; Pass=$output -match "San Francisco with spaces"},
    @{Name="DELETE returns OK"; Pattern="OK"; Pass=$output -match "OK"},
    @{Name="GET deleted key"; Pattern="\(nil\)"; Pass=$output -match "\(nil\)"},
    @{Name="Update existing key"; Pattern="John"; Pass=$output -match "John"}
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
    Write-Host "`n🎉 All tests passed! Phase 1 is complete.`n" -ForegroundColor Green
} else {
    Write-Host "`n⚠️  Some tests failed. Please review the output.`n" -ForegroundColor Yellow
}
