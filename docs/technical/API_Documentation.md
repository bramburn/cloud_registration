# Cloud Registration API Documentation

This document provides technical API documentation for developers working with Cloud Registration components.

## Core Interfaces

### IE57Parser Interface

```cpp
class IE57Parser {
public:
    virtual ~IE57Parser() = default;
    virtual bool openFile(const std::string& filePath) = 0;
    virtual std::vector<PointCloudPoint> getPoints() = 0;
    virtual E57Metadata getMetadata() = 0;
    virtual bool isValid() const = 0;
};
```

**Usage Example:**
```cpp
#include "e57parserlib.h"

E57ParserLib parser;
if (parser.openFile("scan.e57")) {
    auto points = parser.getPoints();
    auto metadata = parser.getMetadata();
    // Process point cloud data
}
```

### IMainView Interface

```cpp
class IMainView {
public:
    virtual ~IMainView() = default;
    virtual void showStatusMessage(const QString& message) = 0;
    virtual void updateProgressBar(int value, const QString& text) = 0;
    virtual IPointCloudViewer* getPointCloudViewer() = 0;
    virtual void displayError(const QString& error) = 0;
};
```

### IPointCloudViewer Interface

```cpp
class IPointCloudViewer {
public:
    virtual ~IPointCloudViewer() = default;
    virtual bool loadPointCloud(const std::vector<PointCloudPoint>& points) = 0;
    virtual void clearPointCloud() = 0;
    virtual void setTransformation(const QMatrix4x4& transform) = 0;
    virtual void updateCamera(const CameraState& state) = 0;
};
```

## Data Structures

### PointCloudPoint

```cpp
struct PointCloudPoint {
    float x, y, z;           // Coordinates
    uint8_t r, g, b;         // Color (optional)
    float intensity;         // Intensity value (optional)
    float normal_x, normal_y, normal_z;  // Surface normals (optional)
};
```

### E57Metadata

```cpp
struct E57Metadata {
    QString scannerModel;
    QString acquisitionDate;
    QVector3D scannerPosition;
    double pointCount;
    BoundingBox bounds;
    CoordinateSystem coordinateSystem;
};
```

## Registration API

### Manual Registration

```cpp
// Create correspondences between scans
std::vector<CorrespondencePair> correspondences;
correspondences.push_back({point1_scan1, point1_scan2});
correspondences.push_back({point2_scan1, point2_scan2});
correspondences.push_back({point3_scan1, point3_scan2});

// Execute manual alignment
RegistrationResult result = registrationEngine.performManualAlignment(correspondences);
if (result.success) {
    QMatrix4x4 transform = result.transformation;
    float rmsError = result.rmsError;
}
```

### ICP Registration

```cpp
// Configure ICP parameters
ICPParameters params;
params.maxIterations = 100;
params.convergenceThreshold = 0.001f;
params.algorithm = ICPAlgorithm::PointToPlane;

// Execute ICP refinement
RegistrationResult result = registrationEngine.performICP(sourcePoints, targetPoints, params);
```

## Project Management API

### Creating Projects

```cpp
ProjectManager manager;
Project project;
project.setName("My Registration Project");
project.setPath("/path/to/project");
project.setDescription("Point cloud registration project");

if (manager.createProject(project)) {
    // Project created successfully
}
```

### Loading Point Clouds

```cpp
PointCloudLoadManager loadManager;
LoadingSettings settings;
settings.coordinateSystem = "UTM Zone 10N";
settings.pointDensity = 1.0f; // Full resolution

loadManager.loadPointCloud("scan.e57", settings);
```

## Error Handling

### Exception Types

```cpp
class E57ParseException : public std::exception {
public:
    const char* what() const noexcept override;
    E57ErrorCode getErrorCode() const;
};

class RegistrationException : public std::exception {
public:
    const char* what() const noexcept override;
    RegistrationErrorCode getErrorCode() const;
};
```

### Error Codes

```cpp
enum class E57ErrorCode {
    FileNotFound,
    InvalidFormat,
    CorruptedData,
    UnsupportedVersion,
    InsufficientMemory
};

enum class RegistrationErrorCode {
    InsufficientCorrespondences,
    PoorAlignment,
    ConvergenceFailure,
    InvalidParameters
};
```

## Performance Optimization

### Memory Management

```cpp
// Configure memory limits
PerformanceSettings settings;
settings.maxMemoryUsage = 8 * 1024 * 1024 * 1024; // 8GB
settings.enableStreaming = true;
settings.cacheSize = 1024 * 1024 * 1024; // 1GB

PerformanceProfiler::configure(settings);
```

### GPU Acceleration

```cpp
// Enable GPU acceleration
RenderingSettings renderSettings;
renderSettings.enableGPU = true;
renderSettings.maxGPUMemory = 4 * 1024 * 1024 * 1024; // 4GB
renderSettings.preferredRenderer = RendererType::OpenGL;

pointCloudViewer->configureRendering(renderSettings);
```

## Integration Examples

### Custom File Format Support

```cpp
class CustomParser : public IE57Parser {
public:
    bool openFile(const std::string& filePath) override {
        // Implement custom file parsing
        return true;
    }
    
    std::vector<PointCloudPoint> getPoints() override {
        // Return parsed points
        return points_;
    }
    
    E57Metadata getMetadata() override {
        // Return file metadata
        return metadata_;
    }
    
    bool isValid() const override {
        return isValid_;
    }
    
private:
    std::vector<PointCloudPoint> points_;
    E57Metadata metadata_;
    bool isValid_ = false;
};
```

### Custom Registration Algorithm

```cpp
class CustomRegistrationAlgorithm {
public:
    RegistrationResult register(
        const std::vector<PointCloudPoint>& source,
        const std::vector<PointCloudPoint>& target,
        const RegistrationParameters& params) {
        
        // Implement custom registration logic
        RegistrationResult result;
        result.transformation = computeTransformation(source, target);
        result.rmsError = computeRMSError(source, target, result.transformation);
        result.success = result.rmsError < params.maxRMSError;
        
        return result;
    }
};
```

## Testing and Validation

### Unit Testing

```cpp
#include <gtest/gtest.h>
#include "e57parserlib.h"

TEST(E57ParserTest, LoadValidFile) {
    E57ParserLib parser;
    ASSERT_TRUE(parser.openFile("test_data/valid_scan.e57"));
    
    auto points = parser.getPoints();
    ASSERT_GT(points.size(), 0);
    
    auto metadata = parser.getMetadata();
    ASSERT_FALSE(metadata.scannerModel.isEmpty());
}
```

### Integration Testing

```cpp
TEST(RegistrationTest, FullWorkflow) {
    // Load two scans
    E57ParserLib parser1, parser2;
    ASSERT_TRUE(parser1.openFile("scan1.e57"));
    ASSERT_TRUE(parser2.openFile("scan2.e57"));
    
    // Create correspondences
    std::vector<CorrespondencePair> correspondences = createTestCorrespondences();
    
    // Perform registration
    RegistrationEngine engine;
    auto result = engine.performManualAlignment(correspondences);
    
    ASSERT_TRUE(result.success);
    ASSERT_LT(result.rmsError, 0.005f); // 5mm threshold
}
```

This API documentation provides the foundation for extending and integrating with Cloud Registration components.
