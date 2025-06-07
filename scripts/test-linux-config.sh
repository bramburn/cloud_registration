#!/bin/bash
# Cloud Registration Linux Configuration Test Script
# Tests the Linux build configuration without requiring all dependencies

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Cloud Registration Linux Configuration Test ===${NC}"

# Test 1: CMake Version
echo -e "\n${CYAN}1. Testing CMake version...${NC}"
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo -e "${GREEN}✓ CMake found: $CMAKE_VERSION${NC}"
    
    # Check if version is >= 3.16
    if [ "$(printf '%s\n' "3.16" "$CMAKE_VERSION" | sort -V | head -n1)" = "3.16" ]; then
        echo -e "${GREEN}✓ CMake version is sufficient (>= 3.16)${NC}"
    else
        echo -e "${RED}✗ CMake version too old (< 3.16)${NC}"
    fi
else
    echo -e "${RED}✗ CMake not found${NC}"
fi

# Test 2: Compiler Detection
echo -e "\n${CYAN}2. Testing compiler availability...${NC}"

# Test GCC
if command -v gcc &> /dev/null && command -v g++ &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+' | head -n1)
    echo -e "${GREEN}✓ GCC found: $GCC_VERSION${NC}"
    
    # Check if version is >= 8.0
    if [ "$(printf '%s\n' "8.0" "$GCC_VERSION" | sort -V | head -n1)" = "8.0" ]; then
        echo -e "${GREEN}✓ GCC version is sufficient (>= 8.0)${NC}"
    else
        echo -e "${YELLOW}⚠ GCC version may be too old (< 8.0)${NC}"
    fi
else
    echo -e "${RED}✗ GCC not found${NC}"
fi

# Test Clang
if command -v clang &> /dev/null && command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+' | head -n1)
    echo -e "${GREEN}✓ Clang found: $CLANG_VERSION${NC}"
    
    # Check if version is >= 7.0
    if [ "$(printf '%s\n' "7.0" "$CLANG_VERSION" | sort -V | head -n1)" = "7.0" ]; then
        echo -e "${GREEN}✓ Clang version is sufficient (>= 7.0)${NC}"
    else
        echo -e "${YELLOW}⚠ Clang version may be too old (< 7.0)${NC}"
    fi
else
    echo -e "${YELLOW}⚠ Clang not found (optional)${NC}"
fi

# Test 3: Build System
echo -e "\n${CYAN}3. Testing build systems...${NC}"

if command -v ninja &> /dev/null; then
    NINJA_VERSION=$(ninja --version)
    echo -e "${GREEN}✓ Ninja found: $NINJA_VERSION${NC}"
else
    echo -e "${YELLOW}⚠ Ninja not found (recommended)${NC}"
fi

if command -v make &> /dev/null; then
    MAKE_VERSION=$(make --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+' | head -n1)
    echo -e "${GREEN}✓ Make found: $MAKE_VERSION${NC}"
else
    echo -e "${RED}✗ Make not found${NC}"
fi

# Test 4: CMake Presets
echo -e "\n${CYAN}4. Testing CMake presets...${NC}"

if cmake --list-presets &> /dev/null; then
    echo -e "${GREEN}✓ CMake presets supported${NC}"
    
    # Check for Linux presets
    PRESETS=$(cmake --list-presets 2>/dev/null | grep -E "(linux-gcc|linux-clang)" || echo "")
    if [ -n "$PRESETS" ]; then
        echo -e "${GREEN}✓ Linux presets found:${NC}"
        echo "$PRESETS" | sed 's/^/  /'
    else
        echo -e "${RED}✗ No Linux presets found${NC}"
    fi
else
    echo -e "${YELLOW}⚠ CMake presets not supported (CMake < 3.19)${NC}"
fi

# Test 5: CMake Configuration Test
echo -e "\n${CYAN}5. Testing CMake configuration...${NC}"

# Create temporary build directory
TEST_BUILD_DIR="build-config-test"
rm -rf "$TEST_BUILD_DIR"
mkdir -p "$TEST_BUILD_DIR"

cd "$TEST_BUILD_DIR"

# Test basic CMake configuration
echo -e "${WHITE}Testing basic CMake configuration...${NC}"
if cmake .. -DCMAKE_BUILD_TYPE=Debug &> cmake_config.log; then
    echo -e "${GREEN}✓ CMake configuration successful${NC}"
    
    # Check for specific features
    if grep -q "Found Qt6" cmake_config.log; then
        echo -e "${GREEN}✓ Qt6 found${NC}"
    else
        echo -e "${YELLOW}⚠ Qt6 not found (expected for test environment)${NC}"
    fi
    
    if grep -q "Found E57Format" cmake_config.log; then
        echo -e "${GREEN}✓ E57Format found${NC}"
    else
        echo -e "${YELLOW}⚠ E57Format not found (expected for test environment)${NC}"
    fi
    
    if grep -q "Found GTest" cmake_config.log; then
        echo -e "${GREEN}✓ Google Test found${NC}"
    else
        echo -e "${YELLOW}⚠ Google Test not found (expected for test environment)${NC}"
    fi
    
else
    echo -e "${YELLOW}⚠ CMake configuration failed (expected without dependencies)${NC}"
    
    # Show relevant error messages
    if grep -q "Qt6" cmake_config.log; then
        echo -e "${WHITE}Qt6 status:${NC}"
        grep -A2 -B2 "Qt6" cmake_config.log | sed 's/^/  /'
    fi
fi

cd ..
rm -rf "$TEST_BUILD_DIR"

# Test 6: Platform Detection
echo -e "\n${CYAN}6. Testing platform detection...${NC}"

# Check OS
if [ -f /etc/os-release ]; then
    OS_NAME=$(grep '^NAME=' /etc/os-release | cut -d'"' -f2)
    OS_VERSION=$(grep '^VERSION=' /etc/os-release | cut -d'"' -f2 || echo "Unknown")
    echo -e "${GREEN}✓ OS detected: $OS_NAME $OS_VERSION${NC}"
    
    # Check for supported distributions
    case "$OS_NAME" in
        *Ubuntu*|*Debian*)
            echo -e "${GREEN}✓ Supported distribution (Ubuntu/Debian)${NC}"
            echo -e "${WHITE}Install command: sudo apt-get install qt6-base-dev libgtest-dev${NC}"
            ;;
        *Fedora*|*Red\ Hat*|*CentOS*)
            echo -e "${GREEN}✓ Supported distribution (Fedora/RHEL)${NC}"
            echo -e "${WHITE}Install command: sudo dnf install qt6-qtbase-devel gtest-devel${NC}"
            ;;
        *Arch*)
            echo -e "${GREEN}✓ Supported distribution (Arch Linux)${NC}"
            echo -e "${WHITE}Install command: sudo pacman -S qt6-base gtest${NC}"
            ;;
        *)
            echo -e "${YELLOW}⚠ Unknown distribution, may require manual setup${NC}"
            ;;
    esac
