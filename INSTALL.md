# Installation Guide for PebbleKV on Windows

## Prerequisites

PebbleKV requires:
1. **CMake 3.14+** - Build system
2. **C++20 Compiler** - One of:
   - GCC 10+ (MinGW-w64)
   - Clang 11+
   - MSVC 2019+ (Visual Studio 2019 or newer)

## Current System Status

You have:
- ✅ GCC (but version 6.3.0 is too old - need 10+)
- ❌ CMake (not installed)

## Recommended: Install Visual Studio Build Tools

The easiest path on Windows:

### Option 1: Visual Studio 2022 Community (Free)

1. Download from: https://visualstudio.microsoft.com/downloads/
2. Run installer
3. Select "Desktop development with C++"
4. Install
5. CMake is included automatically

### Option 2: Build Tools for Visual Studio (Lightweight)

1. Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
2. Install "C++ build tools" workload
3. Install CMake separately: https://cmake.org/download/

### Option 3: MinGW-w64 with MSYS2 (Linux-like)

1. Install MSYS2: https://www.msys2.org/
2. Open MSYS2 terminal:
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-cmake
   pacman -S make
   ```
3. Add to PATH: `C:\msys64\mingw64\bin`

### Option 4: Chocolatey (Package Manager)

If you have Chocolatey:
```powershell
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
choco install mingw
```

## Building PebbleKV

### With Visual Studio / MSVC

```powershell
# From PebbleKV directory
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### With MinGW / GCC

```powershell
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

## Running Tests

```powershell
cd build
ctest --output-on-failure
```

## Running PebbleKV

```powershell
# Windows (MSVC)
.\build\Release\pebblekv.exe

# Windows (MinGW)
.\build\pebblekv.exe
```

## Troubleshooting

### "cmake is not recognized"
- CMake not in PATH. Reinstall CMake with "Add to PATH" option.
- Or add manually: System Properties → Environment Variables → Path

### "C++ compiler not found"
- Install Visual Studio Build Tools or MinGW-w64
- Ensure compiler is in PATH

### "C++20 features not supported"
- Your compiler is too old
- GCC: Need version 10+
- MSVC: Need Visual Studio 2019+
- Clang: Need version 11+

### Cannot download GoogleTest
- Check internet connection
- May need to configure proxy in CMake
- Or disable tests: `cmake -DBUILD_TESTING=OFF ..`

## Alternative: Manual Compilation (No CMake)

If you can't install CMake, you can compile manually with any C++20 compiler:

### Manual Build Script (PowerShell)

Save as `build.ps1`:
```powershell
# Compile without CMake
$CXX = "g++"  # or "cl.exe" for MSVC
$CXXFLAGS = "-std=c++20 -Wall -Wextra -I./include"
$SOURCES = "src/main.cpp src/database.cpp src/command_parser.cpp"
$OUTPUT = "pebblekv.exe"

& $CXX $CXXFLAGS.Split() $SOURCES.Split() -o $OUTPUT

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Build successful! Run with: .\$OUTPUT"
} else {
    Write-Host "❌ Build failed"
}
```

Run: `powershell .\build.ps1`

## Next Steps

Once built successfully:
1. Run tests: `ctest --output-on-failure`
2. Try the CLI: `.\pebblekv.exe`
3. Experiment with SET, GET, DELETE commands
4. Ready for Phase 2!

## Need Help?

Common issues and solutions in README.md
