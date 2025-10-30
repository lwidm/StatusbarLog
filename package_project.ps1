#!/usr/bin/env pwsh

param()

$ErrorActionPreference = "Stop"

# Configuration
$RELEASE_DIR = "export_release"
$FULL_PKG = "statusbarlog-full"
$MINIMAL_PKG = "statusbarlog-minimal"
$VERSION = "1.1.0"

# Colors for output
$RED = "`e[31m"
$GREEN = "`e[32m"
$YELLOW = "`e[33m"
$NC = "`e[0m"

function Write-ColorOutput {
    param($Color, $Message)
    Write-Host $Color $Message $NC
}

function Detect-Platform {
    $os_name = "unknown"
    $arch = "unknown"
    
    # Get OS
    if ($IsLinux) {
        $os_name = "linux"
    } elseif ($IsMacOS) {
        $os_name = "macos"
    } elseif ($IsWindows) {
        $os_name = "windows"
    } else {
        # Fallback for older PowerShell versions
        $os_name = $PSVersionTable.Platform.ToLower()
        if (-not $os_name -or $os_name -eq "win32nt") {
            $os_name = "windows"
        }
    }
    
    # Get architecture
    if ([Environment]::Is64BitOperatingSystem) {
        $arch = "x86_64"
    } else {
        $arch = "x86"
    }
    
    # More specific architecture detection
    if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") {
        $arch = "arm64"
    }
    
    return "${os_name}-${arch}"
}

Write-ColorOutput -Color $GREEN -Message "=== StatusbarLog Release Packaging ==="

$PLATFORM_ARCH = Detect-Platform
$LIBRARY_NAME = "statusbarlog-v${VERSION}-${PLATFORM_ARCH}"

Write-ColorOutput -Color $YELLOW -Message "Detected platform: ${PLATFORM_ARCH}"

# Step 1: Rebuild project
Write-ColorOutput -Color $YELLOW -Message "Rebuilding project with Release configuration..."
Write-Host "Cleaning and reconfiguring build..."

# Clean and create build directory
Remove-Item -Path "build" -Recurse -ErrorAction SilentlyContinue
New-Item -Path "build" -ItemType Directory -Force | Out-Null
Set-Location "build"

# Configure with specified options
Write-Host "Configuring CMake with:"
Write-Host "  - CMAKE_BUILD_TYPE=Release"
Write-Host "  - STATUSBARLOG_BUILD_TESTS=OFF" 
Write-Host "  - STATUSBARLOG_LOG_LEVEL=kLogLevelInf"
& cmake -S .. -B . `
    -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Release `
    -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf `
    -DSTATUSBARLOG_BUILD_TESTS=ON `
    -DSTATUSBARLOG_BUILD_TEST_MAIN=ON

if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput -Color $RED -Message "CMake configuration failed!"
    exit 1
}

# Build the project
Write-Host "Building project..."
& cmake --build . --config Release --parallel

if ($LASTEXITCODE -ne 0) {
    Write-ColorOutput -Color $RED -Message "Build failed!"
    exit 1
}

# Return to root directory
Set-Location ".."

Write-ColorOutput -Color $GREEN -Message "Build completed successfully"

# Step 2: Clean and create release directories
Write-Host "Cleaning previous release..."
Remove-Item -Path $RELEASE_DIR -Recurse -ErrorAction SilentlyContinue
New-Item -Path "$RELEASE_DIR/$FULL_PKG" -ItemType Directory -Force | Out-Null
New-Item -Path "$RELEASE_DIR/$MINIMAL_PKG" -ItemType Directory -Force | Out-Null

function Test-FileExists {
    param($FilePath)
    
    if (-not (Test-Path $FilePath)) {
        Write-ColorOutput -Color $RED -Message "Error: Required file/directory '$FilePath' not found"
        return $false
    }
    return $true
}

Write-Host "Verifying essential files..."
$essential_files = @(
    "CMakeLists.txt"
    "include/statusbarlog/statusbarlog.h.in"
    "build/include/statusbarlog/statusbarlog.h"
    "build/statusbarlog.lib"
    "LICENSE"
    "README.md"
    ".clang-format"
    ".gitignore"
    "compile_commands.json.linux"
    "compile_commands.json.windows"
    "Doxyfile"
)

foreach ($file in $essential_files) {
    if (-not (Test-FileExists -FilePath $file)) {
        exit 1
    }
}

# ========== Create full package (includes tests and documentation source) ==========
Write-ColorOutput -Color $YELLOW -Message "Creating full package..."

# Copy source files
Copy-Item -Path "include" -Destination "$RELEASE_DIR/$FULL_PKG" -Recurse -Force
if (Test-Path "src") {
    Copy-Item -Path "src" -Destination "$RELEASE_DIR/$FULL_PKG" -Recurse -Force
} else {
    Write-Host "src directory not found, continuing..."
}
Copy-Item -Path "tests" -Destination "$RELEASE_DIR/$FULL_PKG" -Recurse -Force
if (Test-Path "docs") {
    Get-ChildItem -Path "docs" -Exclude @("latex", "html") | Copy-Item -Destination "$RELEASE_DIR/$FULL_PKG/docs" -Recurse -Force
} else {
    Write-Host "docs directory not found, continuing..."
}

# Copy configuration and documentation files
$config_files = @(
    "CMakeLists.txt",
    "LICENSE", 
    "README.md",
    ".clang-format",
    ".gitignore", 
    "Doxyfile",
    "compile_commands.json.linux",
    "compile_commands.json.windows"
)

