#!/bin/bash
# Comprehensive Test Runner for Cloud Registration (Linux/macOS)
# This script runs all test suites for Sprint 8 validation

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "PASS")
            echo -e "${GREEN}✓ $message${NC}"
            ;;
        "FAIL")
            echo -e "${RED}✗ $message${NC}"
            ;;
        "WARN")
            echo -e "${YELLOW}⚠ $message${NC}"
            ;;
        "INFO")
            echo -e "${BLUE}ℹ $message${NC}"
            ;;
    esac
}

echo "========================================"
echo "Cloud Registration Comprehensive Testing"
echo "Sprint 8 - Final Validation"
echo "========================================"
echo

# Set test configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"
TEST_RESULTS_DIR="$SCRIPT_DIR/results"
TIMESTAMP=$(date '+%Y-%m-%d_%H-%M-%S')

# Create results directory
mkdir -p "$TEST_RESULTS_DIR"

echo "Test Configuration:"
echo "- Build Directory: $BUILD_DIR"
echo "- Results Directory: $TEST_RESULTS_DIR"
echo "- Timestamp: $TIMESTAMP"
echo

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    print_status "FAIL" "Build directory not found: $BUILD_DIR"
    echo "Please build the project first using CMake"
    exit 1
fi

# Initialize test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
TEST_SUITES=0

echo "========================================"
echo "Phase 1: Unit Tests"
echo "========================================"

# Run unit tests
UNIT_TESTS="test_e57parser test_lasparser test_mainpresenter test_projectmanager test_pointcloudviewer test_voxelgridfilter"

for test in $UNIT_TESTS; do
    echo
    echo "Running unit test: $test"
    echo "----------------------------------------"
    
    TEST_EXECUTABLE="$BUILD_DIR/tests/$test"
    
    if [ -x "$TEST_EXECUTABLE" ]; then
        if "$TEST_EXECUTABLE" --gtest_output=xml:"$TEST_RESULTS_DIR/${test}_results.xml"; then
            print_status "PASS" "$test PASSED"
            ((PASSED_TESTS++))
        else
            EXIT_CODE=$?
            print_status "FAIL" "$test FAILED (Exit Code: $EXIT_CODE)"
            ((FAILED_TESTS++))
        fi
        ((TOTAL_TESTS++))
        ((TEST_SUITES++))
    else
        print_status "WARN" "$test executable not found: $TEST_EXECUTABLE"
        ((FAILED_TESTS++))
        ((TOTAL_TESTS++))
    fi
done

echo
echo "========================================"
echo "Phase 2: Integration Tests"
echo "========================================"

# Run integration tests
echo
echo "Running integration tests..."
echo "----------------------------------------"

INTEGRATION_TEST="$BUILD_DIR/tests/test_integration"

if [ -x "$INTEGRATION_TEST" ]; then
    if "$INTEGRATION_TEST" --gtest_output=xml:"$TEST_RESULTS_DIR/integration_results.xml"; then
        print_status "PASS" "Integration tests PASSED"
        ((PASSED_TESTS++))
    else
        EXIT_CODE=$?
        print_status "FAIL" "Integration tests FAILED (Exit Code: $EXIT_CODE)"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    ((TEST_SUITES++))
else
    print_status "WARN" "Integration test executable not found: $INTEGRATION_TEST"
    ((FAILED_TESTS++))
    ((TOTAL_TESTS++))
fi

echo
echo "========================================"
echo "Phase 3: End-to-End Tests"
echo "========================================"

# Run end-to-end tests
echo
echo "Running end-to-end tests..."
echo "----------------------------------------"

E2E_TEST="$BUILD_DIR/tests/end_to_end_testing"

if [ -x "$E2E_TEST" ]; then
    if "$E2E_TEST" --gtest_output=xml:"$TEST_RESULTS_DIR/e2e_results.xml"; then
        print_status "PASS" "End-to-end tests PASSED"
        ((PASSED_TESTS++))
    else
        EXIT_CODE=$?
        print_status "FAIL" "End-to-end tests FAILED (Exit Code: $EXIT_CODE)"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    ((TEST_SUITES++))
else
    print_status "WARN" "End-to-end test executable not found: $E2E_TEST"
    ((FAILED_TESTS++))
    ((TOTAL_TESTS++))
fi

echo
echo "========================================"
echo "Test Results Summary"
echo "========================================"

echo
echo "Test Execution Completed: $(date)"
echo "Total Test Suites: $TEST_SUITES"
echo "Total Tests: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo
    print_status "PASS" "ALL TESTS PASSED - Sprint 8 validation successful!"
    echo
    OVERALL_RESULT="PASS"
else
    echo
    print_status "FAIL" "$FAILED_TESTS TEST(S) FAILED - Sprint 8 validation incomplete"
    echo
    OVERALL_RESULT="FAIL"
fi

# Generate summary report
SUMMARY_FILE="$TEST_RESULTS_DIR/test_summary_$TIMESTAMP.txt"

cat > "$SUMMARY_FILE" << EOF
Cloud Registration Test Summary
=================================

Test Date: $(date)
Sprint: 8 - Final Validation
Overall Result: $OVERALL_RESULT

Test Statistics:
- Total Test Suites: $TEST_SUITES
- Total Tests: $TOTAL_TESTS
- Passed: $PASSED_TESTS
- Failed: $FAILED_TESTS

EOF

if [ $FAILED_TESTS -gt 0 ]; then
    echo "Failed Tests:" >> "$SUMMARY_FILE"
    echo "See individual test result files for details" >> "$SUMMARY_FILE"
fi

echo
print_status "INFO" "Test summary saved to: $SUMMARY_FILE"
print_status "INFO" "Test results available in: $TEST_RESULTS_DIR"

# Set exit code based on test results
if [ $FAILED_TESTS -eq 0 ]; then
    exit 0
else
    exit 1
fi