else
    echo -e "${YELLOW}⚠ Cannot detect OS distribution${NC}"
fi

# Check architecture
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
    echo -e "${GREEN}✓ Architecture: $ARCH (supported)${NC}"
else
    echo -e "${YELLOW}⚠ Architecture: $ARCH (may not be fully supported)${NC}"
fi

# Test 7: Development Tools
echo -e "\n${CYAN}7. Testing development tools...${NC}"

# Check Git
if command -v git &> /dev/null; then
    GIT_VERSION=$(git --version | cut -d' ' -f3)
    echo -e "${GREEN}✓ Git found: $GIT_VERSION${NC}"
else
    echo -e "${RED}✗ Git not found${NC}"
fi

# Check pkg-config
if command -v pkg-config &> /dev/null; then
    PKG_CONFIG_VERSION=$(pkg-config --version)
    echo -e "${GREEN}✓ pkg-config found: $PKG_CONFIG_VERSION${NC}"
else
    echo -e "${YELLOW}⚠ pkg-config not found (may be needed for some dependencies)${NC}"
fi

# Summary
echo -e "\n${GREEN}=== Configuration Test Summary ===${NC}"

# Count issues
ERRORS=0
WARNINGS=0

# This is a simplified check - in a real implementation, you'd track these during the tests
if ! command -v cmake &> /dev/null; then ((ERRORS++)); fi
if ! command -v gcc &> /dev/null && ! command -v clang &> /dev/null; then ((ERRORS++)); fi
if ! command -v git &> /dev/null; then ((ERRORS++)); fi

if ! command -v ninja &> /dev/null; then ((WARNINGS++)); fi
if ! command -v pkg-config &> /dev/null; then ((WARNINGS++)); fi

echo -e "${CYAN}Errors: $ERRORS${NC}"
echo -e "${CYAN}Warnings: $WARNINGS${NC}"

if [ $ERRORS -eq 0 ]; then
    echo -e "\n${GREEN}✓ Basic Linux build environment is ready!${NC}"
    echo -e "${WHITE}Next steps:${NC}"
    echo -e "${WHITE}1. Install Qt6: Follow the Linux Setup Guide${NC}"
    echo -e "${WHITE}2. Install dependencies: ./setup.sh --install-deps${NC}"
    echo -e "${WHITE}3. Build project: ./scripts/build-linux.sh${NC}"
else
    echo -e "\n${RED}✗ Linux build environment needs setup${NC}"
    echo -e "${WHITE}Please install missing tools and try again${NC}"
fi

echo -e "\n${CYAN}For complete setup instructions, see:${NC}"
echo -e "${WHITE}- docs/LINUX_SETUP_GUIDE.md${NC}"
echo -e "${WHITE}- ./setup.sh --help${NC}"
echo -e "${WHITE}- ./scripts/build-linux.sh --help${NC}"
