# Sprint 3 Testing Guide

## Overview

This guide provides comprehensive testing instructions for Sprint 3's target detection and selection systems. All components have been implemented with full unit test coverage and integration examples.

## Quick Start Testing

### 1. Build the Project

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build all components
make -j$(nproc)

# Build tests specifically
make TargetTests TargetManagerTests SphereDetectorTests NaturalPointSelectorTests
```

### 2. Run Unit Tests

```bash
# Run all Sprint 3 tests
ctest -R "Target|Sphere|Natural" --verbose

# Or run individual test suites
./TargetTests
./TargetManagerTests  
./SphereDetectorTests
./NaturalPointSelectorTests
```

### 3. Run Integration Example

```bash
# Build the integration example
cd examples
qmake sprint3_integration_example.pro  # If using qmake
# OR add to CMakeLists.txt and build with cmake

./sprint3_integration_example
```

## Component Testing

### Target System Testing

**Test File**: `tests/test_target.cpp`

**Key Test Cases**:
- Target creation and basic properties
- Serialization/deserialization of all target types
- Target validation and quality metrics
- Correspondence creation and validation

**Manual Testing**:
```cpp
// Create test targets
auto sphere = std::make_shared<SphereTarget>("test_sphere", QVector3D(1,2,3), 0.15f);
auto natural = std::make_shared<NaturalPointTarget>("test_natural", QVector3D(4,5,6), "Corner");

// Test serialization
QVariantMap data = sphere->serialize();
SphereTarget newSphere("", QVector3D(), 0.0f);
bool success = newSphere.deserialize(data);
// Verify: success == true and properties match
```

### Target Manager Testing

**Test File**: `tests/test_targetmanager.cpp`

**Key Test Cases**:
- Adding/removing targets from scans
- Correspondence management
- Statistics calculation
- File save/load operations
- Potential correspondence finding

**Manual Testing**:
```cpp
TargetManager manager;

// Add targets to different scans
manager.addTarget("scan1", sphereTarget1);
manager.addTarget("scan2", sphereTarget2);

// Create correspondence
TargetCorrespondence corr("sphere1", "sphere2", "scan1", "scan2");
manager.addCorrespondence(corr);

// Test statistics
auto stats = manager.getStatistics();
// Verify: stats.totalTargets == 2, stats.correspondences == 1
```

### Sphere Detection Testing

**Test File**: `tests/test_spheredetector.cpp`

**Key Test Cases**:
- Perfect sphere detection
- Noisy data handling
- Multiple sphere detection
- Parameter validation
- Size filtering

**Manual Testing with Synthetic Data**:
```cpp
SphereDetector detector;

// Generate synthetic sphere points
std::vector<PointFullData> points = generateSpherePoints(
    QVector3D(0,0,0), 0.15f, 200);

// Configure parameters
TargetDetectionBase::DetectionParams params;
params.minRadius = 0.1f;
params.maxRadius = 0.2f;
params.minInliers = 50;

// Run detection
auto result = detector.detect(points, params);

// Verify: result.success == true, result.targets.size() >= 1
// Check detected sphere parameters match input
```

### Natural Point Selection Testing

**Test File**: `tests/test_naturalpointselector.cpp`

**Key Test Cases**:
- Screen coordinate to 3D point selection
- Closest point selection
- Feature analysis and confidence calculation
- Correspondence suggestion

**Manual Testing**:
```cpp
NaturalPointSelector selector;

// Create test point cloud with features
std::vector<PointFullData> points = generateTestPointCloud();

// Test closest point selection
QVector3D targetPos(1.0f, 1.0f, 0.5f);
auto result = selector.selectClosestPoint(points, targetPos, 0.1f);

// Verify: result.success == true, result.confidence > 0
// Check selected point is within tolerance of target
```

## Integration Testing

### Target Detection Dialog Testing

**Manual UI Testing Steps**:

1. **Launch Dialog**:
   ```cpp
   TargetDetectionDialog dialog(&targetManager);
   dialog.setPointCloudData("test_scan", testPoints);
   dialog.show();
   ```

2. **Test Parameter Configuration**:
   - Switch between detection modes
   - Adjust sphere detection parameters
   - Validate parameter ranges
   - Save/load parameter presets

3. **Test Automatic Detection**:
   - Load point cloud with known spheres
   - Run detection with various parameters
   - Verify results table shows detected targets
   - Accept/reject detection results

4. **Test Manual Selection Mode**:
   - Enable manual selection
   - Verify UI state changes appropriately
   - Test parameter validation

### Full Integration Testing

**Test File**: `examples/sprint3_integration_example.cpp`

**Testing Steps**:

1. **Launch Example Application**:
   ```bash
   ./sprint3_integration_example
   ```

2. **Test Automatic Detection**:
   - Click "Run Automatic Detection"
   - Verify progress bar and status updates
   - Check targets appear in results table
   - Verify statistics update correctly

3. **Test Detection Dialog**:
   - Click "Open Detection Dialog"
   - Configure parameters
   - Run detection
   - Accept results

4. **Test Manual Selection**:
   - Click "Enable Manual Selection"
   - Verify status message changes
   - Simulate mouse clicks (in real app)

5. **Test File Operations**:
   - Save targets to file
   - Clear targets
   - Load targets from file
   - Verify data persistence

## Performance Testing

### Sphere Detection Performance

**Test with Various Point Counts**:
```cpp
// Test performance with different point cloud sizes
std::vector<int> pointCounts = {1000, 10000, 100000, 1000000};

