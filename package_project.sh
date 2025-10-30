#!/usr/bin/env bash

set -e

RELEASE_DIR="export_release"
FULL_PKG="statusbarlog-full"
MINIMAL_PKG="statusbarlog-minimal"
VERSION="1.1.0"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

detect_platform() {
    local os_name
    local arch
    # Get OS
    case "$(uname -s)" in
        *Linux*)     os_name="linux" ;;
        *Darwin*)    os_name="macos" ;;
        *CYGWIN*)    os_name="windows" ;;
        *MINGW*)     os_name="windows" ;;
        *)          os_name="unknown" ;;
    esac
    # Get architecture
    case "$(uname -m)" in
        x86_64)     arch="x86_64" ;;
        amd64)      arch="x86_64" ;;
        i386)       arch="i386" ;;
        i686)       arch="i686" ;;
        aarch64)    arch="aarch64" ;;
        arm64)      arch="arm64" ;;
        armv7l)     arch="armv7" ;;
        armv6l)     arch="armv6" ;;
        *)          arch="unknown" ;;
    esac
    echo "${os_name}-${arch}"
}
echo -e "${GREEN}=== StatusbarLog Release Packaging ===${NC}"

PLATFORM_ARCH=$(detect_platform)
LIBRARY_NAME="statusbarlog-v${VERSION}-${PLATFORM_ARCH}"

echo -e "${YELLOW}Detected platform: ${PLATFORM_ARCH}${NC}"

echo -e "${YELLOW}Rebuilding project with Release configuration...${NC}"
echo "Cleaning and reconfiguring build..."
mkdir -p build
cd build
rm -rf *

# Configure with specified options
echo "Configuring CMake with:"
echo "  - CMAKE_BUILD_TYPE=Release"
echo "  - STATUSBARLOG_BUILD_TESTS=OFF" 
echo "  - STATUSBARLOG_LOG_LEVEL=kLogLevelInf"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DSTATUSBARLOG_BUILD_TESTS=OFF \
    -DSTATUSBARLOG_LOG_LEVEL=kLogLevelInf

# Build the project
echo "Building project..."
cmake --build . -j$(nproc) --config Release

# Return to root directory
cd ..

echo -e "${GREEN}Build completed successfully${NC}"

echo "Cleaning previous release..."
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR/$FULL_PKG"
mkdir -p "$RELEASE_DIR/$MINIMAL_PKG"

check_file() {
    if [ ! -f "$1" ] && [ ! -d "$1" ]; then
        echo -e "${RED}Error: Required file/directory '$1' not found${NC}"
        return 1
    fi
    return 0
}

echo "Verifying essential files..."
essential_files=(
    "CMakeLists.txt"
    "include/statusbarlog/statusbarlog.h.in"
    "build/include/statusbarlog/statusbarlog.h"
    "build/libstatusbarlog.a"
    "LICENSE"
    "README.md"
    ".clang-format"
    ".gitignore"
    "compile_commands.json.linux"
    "compile_commands.json.windows"
    "Doxyfile"
)

for file in "${essential_files[@]}"; do
    check_file "$file" || exit 1
done

# ========== Create full package (includes tests and documentation source) ==========
echo -e "${YELLOW}Creating full package...${NC}"

# Copy source files
cp -r "include" "$RELEASE_DIR/$FULL_PKG/"
cp -r "src" "$RELEASE_DIR/$FULL_PKG/" 2>/dev/null || echo "src directory not found, continuing..."
cp -r "tests" "$RELEASE_DIR/$FULL_PKG/"
rsync -av --exclude='docs/latex/' --exclude='docs/html/' "docs" "$RELEASE_DIR/$FULL_PKG/" 2>/dev/null || echo "docs directory not found, continuing..."

# Copy configuration and documentation files
cp "CMakeLists.txt" "$RELEASE_DIR/$FULL_PKG/"
cp "LICENSE" "$RELEASE_DIR/$FULL_PKG/"
cp "README.md" "$RELEASE_DIR/$FULL_PKG/"
cp ".clang-format" "$RELEASE_DIR/$FULL_PKG/"
cp ".gitignore" "$RELEASE_DIR/$FULL_PKG/"
cp "Doxyfile" "$RELEASE_DIR/$FULL_PKG/"
cp "compile_commands.json.linux" "$RELEASE_DIR/$FULL_PKG/" 2>/dev/null || echo "compile_commands.json.in not found, continuing..."
cp "compile_commands.json.windows" "$RELEASE_DIR/$FULL_PKG/" 2>/dev/null || echo "compile_commands.json.in not found, continuing..."

