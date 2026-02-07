#!/bin/bash
# Installation script for Find My Files

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="/usr/local/bin"
BUILD_TYPE="Release"
BUILD_DIR="build"

# Print with color
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Print header
echo "=========================================="
echo "  Find My Files - Installation Script"
echo "=========================================="
echo ""

# Check prerequisites
print_info "Checking prerequisites..."

if ! command_exists cmake; then
    print_error "CMake is not installed. Please install CMake 3.14 or higher."
    exit 1
fi

if ! command_exists g++ && ! command_exists clang++; then
    print_error "No C++ compiler found. Please install g++ or clang++."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
print_info "Found CMake version: $CMAKE_VERSION"

if command_exists g++; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_info "Found compiler: $GCC_VERSION"
elif command_exists clang++; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    print_info "Found compiler: $CLANG_VERSION"
fi

# Parse command line arguments
INSTALL_SYSTEM=false
RUN_TESTS=true
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --system)
            INSTALL_SYSTEM=true
            shift
            ;;
        --no-tests)
            RUN_TESTS=false
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --system      Install to system directory ($INSTALL_DIR)"
            echo "               Requires sudo privileges"
            echo "  --no-tests    Skip running tests"
            echo "  --clean       Clean build directory before building"
            echo "  --help, -h    Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                   # Build and run tests"
            echo "  $0 --system          # Build and install to system"
            echo "  $0 --clean --system  # Clean build and install to system"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help to see available options"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    if [ -d "$BUILD_DIR" ]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
fi

# Create build directory
print_info "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
print_info "Configuring project (${BUILD_TYPE} build)..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE .. || {
    print_error "CMake configuration failed"
    exit 1
}

# Build
print_info "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
    print_error "Build failed"
    exit 1
}

print_info "Build completed successfully!"

# Run tests if enabled
if [ "$RUN_TESTS" = true ]; then
    print_info "Running tests..."
    if ./tests; then
        print_info "All tests passed!"
    else
        print_warning "Some tests failed. Continue installation? (y/n)"
        read -r response
        if [[ ! "$response" =~ ^[Yy]$ ]]; then
            print_error "Installation aborted"
            exit 1
        fi
    fi
fi

# Install
if [ "$INSTALL_SYSTEM" = true ]; then
    print_info "Installing to system directory: $INSTALL_DIR"
    
    if [ ! -w "$INSTALL_DIR" ]; then
        print_warning "Need sudo privileges to install to $INSTALL_DIR"
        sudo cp find_my_files "$INSTALL_DIR/" || {
            print_error "Installation failed"
            exit 1
        }
    else
        cp find_my_files "$INSTALL_DIR/" || {
            print_error "Installation failed"
            exit 1
        }
    fi
    
    print_info "Installation completed!"
    print_info "You can now run 'find_my_files' from anywhere"
    
    # Check if installed successfully
    if command_exists find_my_files; then
        INSTALLED_VERSION=$(find_my_files --help | head -n1 || echo "unknown")
        print_info "Installed version: $INSTALLED_VERSION"
    fi
else
    print_info "Build completed successfully!"
    print_info "Binary location: $(pwd)/find_my_files"
    print_info ""
    print_info "To install system-wide, run:"
    print_info "  sudo cp find_my_files $INSTALL_DIR/"
    print_info "Or re-run this script with --system flag:"
    print_info "  ./install.sh --system"
fi

echo ""
echo "=========================================="
print_info "Installation process completed!"
echo "=========================================="

# Show usage hint
if [ "$INSTALL_SYSTEM" = false ]; then
    echo ""
    print_info "Quick Start:"
    echo "  ./build/find_my_files --help"
    echo "  ./build/find_my_files -r ."
fi
