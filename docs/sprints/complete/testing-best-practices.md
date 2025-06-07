# Testing Best Practices for Cloud Registration Project

## Overview

This document outlines the testing standards and best practices for the Cloud Registration project, implementing Phase 1 of the Google Test PRD. These guidelines ensure consistent, maintainable, and effective unit testing across all modules.

## Testing Framework

- **Primary Framework**: Google Test (GTest)
- **Qt Integration**: Qt Test framework for Qt-specific functionality
- **Build System**: CMake with CTest integration
- **Code Coverage**: GCOV/LCOV for coverage reporting

## Test Organization

### Directory Structure
```
tests/
├── test_e57parser.cpp          # E57 parser unit tests
├── test_lasparser.cpp          # LAS parser unit tests  
├── test_voxelgridfilter.cpp    # VoxelGrid filter unit tests
├── test_sprint1_functionality.cpp # Sprint 1 integration tests
└── test_point_cloud_registration.cpp # Future: main integration tests
```

### File Naming Conventions
- Test files: `test_<module_name>.cpp`
- Test classes: `<ModuleName>Test` (e.g., `E57ParserTest`)
- Test methods: `<Functionality>_<Scenario>` (e.g., `ValidE57FileDetection`)

## Test Structure Standards

### Test Fixture Pattern
All test classes should inherit from `::testing::Test` and follow this pattern:

```cpp
class ModuleNameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize Qt application if needed
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
        
        // Initialize test objects
        objectUnderTest = new ModuleClass();
    }
    
    void TearDown() override
    {
        delete objectUnderTest;
        // Don't delete app - shared across tests
    }
    
    // Helper methods for test data creation
    QString createTestFile() { /* implementation */ }
    
    QCoreApplication* app = nullptr;
    ModuleClass* objectUnderTest = nullptr;
};
```

### Arrange-Act-Assert Pattern
Every test should follow the AAA pattern:

```cpp
TEST_F(ModuleNameTest, SpecificFunctionality_ExpectedScenario)
{
    // Arrange: Set up test data and conditions
    QString testFile = createTestFile();
    ExpectedType expectedResult = /* expected value */;
    
    // Act: Execute the functionality being tested
    ActualType actualResult = objectUnderTest->methodUnderTest(testFile);
    
    // Assert: Verify the results
    EXPECT_EQ(actualResult, expectedResult);
    
    // Cleanup if needed
    QFile::remove(testFile);
}
```

## Assertion Guidelines

### Preferred Assertions
- Use `EXPECT_*` for non-fatal assertions (test continues)
- Use `ASSERT_*` for fatal assertions (test stops on failure)
- Use `FAIL()` for explicit test failures
- Use `SUCCEED()` for explicit test success

### Specific Assertion Types
```cpp
// Equality
EXPECT_EQ(actual, expected);
EXPECT_NE(actual, unexpected);

// Floating point comparisons
EXPECT_FLOAT_EQ(actual, expected);
EXPECT_NEAR(actual, expected, tolerance);

// Boolean conditions
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// String comparisons
EXPECT_STREQ(actual, expected);

// Container checks
EXPECT_TRUE(container.empty());
EXPECT_EQ(container.size(), expectedSize);

// Exception testing
EXPECT_THROW(statement, ExceptionType);
EXPECT_NO_THROW(statement);
```

## Test Categories

### 1. Unit Tests
Test individual functions/methods in isolation:
- Valid input handling
- Invalid input handling
- Edge cases
- Error conditions
- Boundary values

### 2. Integration Tests
Test interactions between components:
- Module-to-module communication
- Settings persistence
- Signal/slot connections
- File I/O operations

### 3. Performance Tests
Test performance characteristics:
```cpp
TEST_F(ModuleTest, PerformanceTest)
{
    auto start = std::chrono::high_resolution_clock::now();
    
    // Execute operation
    result = objectUnderTest->expensiveOperation(largeDataSet);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000); // Should complete in < 1 second
}
```

## Test Data Management

### Temporary Files
Always use Qt's temporary file mechanisms:
```cpp
QString createTestFile()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    tempFile->setAutoRemove(false);
    
    if (!tempFile->open()) {
        return QString();
    }
    
    // Write test data
    // ...
    
    QString fileName = tempFile->fileName();
    tempFile->close();
    delete tempFile;
    
    return fileName;
}
```

