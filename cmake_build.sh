#!/bin/bash
# CMake build script for TinyWebServer

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=false
INSTALL=false
JOBS=$(nproc)

# Print usage
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -d, --debug         Build in Debug mode (default: Release)"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -i, --install       Install after building"
    echo "  -j, --jobs N        Number of parallel jobs (default: $(nproc))"
    echo "  -h, --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                  # Build in Release mode"
    echo "  $0 -d               # Build in Debug mode"
    echo "  $0 -c -d            # Clean build and build in Debug mode"
    echo "  $0 -i               # Build and install"
    exit 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            usage
            ;;
    esac
done

echo -e "${GREEN}=== TinyWebServer CMake Build ===${NC}"
echo -e "${YELLOW}Build Type: ${BUILD_TYPE}${NC}"
echo -e "${YELLOW}Build Directory: ${BUILD_DIR}${NC}"
echo -e "${YELLOW}Parallel Jobs: ${JOBS}${NC}"
echo ""

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo -e "${GREEN}Configuring CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . -j "$JOBS"

echo ""
echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${YELLOW}Executable location: ${BUILD_DIR}/bin/server${NC}"

# Install if requested
if [ "$INSTALL" = true ]; then
    echo ""
    echo -e "${GREEN}Installing...${NC}"
    sudo cmake --install .
    echo -e "${GREEN}Installation completed!${NC}"
fi

echo ""
echo -e "${GREEN}=== Build Summary ===${NC}"
echo -e "  Build Type: ${BUILD_TYPE}"
echo -e "  Executable: ${BUILD_DIR}/bin/server"
if [ "$INSTALL" = true ]; then
    echo -e "  Installed: Yes"
fi
echo ""
echo -e "${YELLOW}To run the server:${NC}"
echo -e "  cd ${BUILD_DIR}/bin"
echo -e "  ./server [options]"
echo ""
