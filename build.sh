#!/bin/bash
# ============================================================================
# Build script for Universal DLL - Linux/Unix/WSL
# Supports multiple configurations with/without CRT
# ============================================================================

set -e

# Default values
BUILD_TYPE="Release"
USE_CRT="ON"
BUILD_DIR="build"
GENERATOR="Ninja"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Function to show help
show_help() {
    cat << EOF
Usage: ./build.sh [options]

Options:
  --debug          Build in Debug mode (default: Release)
  --release        Build in Release mode
  --no-crt         Build without C Runtime Library
  --crt            Build with C Runtime Library (default)
  --generator GEN  Specify CMake generator (default: Ninja)
  --build-dir DIR  Specify build directory (default: build)
  --clean          Clean build directory before building
  --help           Show this help message

Examples:
  ./build.sh --release --crt
  ./build.sh --debug --no-crt
  ./build.sh --release --no-crt --build-dir build-nocrt
  ./build.sh --clean --release

Note: This requires a Windows cross-compiler (mingw-w64) on Linux/Unix
EOF
}

# Parse arguments
CLEAN=0
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --no-crt)
            USE_CRT="OFF"
            shift
            ;;
        --crt)
            USE_CRT="ON"
            shift
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake not found in PATH"
    exit 1
fi

# Print configuration
echo "============================================================================"
echo "Universal DLL Build Configuration"
echo "============================================================================"
echo "Build Type: $BUILD_TYPE"
echo "Use CRT: $USE_CRT"
echo "Build Directory: $BUILD_DIR"
echo "Generator: $GENERATOR"
echo "============================================================================"
echo ""

# Clean if requested
if [ $CLEAN -eq 1 ]; then
    if [ -d "$BUILD_DIR" ]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Configure with CMake
print_info "Configuring project..."
cmake -S . -B "$BUILD_DIR" \
    -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DUSE_CRT="$USE_CRT" \
    -DBUILD_SHARED=ON

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

# Build
echo ""
print_info "Building project..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -- -j$(nproc)

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

echo ""
echo "============================================================================"
print_info "Build completed successfully!"
echo "Output: $BUILD_DIR/$BUILD_TYPE/UniversalDLL.dll"
echo "============================================================================"