foreach ($file in $config_files) {
    if (Test-Path $file) {
        Copy-Item -Path $file -Destination "$RELEASE_DIR/$FULL_PKG" -Force
    } else {
        Write-Host "$file not found, continuing..."
    }
}

# Remove any build artifacts that might have been copied
Get-ChildItem -Path "$RELEASE_DIR/$FULL_PKG" -Recurse -File | Where-Object {
    $_.Extension -in @('.o', '.a', '.so', '.dylib', '.lib') -or 
    $_.Name -eq 'CMakeCache.txt'
} | Remove-Item -Force

Get-ChildItem -Path "$RELEASE_DIR/$FULL_PKG" -Recurse -Directory | Where-Object {
    $_.Name -in @('CMakeFiles', '_deps')
} | Remove-Item -Recurse -Force

# ========== Create minimal package ==========
Write-ColorOutput -Color $YELLOW -Message "Creating minimal package..."

# Copy source files
Copy-Item -Path "include" -Destination "$RELEASE_DIR/$MINIMAL_PKG" -Recurse -Force
if (Test-Path "src") {
    Copy-Item -Path "src" -Destination "$RELEASE_DIR/$MINIMAL_PKG" -Recurse -Force
} else {
    Write-Host "src directory not found, continuing..."
}

# Copy essential files for CMake module
$minimal_files = @(
    "CMakeLists.txt",
    "LICENSE",
    "README.md", 
    ".gitignore"
)

foreach ($file in $minimal_files) {
    Copy-Item -Path $file -Destination "$RELEASE_DIR/$MINIMAL_PKG" -Force
}

# ========== Copy header files and precompiled library with platform-specific names ==========
Write-ColorOutput -Color $YELLOW -Message "Creating platform-specific binaries..."

# Copy header files
Copy-Item -Path "include/statusbarlog/statusbarlog.h.in" -Destination $RELEASE_DIR -Force
Copy-Item -Path "build/include/statusbarlog/statusbarlog.h" -Destination $RELEASE_DIR -Force

# Copy and rename precompiled library based on platform
if (Test-Path "build/statusbarlog.lib") {
    Copy-Item -Path "build/statusbarlog.lib" -Destination "$RELEASE_DIR/${LIBRARY_NAME}.lib" -Force
    Write-ColorOutput -Color $GREEN -Message "Created: ${LIBRARY_NAME}.lib"
} elseif (Test-Path "build/libstatusbarlog.a") {
    Copy-Item -Path "build/libstatusbarlog.a" -Destination "$RELEASE_DIR/${LIBRARY_NAME}.a" -Force
    Write-ColorOutput -Color $GREEN -Message "Created: ${LIBRARY_NAME}.a"
} else {
    Write-ColorOutput -Color $YELLOW -Message "Warning: No precompiled library found in build directory"
}

# ========== Create compressed packages ==========
Write-ColorOutput -Color $YELLOW -Message "Creating compressed archives..."

Set-Location $RELEASE_DIR

# Create .zip packages (tar.gz not as common on Windows)
Compress-Archive -Path "$FULL_PKG" -DestinationPath "${FULL_PKG}-v${VERSION}.zip" -Force
Compress-Archive -Path "$MINIMAL_PKG" -DestinationPath "${MINIMAL_PKG}-v${VERSION}.zip" -Force

# Create checksums
Write-ColorOutput -Color $YELLOW -Message "Creating checksums..."
Get-FileHash -Algorithm SHA256 -Path *.zip, *.a, *.lib -ErrorAction SilentlyContinue | 
    Select-Object Algorithm, Hash, Path | 
    Format-Table -AutoSize | 
    Out-String -Width 4096 | 
    Out-File -FilePath "checksums.sha256" -Encoding UTF8

# Display results
Write-ColorOutput -Color $GREEN -Message "=== Release Packages Created ==="
Write-Host "Platform: ${PLATFORM_ARCH}"
Write-Host ""
Write-Host "Full package (with tests/docs):"
Write-Host "  - ${FULL_PKG}-v${VERSION}.zip"
Write-Host ""
Write-Host "Minimal package (source only, no tests/docs):"
Write-Host "  - ${MINIMAL_PKG}-v${VERSION}.zip" 
Write-Host ""
Write-Host "Individual files:"
if (Test-Path "${LIBRARY_NAME}.lib") {
    Write-Host "  - ${LIBRARY_NAME}.lib (precompiled library)"
} elseif (Test-Path "${LIBRARY_NAME}.a") {
    Write-Host "  - ${LIBRARY_NAME}.a (precompiled library)"
}
Write-Host "  - statusbarlog.h.in (header template)"
Write-Host "  - statusbarlog.h (configured header)"
Write-Host ""
Write-Host "Directory structure:"
Write-Host "Full package:"
Get-ChildItem -Path $FULL_PKG -Recurse -File | Select-Object -First 20 Name
Write-Host ""
Write-Host "Minimal package:"
Get-ChildItem -Path $MINIMAL_PKG -Recurse -File | Select-Object -First 20 Name

Write-ColorOutput -Color $GREEN -Message "=== Packaging Complete ==="
Write-Host "Files are in: $RELEASE_DIR/"
Write-Host "Checksums: $RELEASE_DIR/checksums.sha256"

# Return to original directory
Set-Location ".."