### Cleanup
Always clean up test resources:
```cpp
// In test method
QString testFile = createTestFile();
// ... test logic ...
QFile::remove(testFile); // Always cleanup
```

## Qt-Specific Testing

### QApplication Initialization
For tests requiring Qt widgets:
```cpp
void SetUp() override
{
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        app = new QCoreApplication(argc, argv);
    }
}
```

### Signal Testing
Use QSignalSpy for signal verification:
```cpp
TEST_F(ParserTest, SignalEmission)
{
    QSignalSpy spy(parser, &Parser::progressUpdated);
    
    parser->parse(testFile);
    
    EXPECT_GT(spy.count(), 0);
}
```

## Error Handling Tests

### Exception Testing
```cpp
TEST_F(ParserTest, InvalidFileHandling)
{
    try {
        parser->parse("/invalid/path");
        FAIL() << "Expected exception was not thrown";
    } catch (const ParseException& e) {
        EXPECT_FALSE(parser->getLastError().isEmpty());
    } catch (const std::exception& e) {
        FAIL() << "Wrong exception type: " << e.what();
    }
}
```

## Test Independence

### Requirements
- Each test must be independent and repeatable
- Tests should not depend on execution order
- Tests should not share state between runs
- Use SetUp()/TearDown() for consistent initialization

### Avoiding Common Pitfalls
```cpp
// BAD: Static/global state
static int testCounter = 0; // Don't do this

// GOOD: Instance state in test fixture
class MyTest : public ::testing::Test {
protected:
    int testCounter = 0; // Reset for each test
};
```

## Running Tests

### Command Line
```bash
# Build tests
cmake --build build --target AllTests

# Run all tests
ctest --output-on-failure

# Run specific test
./build/E57ParserTests

# Run with filter
./build/AllTests --gtest_filter="*ValidFile*"

# Generate coverage report (if enabled)
cmake --build build --target coverage
```

### CMake Integration
Tests are automatically discovered and can be run via:
```bash
make run_tests  # Custom target
ctest          # Standard CTest
```

## Code Coverage

### Enabling Coverage
```bash
cmake -DENABLE_COVERAGE=ON ..
make coverage
```

### Coverage Targets
- **Minimum**: 70% for new code
- **Goal**: 80% overall coverage
- **Critical paths**: 90%+ coverage required

### Coverage Reports
HTML reports generated in `build/coverage_html/index.html`

## Continuous Integration

### Test Execution
- All tests run on every commit
- Tests must pass before merge
- Coverage reports generated automatically
- Performance regression detection

### Test Requirements
- All new code must include tests
- Tests must pass on all supported platforms
- No test should take longer than 30 seconds
- Total test suite should complete in under 5 minutes

## Common Patterns

### Parameterized Tests
For testing multiple similar scenarios:
```cpp
class ParameterizedTest : public ::testing::TestWithParam<TestData> {};

TEST_P(ParameterizedTest, TestName)
{
    TestData data = GetParam();
    // Test with data
}

INSTANTIATE_TEST_SUITE_P(TestSuite, ParameterizedTest,
    ::testing::Values(data1, data2, data3));
```

### Mock Data Generation
Create realistic test data:
```cpp
std::vector<float> generateMockPointCloud(int numPoints)
{
    std::vector<float> points;
    points.reserve(numPoints * 3);
    
    for (int i = 0; i < numPoints; ++i) {
        points.push_back(static_cast<float>(i));      // X
        points.push_back(static_cast<float>(i + 1));  // Y  
        points.push_back(static_cast<float>(i + 2));  // Z
    }
    
    return points;
}
```

## Review Checklist

Before submitting tests, verify:
- [ ] Tests follow naming conventions
- [ ] AAA pattern is used consistently
- [ ] Proper assertions are chosen
- [ ] Test independence is maintained
- [ ] Resources are properly cleaned up
- [ ] Error cases are tested
- [ ] Performance is reasonable
- [ ] Qt integration is correct
- [ ] Documentation is clear

## Future Enhancements

### Phase 2 Additions
- Parameterized tests for comprehensive coverage
- Death tests for critical error conditions
- Custom matchers for complex assertions
- Mock framework integration (Google Mock)

### Advanced Features
- Benchmark integration
- Fuzz testing capabilities
- Property-based testing
- Visual test result reporting
