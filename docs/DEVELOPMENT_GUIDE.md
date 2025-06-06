# CloudRegistration Development Guide

This guide provides comprehensive information for developers contributing to the CloudRegistration project.

## Getting Started

### Prerequisites

Before you begin development, ensure you have completed the [Windows Setup Guide](WINDOWS_SETUP_GUIDE.md) and have:

- Visual Studio 2022 with C++ workload
- Qt6 (6.9.0 recommended)
- vcpkg with required dependencies
- CMake 3.16+
- Git for version control

### Development Environment Setup

1. **Clone the Repository**
   ```powershell
   git clone https://github.com/bramburn/cloud_registration.git
   cd cloud_registration
   ```

2. **Configure IDE**
   
   **Visual Studio 2022:**
   - Open `CMakeLists.txt` as a CMake project
   - Configure CMake settings with vcpkg toolchain
   - Set startup project to CloudRegistration

   **VS Code:**
   - Install C++ Extension Pack and CMake Tools
   - Open project folder
   - Configure CMake kit (MSVC 2022)

3. **Build and Test**
   ```powershell
   # Build debug version for development
   .\scripts\build-clean.ps1 -BuildType Debug
   
   # Run tests to verify setup
   .\scripts\run-tests.ps1
   ```

## Project Architecture

### Core Components

1. **Application Framework**
   - `MainWindow`: Primary application window and UI coordination
   - `ProjectManager`: Project lifecycle and workspace management
   - `ProjectHubWidget`: Project selection and recent projects interface

2. **Point Cloud Processing**
   - `E57ParserLib`: E57 file reading using libE57Format
   - `E57WriterLib`: E57 file writing capabilities
   - `E57DataManager`: High-level E57 operations with progress reporting
   - `LasParser`: LAS/LAZ file parsing and validation

3. **3D Visualization**
   - `PointCloudViewerWidget`: OpenGL-based 3D rendering
   - `Octree`: Spatial indexing for level-of-detail rendering
   - `ScreenSpaceError`: LOD error calculations
   - `VoxelGridFilter`: Point cloud subsampling

4. **Data Management**
   - `SqliteManager`: Database operations for project persistence
   - `ScanImportManager`: Batch import workflows
   - `PointCloudLoadManager`: Asynchronous point cloud loading
   - `ProjectTreeModel`: Qt model for project hierarchy

### Design Patterns

1. **Model-View-Controller (MVC)**
   - Models: `ProjectTreeModel`, data structures
   - Views: Qt widgets and custom OpenGL widgets
   - Controllers: Manager classes coordinating between models and views

2. **Observer Pattern**
   - Qt signals/slots for loose coupling
   - Progress reporting through signal chains
   - Event-driven UI updates

3. **Factory Pattern**
   - Parser creation based on file extensions
   - Renderer creation for different data types

4. **Command Pattern**
   - Undo/redo operations (planned)
   - Batch processing operations

## Coding Standards

### C++ Style Guidelines

1. **Naming Conventions**
   ```cpp
   // Classes: PascalCase
   class PointCloudManager;
   
   // Methods and variables: camelCase
   void loadPointCloud();
   int pointCount;
   
   // Member variables: m_ prefix
   int m_pointCount;
   QString m_filePath;
   
   // Constants: UPPER_CASE
   static const int MAX_POINTS = 1000000;
   ```

2. **File Organization**
   ```cpp
   // Header file structure
   #ifndef CLASSNAME_H
   #define CLASSNAME_H
   
   #include <QtCore>  // Qt includes first
   #include <standard_library>  // Standard library second
   #include "local_headers.h"  // Local includes last
   
   class ClassName : public QObject {
       Q_OBJECT
   
   public:
       explicit ClassName(QObject* parent = nullptr);
       ~ClassName();
   
   public slots:
       void publicSlot();
   
   signals:
       void signalEmitted();
   
   private slots:
       void privateSlot();
   
   private:
       void privateMethod();
       
       // Member variables
       int m_memberVariable;
   };
   
   #endif // CLASSNAME_H
   ```

3. **Documentation Standards**
   ```cpp
   /**
    * @brief Brief description of the class
    * 
    * Detailed description of the class purpose and usage.
    * Include examples if the API is complex.
    * 
    * @since Version 1.0
    */
   class ExampleClass {
   public:
       /**
        * @brief Brief method description
        * @param parameter Description of parameter
        * @return Description of return value
        * @throws ExceptionType When this exception is thrown
        */
       bool exampleMethod(const QString& parameter);
   };
   ```

