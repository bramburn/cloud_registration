# Advanced Testing Framework - Sprint 2.4

## Quick Start

### Build and Run Tests

```powershell
# Build the advanced test suite
cmake --build . --target Sprint24AdvancedTests

# Generate complex test files (first time setup)
.\Sprint24AdvancedTests.exe --generate-test-files

# Run all advanced tests
.\Sprint24AdvancedTests.exe

# Run stress tests only
.\Sprint24AdvancedTests.exe --stress-test
```

### Test Categories

1. **Very Large Files**: 25M+ point E57 files
2. **Multi-Scan E57**: Files with multiple data3D sections  
3. **Extreme Coordinates**: LAS files with unusual scale/offset
4. **Many VLRs**: LAS files with numerous Variable Length Records
5. **Corrupted Files**: Intentionally malformed files for error testing
6. **Memory Stress**: Files approaching system memory limits
7. **Edge Case PDRF**: Unusual but valid Point Data Record Formats

### Generated Test Files

All test files are created in `tests/data/advanced/`:
- `very_large_25M.e57` - 25 million point E57 file
- `multi_scan_5.e57` - E57 with 5 scan sections
- `extreme_coords.las` - LAS with extreme coordinate values
- `many_vlrs_100.las` - LAS with 100 Variable Length Records
- `corrupted_header.e57` - E57 with corrupted header
- `memory_stress_50M.e57` - 50 million point stress test file
- `edge_case_pdrf3.las` - LAS using Point Data Record Format 3

### Test Reports

After running tests, detailed reports are generated:
- `comprehensive_report.json` - Complete test analysis
- `stress_test_report.json` - Stress test results  
- `sprint24_test_report.json` - Individual test summaries

### Key Metrics Tracked

- **Loading Time**: Time to parse and load point cloud data
- **Memory Usage**: Peak memory consumption during loading
- **Points Loaded**: Number of points successfully extracted
- **Loading Speed**: Points per second performance metric
- **Memory Efficiency**: Points loaded per MB of memory used
- **Memory Leaks**: Detection of memory not properly released

### Expected Results

✅ **Success Criteria**:
- Large files load without crashing
- Memory usage stays within reasonable limits (<8GB for 25M points)
- Loading completes within timeout (10 minutes max)
- No memory leaks detected
- Corrupted files fail gracefully with error messages

⚠️ **Performance Warnings**:
- Loading speed < 1000 points/second
- Memory efficiency < 1000 points/MB
- Memory usage > 10x file size

❌ **Failure Conditions**:
- Application crashes or hangs
- Memory leaks detected
- No error message for corrupted files

### Troubleshooting

**Test files not generated?**
- Ensure `tests/data/advanced/` directory exists
- Check disk space (large files require several GB)
- Run with administrator privileges if needed

**Tests timing out?**
- Large file tests can take 5-10 minutes
- Increase timeout with `m_executor->setTimeout(600000)` (10 minutes)
- Monitor system memory during execution

**Memory monitoring not working?**
- Linux: Requires `/proc/self/status` access
- Windows: Memory monitoring is simplified (returns 0)
- Enable with `m_executor->setMemoryMonitoringEnabled(true)`

### Integration with CI/CD

Add to your build pipeline:

```yaml
- name: Generate Advanced Test Files
  run: .\Sprint24AdvancedTests.exe --generate-test-files

- name: Run Advanced Tests  
  run: .\Sprint24AdvancedTests.exe
  timeout-minutes: 30

- name: Upload Test Reports
  uses: actions/upload-artifact@v3
  with:
    name: advanced-test-reports
    path: tests/data/advanced/*.json
```

### Customization

**Add New Test Scenarios**:
1. Extend `AdvancedTestFileGenerator::TestScenario` enum
2. Implement generation method in `AdvancedTestFileGenerator`
3. Add test case in `test_sprint2_4_advanced.cpp`

**Modify Performance Thresholds**:
```cpp
// In AdvancedTestExecutor::detectPerformanceIssues()
if (result.loadingSpeed < 500) { // Lower threshold
    emit performanceIssueDetected(result.testName, "Slow loading");
}
```

**Custom Memory Limits**:
```cpp
// In test cases
EXPECT_LT(result.memoryUsageMB, 4000) // 4GB limit
    << "Memory usage too high: " << result.memoryUsageMB << "MB";
```

## Architecture Overview

- **AdvancedTestFileGenerator**: Creates complex test scenarios
- **AdvancedTestExecutor**: Runs tests with monitoring
- **AutomatedTestOracle**: Validates results against learned patterns
- **SpectrumBasedTester**: Fault localization for failed tests
- **IntelligentBugManager**: AI-enhanced bug tracking
- **TestDataManager**: Curated test data repository

This framework provides comprehensive testing capabilities for validating point cloud loading robustness, performance, and error handling under complex real-world scenarios.
