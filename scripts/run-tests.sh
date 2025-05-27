#!/bin/bash
# Cloud Registration Test Runner Script
# Implements Phase 1 testing infrastructure from PRD

set -e

# Default values
BUILD_DIR="build"
CONFIG="Debug"
COVERAGE=false
VERBOSE=false
FILTER=""
HELP=false

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

show_help() {
    echo -e "${GREEN}Cloud Registration Test Runner${NC}"
    echo ""
    echo -e "${YELLOW}Usage: ./scripts/run-tests.sh [options]${NC}"
    echo ""
    echo -e "${CYAN}Options:${NC}"
    echo "  -d, --build-dir <path>    Build directory (default: build)"
    echo "  -c, --config <config>     Build configuration (default: Debug)"
    echo "  -C, --coverage            Generate code coverage report"
    echo "  -v, --verbose             Show detailed test output"
    echo "  -f, --filter <pattern>    Run only tests matching pattern"
    echo "  -h, --help                Show this help message"
    echo ""
    echo -e "${YELLOW}Examples:${NC}"
    echo "  ./scripts/run-tests.sh                    # Run all tests"
    echo "  ./scripts/run-tests.sh --coverage         # Run tests with coverage"
    echo "  ./scripts/run-tests.sh --filter '*E57*'   # Run only E57 tests"
    echo "  ./scripts/run-tests.sh --verbose          # Show detailed output"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -c|--config)
            CONFIG="$2"
            shift 2
            ;;
        -C|--coverage)
            COVERAGE=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -f|--filter)
            FILTER="$2"
            shift 2
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

echo -e "${GREEN}=== Cloud Registration Test Runner ===${NC}"
echo -e "${CYAN}Build Directory: $BUILD_DIR${NC}"
echo -e "${CYAN}Configuration: $CONFIG${NC}"

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}ERROR: Build directory '$BUILD_DIR' does not exist${NC}"
    echo -e "${YELLOW}Please run cmake to configure the project first${NC}"
    exit 1
fi

# Change to build directory
cd "$BUILD_DIR"

# Check if Google Test was found during configuration
if [ -f "CMakeCache.txt" ]; then
    if ! grep -q "GTest_FOUND:BOOL=TRUE" CMakeCache.txt; then
        echo -e "${YELLOW}WARNING: Google Test not found during configuration${NC}"
        echo -e "${YELLOW}Tests will not be available. Please install Google Test:${NC}"
        echo -e "${WHITE}  - Ubuntu/Debian: sudo apt-get install libgtest-dev${NC}"
        echo -e "${WHITE}  - macOS: brew install googletest${NC}"
        exit 1
    fi
fi

echo -e "\n${CYAN}1. Building test executables...${NC}"

# Build all test targets
TEST_TARGETS=("E57ParserTests" "LasParserTests" "VoxelGridFilterTests" "Sprint1FunctionalityTests")

for target in "${TEST_TARGETS[@]}"; do
    echo -e "${WHITE}Building $target...${NC}"
    if ! cmake --build . --target "$target" --config "$CONFIG"; then
        echo -e "${RED}ERROR: Failed to build $target${NC}"
        exit 1
    fi
done

echo -e "${GREEN}✓ All test executables built successfully${NC}"

echo -e "\n${CYAN}2. Running unit tests...${NC}"

# Prepare CTest arguments
CTEST_ARGS=("--output-on-failure" "--config" "$CONFIG")

if [ "$VERBOSE" = true ]; then
    CTEST_ARGS+=("--verbose")
fi

if [ -n "$FILTER" ]; then
    CTEST_ARGS+=("-R" "$FILTER")
fi

# Run tests using CTest
echo -e "${WHITE}Executing: ctest ${CTEST_ARGS[*]}${NC}"
if ctest "${CTEST_ARGS[@]}"; then
    TEST_EXIT_CODE=0
    echo -e "${GREEN}✓ All tests passed!${NC}"