### Qt-Specific Guidelines

1. **Memory Management**
   ```cpp
   // Use Qt parent-child relationships
   auto* widget = new QWidget(parent);  // Parent will delete
   
   // Use smart pointers for non-Qt objects
   std::unique_ptr<DataProcessor> processor;
   
   // Use QPointer for Qt object references
   QPointer<QWidget> weakReference;
   ```

2. **Signal-Slot Connections**
   ```cpp
   // Prefer new syntax with compile-time checking
   connect(sender, &SenderClass::signalName, 
           receiver, &ReceiverClass::slotName);
   
   // Use lambda for simple operations
   connect(button, &QPushButton::clicked, [this]() {
       processData();
   });
   ```

3. **Threading**
   ```cpp
   // Use QThread with moveToThread pattern
   auto* worker = new WorkerClass;
   auto* thread = new QThread(this);
   worker->moveToThread(thread);
   
   connect(thread, &QThread::started, worker, &WorkerClass::process);
   connect(worker, &WorkerClass::finished, thread, &QThread::quit);
   thread->start();
   ```

## Testing Framework

### Unit Testing

1. **Test Structure**
   ```cpp
   #include <gtest/gtest.h>
   #include <QTest>
   #include "ClassUnderTest.h"
   
   class ClassUnderTestTest : public ::testing::Test {
   protected:
       void SetUp() override {
           // Setup test fixtures
           testObject = std::make_unique<ClassUnderTest>();
       }
       
       void TearDown() override {
           // Cleanup
           testObject.reset();
       }
       
       std::unique_ptr<ClassUnderTest> testObject;
   };
   
   TEST_F(ClassUnderTestTest, TestMethodName) {
       // Arrange
       QString input = "test data";
       
       // Act
       bool result = testObject->methodUnderTest(input);
       
       // Assert
       EXPECT_TRUE(result);
       EXPECT_EQ(testObject->getProcessedCount(), 1);
   }
   ```

2. **Test Categories**
   - **Unit Tests**: Test individual classes and methods
   - **Integration Tests**: Test component interactions
   - **Performance Tests**: Benchmark critical operations
   - **UI Tests**: Test user interface components

3. **Running Tests**
   ```powershell
   # Run all tests
   .\scripts\run-tests.ps1
   
   # Run specific test suite
   .\build-debug\bin\Debug\E57ParserTests.exe
   
   # Run with Google Test filters
   .\build-debug\bin\Debug\E57ParserTests.exe --gtest_filter="*Performance*"
   ```

### Test Data Management

1. **Sample Files**
   - Store test files in `test_data/` directory
   - Use descriptive filenames: `test_triangle.e57`, `malformed_header.las`
   - Include both valid and invalid test cases

2. **Test File Generation**
   ```cpp
   // Create test files programmatically
   void createTestE57File(const QString& filename) {
       E57WriterLib writer;
       writer.createFile(filename);
       writer.addScan("Test Scan");
       writer.defineXYZPrototype();
       
       std::vector<E57WriterLib::Point3D> points = {
           {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}
       };
       writer.writePoints(points);
       writer.closeFile();
   }
   ```

## Performance Considerations

### Optimization Guidelines

1. **Memory Management**
   - Use object pools for frequently created/destroyed objects
   - Implement lazy loading for large datasets
   - Monitor memory usage with profiling tools

2. **Rendering Optimization**
   - Implement level-of-detail (LOD) for large point clouds
   - Use frustum culling to avoid rendering invisible points
   - Batch OpenGL operations to reduce state changes

3. **File I/O Optimization**
   - Use memory-mapped files for large datasets
   - Implement chunked reading for progress reporting
   - Cache frequently accessed data

### Profiling and Benchmarking

1. **Built-in Profiler**
   ```cpp
   #include "performance_profiler.h"
   
   void expensiveOperation() {
       PROFILE_FUNCTION();  // Automatic timing
       
       // Your code here
   }
   ```

2. **Benchmark Tests**
   ```cpp
   TEST(PerformanceTest, E57ParsingBenchmark) {
       PerformanceProfiler profiler;
       profiler.startTiming("E57Parsing");
       
       // Parse large E57 file
       E57ParserLib parser;
       auto result = parser.openFile("large_dataset.e57");
       
       profiler.endTiming("E57Parsing");
       
       // Assert performance requirements
       EXPECT_LT(profiler.getElapsedTime("E57Parsing"), 5000); // 5 seconds max
   }
   ```

