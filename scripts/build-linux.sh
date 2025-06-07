#!/bin/bash
# Cloud Registration Linux Build Script
# Cross-platform build script for Linux systems

set -e

# Default values
BUILD_TYPE="Debug"
COMPILER="gcc"
CLEAN_BUILD=false
VERBOSE=false
ENABLE_TESTS=true
ENABLE_COVERAGE=false
JOBS=$(nproc)
HELP=false

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

show_help() {
    echo -e "${GREEN}Cloud Registration Linux Build Script${NC}"
    echo ""
    echo -e "${YELLOW}Usage: ./scripts/build-linux.sh [options]${NC}"
    echo ""
    echo -e "${CYAN}Options:${NC}"
    echo "  -t, --type <type>         Build type: Debug, Release (default: Debug)"
    echo "  -c, --compiler <comp>     Compiler: gcc, clang (default: gcc)"
    echo "  -j, --jobs <num>          Number of parallel jobs (default: $(nproc))"
    echo "  -C, --clean               Clean build directory before building"
    echo "  -v, --verbose             Show verbose build output"
    echo "  --no-tests                Disable building tests"
    echo "  --coverage                Enable code coverage (Debug builds only)"
    echo "  -h, --help                Show this help message"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  ./scripts/build-linux.sh                           # Debug build with GCC"
    echo "  ./scripts/build-linux.sh -t Release -c clang       # Release build with Clang"
    echo "  ./scripts/build-linux.sh --clean --coverage        # Clean Debug build with coverage"
    echo "  ./scripts/build-linux.sh -t Release -j 8           # Release build with 8 parallel jobs"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -c|--compiler)
            COMPILER="$2"
            shift 2
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -C|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --no-tests)
            ENABLE_TESTS=false
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE=true
            shift
            ;;
        -h|--help)
            HELP=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

if [ "$HELP" = true ]; then
    show_help
    exit 0
fi

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo -e "${RED}ERROR: Invalid build type '$BUILD_TYPE'. Must be Debug or Release.${NC}"
    exit 1
fi

# Validate compiler
if [[ "$COMPILER" != "gcc" && "$COMPILER" != "clang" ]]; then
    echo -e "${RED}ERROR: Invalid compiler '$COMPILER'. Must be gcc or clang.${NC}"
    exit 1
fi

# Coverage only makes sense for Debug builds
if [ "$ENABLE_COVERAGE" = true ] && [ "$BUILD_TYPE" != "Debug" ]; then
    echo -e "${YELLOW}WARNING: Code coverage is only supported for Debug builds. Disabling coverage.${NC}"
    ENABLE_COVERAGE=false
fi

echo -e "${GREEN}=== Cloud Registration Linux Build ===${NC}"
echo -e "${CYAN}Build Type: $BUILD_TYPE${NC}"
echo -e "${CYAN}Compiler: $COMPILER${NC}"
echo -e "${CYAN}Jobs: $JOBS${NC}"
echo -e "${CYAN}Tests: $([ "$ENABLE_TESTS" = true ] && echo "Enabled" || echo "Disabled")${NC}"
echo -e "${CYAN}Coverage: $([ "$ENABLE_COVERAGE" = true ] && echo "Enabled" || echo "Disabled")${NC}"

# Check if required tools are installed
echo -e "\n${CYAN}1. Checking build dependencies...${NC}"

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake not found. Please install CMake.${NC}"
    echo -e "${WHITE}Ubuntu/Debian: sudo apt-get install cmake${NC}"
    echo -e "${WHITE}Fedora/RHEL: sudo dnf install cmake${NC}"
    echo -e "${WHITE}Arch: sudo pacman -S cmake${NC}"
    exit 1
fi

# Check Ninja
if ! command -v ninja &> /dev/null; then
    echo -e "${RED}ERROR: Ninja not found. Please install Ninja build system.${NC}"
    echo -e "${WHITE}Ubuntu/Debian: sudo apt-get install ninja-build${NC}"
    echo -e "${WHITE}Fedora/RHEL: sudo dnf install ninja-build${NC}"
    echo -e "${WHITE}Arch: sudo pacman -S ninja${NC}"
    exit 1
fi

