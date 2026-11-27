# Manual build script for PebbleKV (no CMake required)
# Requires C++20 compiler (GCC 10+, Clang 11+, or MSVC 2019+)

Write-Host "PebbleKV Manual Build Script" -ForegroundColor Cyan
Write-Host "============================`n"

# Check for compilers
$compiler = $null
$compilerName = ""

# Try g++
if (Get-Command g++ -ErrorAction SilentlyContinue) {
    $version = & g++ --version 2>&1 | Select-Object -First 1
    Write-Host "Found: $version" -ForegroundColor Green
    
    # Check if GCC version is sufficient (need 10+)
    if ($version -match "(\d+)\.(\d+)\.(\d+)") {
        $major = [int]$matches[1]
        if ($major -ge 10) {
            $compiler = "g++"
            $compilerName = "GCC"
            Write-Host "✅ GCC version $major is compatible with C++20`n" -ForegroundColor Green
        } else {
            Write-Host "⚠️  GCC version $major is too old. Need version 10+ for C++20`n" -ForegroundColor Yellow
        }
    }
}

# Try clang++ if g++ not suitable
if (-not $compiler -and (Get-Command clang++ -ErrorAction SilentlyContinue)) {
    $version = & clang++ --version 2>&1 | Select-Object -First 1
    Write-Host "Found: $version" -ForegroundColor Green
    $compiler = "clang++"
    $compilerName = "Clang"
}

# Try MSVC (cl.exe) if neither g++ nor clang++ found
if (-not $compiler -and (Get-Command cl -ErrorAction SilentlyContinue)) {
    $version = & cl 2>&1 | Select-Object -First 1
    Write-Host "Found: $version" -ForegroundColor Green
    $compiler = "cl"
    $compilerName = "MSVC"
}

if (-not $compiler) {
    Write-Host "❌ No C++ compiler found!" -ForegroundColor Red
    Write-Host "`nPlease install one of:" -ForegroundColor Yellow
    Write-Host "  - Visual Studio 2019+ (https://visualstudio.microsoft.com/)"
    Write-Host "  - MinGW-w64 GCC 10+ (https://www.msys2.org/)"
    Write-Host "  - Clang 11+ (https://releases.llvm.org/)"
    Write-Host "`nSee INSTALL.md for detailed instructions."
    exit 1
}

Write-Host "Using $compilerName compiler`n" -ForegroundColor Cyan

# Build parameters
$outputDir = "build_manual"
$outputExe = "$outputDir\pebblekv.exe"
$sources = @(
    "src\main.cpp",
    "src\database.cpp",
    "src\command_parser.cpp"
)

# Create output directory
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

Write-Host "Compiling PebbleKV..." -ForegroundColor Cyan

# Compile based on compiler type
$success = $false

if ($compiler -eq "cl") {
    # MSVC
    $flags = "/std:c++20 /EHsc /W4 /I.\include /Fe:$outputExe"
    $cmd = "$compiler $flags $($sources -join ' ')"
    Write-Host $cmd
    Invoke-Expression $cmd
    $success = $LASTEXITCODE -eq 0
} else {
    # GCC or Clang
    $flags = "-std=c++20 -Wall -Wextra -Wpedantic -I./include -o $outputExe"
    $cmd = "$compiler $flags $($sources -join ' ')"
    Write-Host $cmd
    Invoke-Expression $cmd
    $success = $LASTEXITCODE -eq 0
}

Write-Host ""

if ($success) {
    Write-Host "✅ Build successful!" -ForegroundColor Green
    Write-Host "`nRun PebbleKV with:" -ForegroundColor Cyan
    Write-Host "  .\$outputExe`n" -ForegroundColor White
    Write-Host "Or simply:" -ForegroundColor Cyan
    Write-Host "  .\$outputDir\pebblekv.exe`n" -ForegroundColor White
} else {
    Write-Host "❌ Build failed!" -ForegroundColor Red
    Write-Host "`nCommon issues:" -ForegroundColor Yellow
    Write-Host "  - Compiler too old (need C++20 support)"
    Write-Host "  - Missing source files"
    Write-Host "  - Syntax errors in code"
    Write-Host "`nCheck the error messages above for details."
    exit 1
}