## Debugging Techniques

### Common Debugging Scenarios

1. **E57 Parsing Issues**
   ```cpp
   // Enable detailed E57 logging
   qDebug() << "E57Parser: Opening file" << filePath;
   qDebug() << "E57Parser: Found" << scanCount << "scans";
   
   // Use try-catch for E57 exceptions
   try {
       e57::ImageFile imageFile(filePath.toStdString(), "r");
       // ... parsing code
   } catch (const e57::E57Exception& ex) {
       qDebug() << "E57 Exception:" << ex.what();
   }
   ```

2. **OpenGL Rendering Issues**
   ```cpp
   // Check OpenGL errors
   void checkGLError(const QString& operation) {
       GLenum error = glGetError();
       if (error != GL_NO_ERROR) {
           qDebug() << "OpenGL error in" << operation << ":" << error;
       }
   }
   
   // Use in rendering code
   glDrawArrays(GL_POINTS, 0, pointCount);
   checkGLError("glDrawArrays");
   ```

3. **Memory Leaks**
   ```cpp
   // Use Qt's debug output for object tracking
   #ifdef QT_DEBUG
   qDebug() << "Creating" << metaObject()->className();
   #endif
   
   // In destructor
   #ifdef QT_DEBUG
   qDebug() << "Destroying" << metaObject()->className();
   #endif
   ```

### Debugging Tools

1. **Visual Studio Debugger**
   - Set breakpoints in critical code paths
   - Use conditional breakpoints for specific conditions
   - Inspect Qt objects with Qt visualizers

2. **Qt Creator Debugger**
   - Excellent Qt object inspection
   - QML debugging capabilities
   - Memory usage analysis

3. **External Tools**
   - **Application Verifier**: Windows heap corruption detection
   - **PerfView**: .NET and native performance analysis
   - **Intel VTune**: CPU profiling and optimization

## Contributing Workflow

### Branch Strategy

1. **Main Branches**
   - `main`: Stable release branch
   - `develop`: Integration branch for features

2. **Feature Branches**
   - `feature/description`: New features
   - `fix/description`: Bug fixes
   - `refactor/description`: Code improvements

### Pull Request Process

1. **Before Creating PR**
   ```powershell
   # Ensure tests pass
   .\scripts\run-tests.ps1
   
   # Check code formatting
   # (Add formatting tools as needed)
   
   # Update documentation if needed
   ```

2. **PR Requirements**
   - Clear description of changes
   - Reference to related issues
   - Test coverage for new features
   - Documentation updates for user-facing changes

3. **Review Process**
   - Code review by maintainers
   - Automated testing via CI/CD
   - Performance impact assessment
   - Documentation review

## Release Process

### Version Management

1. **Semantic Versioning**
   - MAJOR.MINOR.PATCH format
   - MAJOR: Breaking changes
   - MINOR: New features, backward compatible
   - PATCH: Bug fixes

2. **Release Preparation**
   ```powershell
   # Update version numbers
   # Update CHANGELOG.md
   # Create release branch
   git checkout -b release/v1.2.0
   
   # Final testing
   .\scripts\run-tests.ps1 -Coverage
   
   # Build release packages
   .\scripts\build-clean.ps1 -BuildType Release
   ```

### Documentation Updates

1. **User Documentation**
   - Update README.md for new features
   - Add usage examples
   - Update system requirements

2. **Developer Documentation**
   - Update API documentation
   - Add architecture diagrams
   - Document breaking changes

## Support and Resources

### Internal Resources

- **Code Documentation**: Generated from source comments
- **Architecture Diagrams**: In `docs/` directory
- **Test Data**: Sample files in `sample/` and `test_data/`

### External Resources

- **Qt Documentation**: [doc.qt.io](https://doc.qt.io/)
- **libE57Format**: [E57 Format Documentation](https://www.libe57.org/)
- **CMake Documentation**: [cmake.org](https://cmake.org/documentation/)
- **vcpkg**: [vcpkg.io](https://vcpkg.io/)

### Getting Help

1. **GitHub Issues**: For bugs and feature requests
2. **Discussions**: For questions and ideas
3. **Code Review**: For implementation guidance
4. **Documentation**: For usage and setup questions

---

This development guide is a living document. Please update it as the project evolves and new practices are established.