for (int count : pointCounts) {
    auto points = generateRandomPoints(count);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = detector.detect(points, params);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug() << "Points:" << count << "Time:" << duration.count() << "ms";
}
```

**Expected Performance**:
- 1K points: < 10ms
- 10K points: < 100ms  
- 100K points: < 1s
- 1M points: < 10s

### Memory Usage Testing

**Monitor Memory During Detection**:
```cpp
// Check memory usage before/after detection
size_t memBefore = getCurrentMemoryUsage();
auto result = detector.detect(largePointCloud, params);
size_t memAfter = getCurrentMemoryUsage();

qDebug() << "Memory used:" << (memAfter - memBefore) / 1024 / 1024 << "MB";
```

## Real Data Testing

### Using LAS Test File

The project includes a test LAS file at `sample/S2max-Power line202503.las`:

```cpp
// Load real LAS data
LasParser parser;
auto points = parser.parseFile("sample/S2max-Power line202503.las");

// Run detection on real data
SphereDetector detector;
TargetDetectionBase::DetectionParams params;
params.enablePreprocessing = true;  // Important for real data
params.distanceThreshold = 0.02f;   // Larger tolerance for real data

auto result = detector.detect(points, params);
qDebug() << "Found" << result.targets.size() << "targets in real data";
```

### Testing with Different Data Types

1. **Clean Synthetic Data**: Perfect spheres, no noise
2. **Noisy Synthetic Data**: Spheres with Gaussian noise
3. **Real LAS Data**: Actual laser scan data
4. **Mixed Data**: Combination of targets and background

## Debugging and Troubleshooting

### Enable Debug Logging

```cpp
// Enable detailed logging
QLoggingCategory::setFilterRules("*.debug=true");

// Or specific categories
QLoggingCategory::setFilterRules("SphereDetector.debug=true");
```

### Common Test Failures

1. **Sphere Detection Fails**:
   - Check point cloud has actual sphere-like structures
   - Verify radius parameters match data
   - Increase distance threshold for noisy data

2. **Manual Selection Fails**:
   - Verify view/projection matrices are valid
   - Check viewport size matches test setup
   - Ensure point cloud has sufficient density

3. **Serialization Fails**:
   - Check all required fields are present
   - Verify data types match expected formats
   - Test with minimal valid data first

### Test Data Generation

**Generate Test Spheres**:
```cpp
std::vector<PointFullData> generateSphere(QVector3D center, float radius, int points) {
    std::vector<PointFullData> result;
    for (int i = 0; i < points; ++i) {
        // Generate random point on sphere surface
        float theta = 2.0f * M_PI * rand() / RAND_MAX;
        float phi = acos(1.0f - 2.0f * rand() / RAND_MAX);
        
        PointFullData point;
        point.x = center.x() + radius * sin(phi) * cos(theta);
        point.y = center.y() + radius * sin(phi) * sin(theta);
        point.z = center.z() + radius * cos(phi);
        point.intensity = 100.0f;
        point.hasIntensity = true;
        
        result.push_back(point);
    }
    return result;
}
```

## Continuous Integration Testing

### Automated Test Pipeline

```yaml
# Example CI configuration
test_sprint3:
  script:
    - mkdir build && cd build
    - cmake .. -DCMAKE_BUILD_TYPE=Debug
    - make -j$(nproc)
    - ctest -R "Target|Sphere|Natural" --output-on-failure
  artifacts:
    reports:
      junit: build/test_results.xml
```

### Test Coverage

Run with coverage analysis:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
make
ctest
gcov src/registration/*.cpp src/detection/*.cpp
```

**Expected Coverage**:
- Target classes: > 95%
- Detection algorithms: > 90%
- UI components: > 80%

## Conclusion

Sprint 3 includes comprehensive testing at all levels:
- **Unit Tests**: Individual component validation
- **Integration Tests**: Component interaction testing  
- **Performance Tests**: Scalability and efficiency
- **Real Data Tests**: Practical validation

All tests are designed to be run automatically in CI/CD pipelines while also supporting manual testing and debugging during development.