else
    TEST_EXIT_CODE=$?
    echo -e "${RED}✗ Some tests failed (exit code: $TEST_EXIT_CODE)${NC}"
fi

# Show test summary
echo -e "\n${CYAN}3. Test Summary:${NC}"
TEST_OUTPUT=$(ctest "${CTEST_ARGS[@]}" 2>&1 || true)

# Parse test results
PASSED_TESTS=$(echo "$TEST_OUTPUT" | grep -c "Test #[0-9]*: .* \.\.\. Passed" || echo "0")
FAILED_TESTS=$(echo "$TEST_OUTPUT" | grep -c "Test #[0-9]*: .* \.\.\. Failed" || echo "0")
TOTAL_TESTS=$((PASSED_TESTS + FAILED_TESTS))

echo -e "${WHITE}Total Tests: $TOTAL_TESTS${NC}"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
if [ "$FAILED_TESTS" -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
else
    echo -e "${GREEN}Failed: $FAILED_TESTS${NC}"
fi

# Generate coverage report if requested
if [ "$COVERAGE" = true ]; then
    echo -e "\n${CYAN}4. Generating code coverage report...${NC}"
    
    # Check if coverage tools are available
    if ! command -v lcov &> /dev/null || ! command -v genhtml &> /dev/null; then
        echo -e "${YELLOW}WARNING: lcov/genhtml not found. Coverage report not available.${NC}"
        echo -e "${WHITE}To install on Ubuntu/Debian: sudo apt-get install lcov${NC}"
        echo -e "${WHITE}To install on macOS: brew install lcov${NC}"
    else
        # Run coverage target if available
        if cmake --build . --target coverage --config "$CONFIG"; then
            echo -e "${GREEN}✓ Coverage report generated successfully${NC}"
            
            if [ -f "coverage_html/index.html" ]; then
                COVERAGE_FILE=$(realpath "coverage_html/index.html")
                echo -e "${CYAN}Coverage report available at: $COVERAGE_FILE${NC}"
                
                # Try to open coverage report in default browser
                if command -v xdg-open &> /dev/null; then
                    xdg-open "$COVERAGE_FILE" 2>/dev/null &
                    echo -e "${GREEN}✓ Coverage report opened in browser${NC}"
                elif command -v open &> /dev/null; then
                    open "$COVERAGE_FILE" 2>/dev/null &
                    echo -e "${GREEN}✓ Coverage report opened in browser${NC}"
                else
                    echo -e "${YELLOW}Note: Could not auto-open coverage report${NC}"
                fi
            fi
        else
            echo -e "${RED}✗ Failed to generate coverage report${NC}"
        fi
    fi
fi

# Individual test execution (for detailed output)
if [ "$VERBOSE" = true ] && [ "$TEST_EXIT_CODE" -ne 0 ]; then
    echo -e "\n${CYAN}5. Running individual tests for detailed output...${NC}"
    
    for target in "${TEST_TARGETS[@]}"; do
        # Find the executable
        EXE_PATH=""
        POSSIBLE_PATHS=(
            "bin/$CONFIG/$target"
            "bin/$target"
            "$CONFIG/$target"
            "$target"
        )
        
        for path in "${POSSIBLE_PATHS[@]}"; do
            if [ -x "$path" ]; then
                EXE_PATH="$path"
                break
            fi
        done
        
        if [ -n "$EXE_PATH" ]; then
            echo -e "\n${YELLOW}Running $target individually:${NC}"
            
            ARGS=("--gtest_color=yes")
            if [ -n "$FILTER" ]; then
                ARGS+=("--gtest_filter=$FILTER")
            fi
            
            "./$EXE_PATH" "${ARGS[@]}" || true
        else
            echo -e "${YELLOW}Could not find executable for $target${NC}"
        fi
    done
fi

echo -e "\n${GREEN}=== Test Execution Complete ===${NC}"

# Return appropriate exit code
exit $TEST_EXIT_CODE