# Remove any build artifacts that might have been copied
find "$RELEASE_DIR/$FULL_PKG" -name "*.o" -delete 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "*.a" -delete 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "CMakeCache.txt" -delete 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "_deps" -type d -exec rm -rf {} + 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "*.so" -delete 2>/dev/null || true
find "$RELEASE_DIR/$FULL_PKG" -name "*.dylib" -delete 2>/dev/null || true

# ========== Create minimal package ==========
echo -e "${YELLOW}Creating minimal package...${NC}"

# Create directory structure
cp -r "include" "$RELEASE_DIR/$MINIMAL_PKG/"
cp -r "src" "$RELEASE_DIR/$MINIMAL_PKG/" 2>/dev/null || echo "src directory not found, continuing..."

# Copy essential files for CMake module
cp "CMakeLists.txt" "$RELEASE_DIR/$MINIMAL_PKG/"
cp "LICENSE" "$RELEASE_DIR/$MINIMAL_PKG/"
cp "README.md" "$RELEASE_DIR/$MINIMAL_PKG/"
cp ".gitignore" "$RELEASE_DIR/$MINIMAL_PKG/"

# ========== Copy header files and precompiled library with platform-specific names ==========
echo -e "${YELLOW}Creating platform-specific binaries...${NC}"

# Copy header files
cp "include/statusbarlog/statusbarlog.h.in" "$RELEASE_DIR/"
cp "build/include/statusbarlog/statusbarlog.h" "$RELEASE_DIR/"

# Copy and rename precompiled library based on platform
if [ -f "build/libstatusbarlog.a" ]; then
    cp "build/libstatusbarlog.a" "$RELEASE_DIR/lib${LIBRARY_NAME}.a"
    echo -e "${GREEN}Created: lib${LIBRARY_NAME}.a${NC}"
elif [ -f "build/statusbarlog.lib" ]; then
    cp "build/statusbarlog.lib" "$RELEASE_DIR/${LIBRARY_NAME}.lib"
    echo -e "${GREEN}Created: ${LIBRARY_NAME}.lib${NC}"
else
    echo -e "${YELLOW}Warning: No precompiled library found in build directory${NC}"
fi

# ========== Create compressed packages ==========
echo -e "${YELLOW}Creating compressed archives...${NC}"

cd "$RELEASE_DIR"

# Create .tar.gz packages
tar -czf "${FULL_PKG}-v${VERSION}.tar.gz" "$FULL_PKG"
tar -czf "${MINIMAL_PKG}-v${VERSION}.tar.gz" "$MINIMAL_PKG"

# Create .zip packages
zip -rq "${FULL_PKG}-v${VERSION}.zip" "$FULL_PKG"
zip -rq "${MINIMAL_PKG}-v${VERSION}.zip" "$MINIMAL_PKG"

# Create checksums
echo -e "${YELLOW}Creating checksums...${NC}"
sha256sum *.tar.gz *.zip *.a *.lib 2>/dev/null | grep -v ' No such file' > "checksums.sha256"

# Display results
echo -e "${GREEN}=== Release Packages Created ===${NC}"
echo "Platform: ${PLATFORM_ARCH}"
echo ""
echo "Full package (with tests/docs):"
echo "  - ${FULL_PKG}-v${VERSION}.tar.gz"
echo "  - ${FULL_PKG}-v${VERSION}.zip"
echo ""
echo "Minimal package (source only, no tests/docs):"
echo "  - ${MINIMAL_PKG}-v${VERSION}.tar.gz" 
echo "  - ${MINIMAL_PKG}-v${VERSION}.zip"
echo ""
echo "Individual files:"
if [ -f "${LIBRARY_NAME}.a" ]; then
    echo "  - ${LIBRARY_NAME}.a (precompiled library)"
elif [ -f "${LIBRARY_NAME}.lib" ]; then
    echo "  - ${LIBRARY_NAME}.lib (precompiled library)"
fi
echo "  - statusbarlog.h.in (header template)"
echo "  - statusbarlog.h (configured header)"
echo ""
echo "Directory structure:"
echo "Full package:"
find "$FULL_PKG" -type f | head -20
echo ""
echo "Minimal package:"
find "$MINIMAL_PKG" -type f

echo -e "${GREEN}=== Packaging Complete ===${NC}"
echo "Files are in: $RELEASE_DIR/"
echo "Checksums: $RELEASE_DIR/checksums.sha256"