# Check compiler
if [ "$COMPILER" = "gcc" ]; then
    if ! command -v g++ &> /dev/null; then
        echo -e "${RED}ERROR: g++ not found. Please install GCC.${NC}"
        echo -e "${WHITE}Ubuntu/Debian: sudo apt-get install build-essential${NC}"
        exit 1
    fi
    CC_COMPILER="gcc"
    CXX_COMPILER="g++"
elif [ "$COMPILER" = "clang" ]; then
    if ! command -v clang++ &> /dev/null; then
        echo -e "${RED}ERROR: clang++ not found. Please install Clang.${NC}"
        echo -e "${WHITE}Ubuntu/Debian: sudo apt-get install clang${NC}"
        echo -e "${WHITE}Fedora/RHEL: sudo dnf install clang${NC}"
        echo -e "${WHITE}Arch: sudo pacman -S clang${NC}"
        exit 1
    fi
    CC_COMPILER="clang"
    CXX_COMPILER="clang++"
fi

echo -e "${GREEN}✓ All build dependencies found${NC}"

# Set build directory
BUILD_DIR="build-linux-${COMPILER}-$(echo $BUILD_TYPE | tr '[:upper:]' '[:lower:]')"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "\n${CYAN}2. Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}✓ Build directory cleaned${NC}"
fi

# Create and enter build directory
echo -e "\n${CYAN}3. Configuring project...${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Prepare CMake arguments
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DCMAKE_C_COMPILER=$CC_COMPILER"
    "-DCMAKE_CXX_COMPILER=$CXX_COMPILER"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    "-G" "Ninja"
)

if [ "$ENABLE_COVERAGE" = true ]; then
    CMAKE_ARGS+=("-DENABLE_COVERAGE=ON")
fi

# Configure with CMake
echo -e "${WHITE}Executing: cmake .. ${CMAKE_ARGS[*]}${NC}"
if cmake .. "${CMAKE_ARGS[@]}"; then
    echo -e "${GREEN}✓ Project configured successfully${NC}"
else
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
fi

# Build the project
echo -e "\n${CYAN}4. Building project...${NC}"

NINJA_ARGS=()
if [ "$VERBOSE" = true ]; then
    NINJA_ARGS+=("-v")
fi

if [ "$JOBS" -gt 1 ]; then
    NINJA_ARGS+=("-j" "$JOBS")
fi

echo -e "${WHITE}Executing: ninja ${NINJA_ARGS[*]}${NC}"
if ninja "${NINJA_ARGS[@]}"; then
    echo -e "${GREEN}✓ Project built successfully${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

# Check if executable was created
if [ -f "bin/CloudRegistration" ]; then
    echo -e "${GREEN}✓ Executable created: $BUILD_DIR/bin/CloudRegistration${NC}"
elif [ -f "CloudRegistration" ]; then
    echo -e "${GREEN}✓ Executable created: $BUILD_DIR/CloudRegistration${NC}"
else
    echo -e "${YELLOW}WARNING: Executable not found in expected location${NC}"
fi

# Build and run tests if enabled
if [ "$ENABLE_TESTS" = true ]; then
    echo -e "\n${CYAN}5. Building and running tests...${NC}"
    
    # Build test targets
    if ninja run_tests; then
        echo -e "${GREEN}✓ Tests completed successfully${NC}"
    else
        echo -e "${RED}✗ Some tests failed${NC}"
        # Don't exit on test failure, just warn
    fi
fi

echo -e "\n${GREEN}=== Build Complete ===${NC}"
echo -e "${CYAN}Build Directory: $(pwd)${NC}"
echo -e "${CYAN}Executable: $(find . -name 'CloudRegistration' -type f | head -1)${NC}"

if [ "$ENABLE_COVERAGE" = true ]; then
    echo -e "${CYAN}Coverage Report: $(find . -name 'index.html' -path '*/coverage_html/*' | head -1)${NC}"
fi

echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo -e "${WHITE}1. Run the application: ./$BUILD_DIR/bin/CloudRegistration${NC}"
echo -e "${WHITE}2. Run tests manually: cd $BUILD_DIR && ctest --output-on-failure${NC}"
echo -e "${WHITE}3. Install: cd $BUILD_DIR && sudo ninja install${NC}"
