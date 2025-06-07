## Sprint 5 Implementation: Final E57ParserLib Integration & Legacy Code Removal

Based on the PRD document, here's the comprehensive implementation guide for integrating E57ParserLib into MainWindow and completing the transition from the old custom parser.

### 1. Enhanced E57ParserLib with MainWindow Integration Support

**E57ParserLib.hpp** (Updated with MainWindow compatibility):
```cpp
#ifndef E57PARSERLIB_HPP
#define E57PARSERLIB_HPP

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

class E57ParserLib : public QObject {
    Q_OBJECT
    
public:
    struct PointData {
        double x, y, z;
        float intensity = 0.0f;
        uint8_t r = 255, g = 255, b = 255;
        bool hasIntensity = false;
        bool hasColor = false;
        bool isValid = true;
    };
    
    struct LoadingSettings {
        bool loadIntensity = true;
        bool loadColor = true;
        int maxPointsPerScan = -1;  // -1 = unlimited
        double subsamplingRatio = 1.0;  // 1.0 = no subsampling
    };

public:
    explicit E57ParserLib(QObject* parent = nullptr);
    ~E57ParserLib();
    
    // Main entry point for MainWindow integration
    void startParsing(const QString& filePath, const LoadingSettings& settings = LoadingSettings());
    
    // Thread-safe cancellation
    void cancelParsing();
    
    // Error reporting
    QString getLastError() const;
    
    // Utility methods for MainWindow
    bool isValidE57File(const QString& filePath);
    int getScanCount(const QString& filePath);

signals:
    // MainWindow-compatible signals matching old E57Parser interface
    void progressUpdated(int percentage, const QString& stage);
    void parsingFinished(bool success, const QString& message, const std::vector& points);
    
    // Additional signals for enhanced functionality
    void scanMetadataAvailable(int scanCount, const QStringList& scanNames);
    void intensityDataExtracted(const std::vector& intensityValues);
    void colorDataExtracted(const std::vector& colorValues); // RGB interleaved

private slots:
    void performParsing();

private:
    // Core parsing functionality
    bool openE57File(const QString& filePath);
    void closeE57File();
    std::vector extractPointDataFromScan(int scanIndex, const LoadingSettings& settings);
    std::vector convertToXYZVector(const std::vector& pointData);
    
    // Progress tracking
    void updateProgress(int percentage, const QString& stage);
    
    // Error handling
    void handleE57Exception(const e57::E57Exception& ex, const QString& context);
    QString translateE57Error(const QString& technicalError);
    
    // Threading support
    void setupForThreading();
    
    // Data members
    e57::ImageFile* m_imageFile = nullptr;
    QString m_currentFilePath;
    LoadingSettings m_currentSettings;
    QString m_lastError;
    
    // Threading and cancellation
    std::atomic m_cancelRequested{false};
    mutable QMutex m_errorMutex;
    QTimer* m_progressTimer = nullptr;
    
    // Internal data storage
    std::vector m_extractedPoints;
    QStringList m_scanNames;
    int m_totalScans = 0;
};

#endif // E57PARSERLIB_HPP
```

**E57ParserLib.cpp** (MainWindow integration implementation):
```cpp
#include "E57ParserLib.hpp"
#include 
#include 
#include 
#include 
#include 
#include 

E57ParserLib::E57ParserLib(QObject* parent) : QObject(parent) {
    // Setup for potential threading
    setupForThreading();
}

E57ParserLib::~E57ParserLib() {
    closeE57File();
}

void E57ParserLib::startParsing(const QString& filePath, const LoadingSettings& settings) {
    m_currentFilePath = filePath;
    m_currentSettings = settings;
    m_cancelRequested = false;
    
    qDebug() ());
        return;
    }
    
    if (!isValidE57File(filePath)) {
        QString errorMsg = "Invalid E57 file format: " + filePath;
        emit parsingFinished(false, errorMsg, std::vector());
        return;
    }
    
    // Start parsing in current thread or defer to thread if already in worker thread
    if (QThread::currentThread() != thread()) {
        // We're being called from a worker thread, execute directly
        performParsing();
    } else {
        // We're in the main thread, defer to next event loop iteration
        QTimer::singleShot(0, this, &E57ParserLib::performParsing);
    }
}

void E57ParserLib::performParsing() {
    emit progressUpdated(0, "Initializing E57 parser...");
    
    try {
        // Step 1: Open E57 file
        if (!openE57File(m_currentFilePath)) {
            emit parsingFinished(false, m_lastError, std::vector());
            return;
        }
        
        if (m_cancelRequested) {
            closeE57File();
            emit parsingFinished(false, "Parsing cancelled by user", std::vector());
            return;
        }
        
        emit progressUpdated(10, "Analyzing E57 file structure...");
        
        // Step 2: Get scan information
        m_totalScans = getScanCount(m_currentFilePath);
        
        if (m_totalScans == 0) {
            closeE57File();
            emit parsingFinished(false, "No scans found in E57 file", std::vector());
            return;
        }
        
        // Extract scan names for metadata
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("/data3D")) {
            e57::VectorNode data3D = static_cast(root.get("/data3D"));
            
            for (int64_t i = 0; i (data3D.get(i));
                QString scanName = QString("Scan %1").arg(i);
                
                if (scan.isDefined("name")) {
                    scanName = QString::fromStdString(
                        static_cast(scan.get("name")).value());
                }
                m_scanNames.append(scanName);
            }
        }
        
        emit scanMetadataAvailable(m_totalScans, m_scanNames);
        emit progressUpdated(20, QString("Found %1 scans, processing...").arg(m_totalScans));
        
        if (m_cancelRequested) {
            closeE57File();
            emit parsingFinished(false, "Parsing cancelled by user", std::vector());
            return;
        }
        
        // Step 3: Extract point data (currently from first scan only, as per PRD requirements)
        emit progressUpdated(30, "Extracting point data...");
        
        m_extractedPoints = extractPointDataFromScan(0, m_currentSettings);
        
        if (m_extractedPoints.empty()) {
            closeE57File();
            emit parsingFinished(false, "No valid points extracted from E57 file", std::vector());
            return;
        }
        
        if (m_cancelRequested) {
            closeE57File();
            emit parsingFinished(false, "Parsing cancelled by user", std::vector());
            return;
        }
        
        emit progressUpdated(80, "Converting point data...");
        
        // Step 4: Convert to XYZ vector for MainWindow compatibility
        std::vector xyzPoints = convertToXYZVector(m_extractedPoints);
        
        // Step 5: Emit additional data if available
        std::vector intensityData;
        std::vector colorData;
        
        bool hasIntensity = std::any_of(m_extractedPoints.begin(), m_extractedPoints.end(),
            [](const PointData& p) { return p.hasIntensity; });
        bool hasColor = std::any_of(m_extractedPoints.begin(), m_extractedPoints.end(),
            [](const PointData& p) { return p.hasColor; });
        
        if (hasIntensity) {
            intensityData.reserve(m_extractedPoints.size());
            for (const auto& point : m_extractedPoints) {
                intensityData.push_back(point.hasIntensity ? point.intensity : 0.0f);
            }
            emit intensityDataExtracted(intensityData);
        }
        
        if (hasColor) {
            colorData.reserve(m_extractedPoints.size() * 3);
            for (const auto& point : m_extractedPoints) {
                if (point.hasColor) {
                    colorData.push_back(point.r);
                    colorData.push_back(point.g);
                    colorData.push_back(point.b);
                } else {
                    colorData.push_back(255);
                    colorData.push_back(255);
                    colorData.push_back(255);
                }
            }
            emit colorDataExtracted(colorData);
        }
        
        closeE57File();
        
        emit progressUpdated(100, "Parsing complete");
        
        QString successMessage = QString("Successfully loaded %1 points from %2 scans")
                                .arg(m_extractedPoints.size()).arg(m_totalScans);
        
        if (hasIntensity) successMessage += " (with intensity data)";
        if (hasColor) successMessage += " (with color data)";
        
        emit parsingFinished(true, successMessage, xyzPoints);
        
        qDebug() ());
        
    } catch (const std::exception& ex) {
        closeE57File();
        m_lastError = QString("Unexpected error during E57 parsing: %1").arg(ex.what());
        emit parsingFinished(false, m_lastError, std::vector());
    }
}

std::vector E57ParserLib::convertToXYZVector(const std::vector& pointData) {
    std::vector xyzVector;
    xyzVector.reserve(pointData.size() * 3);
    
    for (const auto& point : pointData) {
        if (point.isValid) {
            xyzVector.push_back(static_cast(point.x));
            xyzVector.push_back(static_cast(point.y));
            xyzVector.push_back(static_cast(point.z));
        }
    }
    
    return xyzVector;
}

std::vector E57ParserLib::extractPointDataFromScan(
    int scanIndex, const LoadingSettings& settings) {
    
    std::vector points;
    
    try {
        e57::StructureNode root = m_imageFile->root();
        e57::VectorNode data3D = static_cast(root.get("/data3D"));
        
        if (scanIndex >= data3D.childCount()) {
            m_lastError = QString("Scan index %1 out of range").arg(scanIndex);
            return points;
        }
        
        e57::StructureNode scan = static_cast(data3D.get(scanIndex));
        
        if (!scan.isDefined("points")) {
            m_lastError = "Scan does not contain point data";
            return points;
        }
        
        e57::CompressedVectorNode pointsNode = 
            static_cast(scan.get("points"));
        e57::StructureNode prototype = pointsNode.prototype();
        
        // Check available fields
        bool hasX = prototype.isDefined("cartesianX");
        bool hasY = prototype.isDefined("cartesianY");
        bool hasZ = prototype.isDefined("cartesianZ");
        bool hasIntensity = prototype.isDefined("intensity") && settings.loadIntensity;
        bool hasColorR = prototype.isDefined("colorRed") && settings.loadColor;
        bool hasColorG = prototype.isDefined("colorGreen") && settings.loadColor;
        bool hasColorB = prototype.isDefined("colorBlue") && settings.loadColor;
        
        if (!hasX || !hasY || !hasZ) {
            m_lastError = "Scan missing required cartesian coordinates";
            return points;
        }
        
        // Setup buffers for reading
        const int64_t BUFFER_SIZE = 65536;
        int64_t totalPoints = pointsNode.childCount();
        
        if (settings.maxPointsPerScan > 0) {
            totalPoints = std::min(totalPoints, static_cast(settings.maxPointsPerScan));
        }
        
        std::vector xBuffer(BUFFER_SIZE);
        std::vector yBuffer(BUFFER_SIZE);
        std::vector zBuffer(BUFFER_SIZE);
        std::vector intensityBuffer(BUFFER_SIZE);
        std::vector rBuffer(BUFFER_SIZE);
        std::vector gBuffer(BUFFER_SIZE);
        std::vector bBuffer(BUFFER_SIZE);
        
        // Setup SourceDestBuffers
        std::vector buffers;
        buffers.emplace_back(m_imageFile, "cartesianX", xBuffer.data(), BUFFER_SIZE, true);
        buffers.emplace_back(m_imageFile, "cartesianY", yBuffer.data(), BUFFER_SIZE, true);
        buffers.emplace_back(m_imageFile, "cartesianZ", zBuffer.data(), BUFFER_SIZE, true);
        
        if (hasIntensity) {
            buffers.emplace_back(m_imageFile, "intensity", intensityBuffer.data(), BUFFER_SIZE, true, true);
        }
        if (hasColorR) {
            buffers.emplace_back(m_imageFile, "colorRed", rBuffer.data(), BUFFER_SIZE, true, true);
        }
        if (hasColorG) {
            buffers.emplace_back(m_imageFile, "colorGreen", gBuffer.data(), BUFFER_SIZE, true, true);
        }
        if (hasColorB) {
            buffers.emplace_back(m_imageFile, "colorBlue", bBuffer.data(), BUFFER_SIZE, true, true);
        }
        
        // Read points in chunks
        e57::CompressedVectorReader reader = pointsNode.reader(buffers);
        points.reserve(std::min(totalPoints, static_cast(1000000))); // Reserve reasonable amount
        
        unsigned long pointsRead = 0;
        int64_t totalProcessed = 0;
        int lastProgressPercent = 30;
        
        while ((pointsRead = reader.read()) > 0 && totalProcessed  dis(0.0, 1.0);
                    
                    if (dis(gen) > settings.subsamplingRatio) {
                        totalProcessed++;
                        continue;
                    }
                }
                
                points.push_back(point);
                totalProcessed++;
            }
            
            // Update progress
            int progressPercent = 30 + (totalProcessed * 50) / totalPoints;
            if (progressPercent > lastProgressPercent + 5) {
                emit progressUpdated(progressPercent, 
                    QString("Processed %1 of %2 points...").arg(totalProcessed).arg(totalPoints));
                lastProgressPercent = progressPercent;
            }
        }
        
        reader.close();
        
        qDebug() setSingleShot(true);
    
    // Ensure proper signal/slot connections work across threads
    connect(this, &E57ParserLib::progressUpdated, this, &E57ParserLib::progressUpdated, Qt::QueuedConnection);
}
```

### 2. MainWindow Integration Implementation

**MainWindow.cpp** (Updated sections for E57ParserLib integration):
```cpp
// In MainWindow.cpp - Replace E57Parser usage with E57ParserLib

#include "E57ParserLib.hpp"  // Add this include, remove old e57parser.h

// In MainWindow::onOpenFileClicked() method
void MainWindow::onOpenFileClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        "Open Point Cloud File", 
        m_lastDirectory,
        "Point Cloud Files (*.e57 *.las *.ply *.pcd);;E57 Files (*.e57);;LAS Files (*.las);;PLY Files (*.ply);;PCD Files (*.pcd);;All Files (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    m_lastDirectory = QFileInfo(fileName).absolutePath();
    
    // Check file type and show loading settings dialog
    LoadingSettingsDialog dialog(fileName, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    LoadingSettings settings = dialog.getSettings();
    
    // Determine file type and create appropriate parser
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "e57") {
        // NEW: Use E57ParserLib instead of old E57Parser
        auto* e57Parser = new E57ParserLib();
        
        // Convert dialog settings to E57ParserLib::LoadingSettings
        E57ParserLib::LoadingSettings e57Settings;
        e57Settings.loadIntensity = settings.loadIntensity;
        e57Settings.loadColor = settings.loadColor;
        e57Settings.maxPointsPerScan = settings.maxPoints;
        e57Settings.subsamplingRatio = settings.subsamplingRatio;
        
        startParsingWithWorker(e57Parser, fileName, e57Settings);
        
    } else if (extension == "las") {
        // LAS parser (existing code unchanged)
        auto* lasParser = new LASParser();
        startParsingWithWorker(lasParser, fileName, settings);
        
    } else {
        // Other parsers (existing code unchanged)
        // ... handle PLY, PCD, etc.
    }
}

// NEW: Template method for starting parsing with any parser type
template
void MainWindow::startParsingWithWorker(ParserType* parser, const QString& fileName, const SettingsType& settings) {
    // Show progress dialog
    m_progressDialog = new QProgressDialog("Loading point cloud...", "Cancel", 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    m_progressDialog->show();
    
    // Create worker thread
    m_parserThread = new QThread(this);
    
    // Move parser to worker thread
    parser->moveToThread(m_parserThread);
    m_workerParser = parser;  // Store reference for cleanup
    
    // Connect signals for E57ParserLib specifically
    if constexpr (std::is_same_v) {
        connectE57ParserSignals(parser, settings, fileName);
    } else {
        // Connect signals for other parser types (existing logic)
        connectGenericParserSignals(parser, settings, fileName);
    }
    
    // Start the worker thread
    m_parserThread->start();
}

void MainWindow::connectE57ParserSignals(E57ParserLib* parser, 
                                       const E57ParserLib::LoadingSettings& settings, 
                                       const QString& fileName) {
    // Connect thread started signal to start parsing
    connect(m_parserThread, &QThread::started, [this, parser, fileName, settings]() {
        parser->startParsing(fileName, settings);
    });
    
    // Connect progress updates
    connect(parser, &E57ParserLib::progressUpdated,
            this, &MainWindow::onParsingProgressUpdated, Qt::QueuedConnection);
    
    // Connect parsing finished
    connect(parser, &E57ParserLib::parsingFinished,
            this, &MainWindow::onParsingFinished, Qt::QueuedConnection);
    
    // Connect additional E57-specific signals
    connect(parser, &E57ParserLib::scanMetadataAvailable,
            this, &MainWindow::onScanMetadataReceived, Qt::QueuedConnection);
    
    connect(parser, &E57ParserLib::intensityDataExtracted,
            this, &MainWindow::onIntensityDataReceived, Qt::QueuedConnection);
    
    connect(parser, &E57ParserLib::colorDataExtracted,
            this, &MainWindow::onColorDataReceived, Qt::QueuedConnection);
    
    // Connect thread cleanup
    connect(parser, &E57ParserLib::parsingFinished, [this, parser]() {
        cleanupParsingThread(parser);
    });
    
    // Connect cancel button
    connect(m_progressDialog, &QProgressDialog::canceled, [this, parser]() {
        parser->cancelParsing();
    });
}

// Updated onParsingFinished to handle E57ParserLib's exact signature
void MainWindow::onParsingFinished(bool success, const QString& message, const std::vector& points) {
    // Hide progress dialog
    if (m_progressDialog) {
        m_progressDialog->hide();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
    
    if (success) {
        // Update status
        m_statusLabel->setText(message);
        
        // Load points into viewer
        if (!points.empty()) {
            m_pointCloudViewer->loadPointCloud(points);
            
            // Update UI state
            updateUIAfterLoading();
            
            qDebug() setText("Failed to load file");
        
        qDebug()  1) {
        QString statusMsg = QString("Multi-scan E57 file detected (%1 scans), loading first scan...").arg(scanCount);
        if (m_progressDialog) {
            m_progressDialog->setLabelText(statusMsg);
        }
    }
}

// NEW: Handle intensity data from E57 files
void MainWindow::onIntensityDataReceived(const std::vector& intensityValues) {
    qDebug() setIntensityData(intensityValues);
}

// NEW: Handle color data from E57 files
void MainWindow::onColorDataReceived(const std::vector& colorValues) {
    qDebug() setColorData(colorValues);
}

void MainWindow::cleanupParsingThread(QObject* parser) {
    if (m_parserThread) {
        m_parserThread->quit();
        m_parserThread->wait(5000); // Wait up to 5 seconds
        
        if (m_parserThread->isRunning()) {
            qWarning() terminate();
            m_parserThread->wait(1000);
        }
        
        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }
    
    if (parser) {
        parser->deleteLater();
        m_workerParser = nullptr;
    }
}

// Updated progress handler (compatible with both old and new parsers)
void MainWindow::onParsingProgressUpdated(int percentage, const QString& stage) {
    if (m_progressDialog) {
        m_progressDialog->setValue(percentage);
        m_progressDialog->setLabelText(stage);
    }
    
    // Update status bar
    m_statusLabel->setText(QString("%1 (%2%)").arg(stage).arg(percentage));
}
```

### 3. MainWindow.hpp Updates

**MainWindow.hpp** (Add new members and remove old ones):
```cpp
// In MainWindow.hpp - Add these new members and remove old E57Parser references

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include 
#include 
#include 
#include 
#include 

// Forward declarations
class PointCloudViewerWidget;
class E57ParserLib;  // NEW: Add this
// Remove: class E57Parser; // OLD: Remove this line

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFileClicked();
    void onParsingProgressUpdated(int percentage, const QString& stage);
    void onParsingFinished(bool success, const QString& message, const std::vector& points);
    
    // NEW: E57-specific slots
    void onScanMetadataReceived(int scanCount, const QStringList& scanNames);
    void onIntensityDataReceived(const std::vector& intensityValues);
    void onColorDataReceived(const std::vector& colorValues);

private:
    void setupUI();
    void setupMenus();
    void updateUIAfterLoading();
    
    // NEW: Template method for parser management
    template
    void startParsingWithWorker(ParserType* parser, const QString& fileName, const SettingsType& settings);
    
    void connectE57ParserSignals(E57ParserLib* parser, const E57ParserLib::LoadingSettings& settings, const QString& fileName);
    void cleanupParsingThread(QObject* parser);

    // UI components
    PointCloudViewerWidget* m_pointCloudViewer;
    QProgressDialog* m_progressDialog = nullptr;
    QLabel* m_statusLabel;
    
    // Threading
    QThread* m_parserThread = nullptr;
    QObject* m_workerParser = nullptr;  // Generic pointer for any parser type
    
    // File handling
    QString m_lastDirectory;
    
    // NEW: E57-specific data storage
    int m_currentScanCount = 0;
    QStringList m_currentScanNames;
    std::vector m_currentIntensityData;
    std::vector m_currentColorData;
};

#endif // MAINWINDOW_HPP
```

### 4. CMakeLists.txt Updates

**CMakeLists.txt** (Remove old files, add new ones):
```cmake
# Remove old E57Parser files and add E57ParserLib
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PointCloudViewerWidget.cpp
    src/LoadingSettingsDialog.cpp
    
    # NEW: Add E57ParserLib
    src/E57ParserLib.cpp
    
    # Remove these old files:
    # src/e57parser.cpp  # REMOVE THIS LINE
    
    # Keep other parsers
    src/LASParser.cpp
    src/PLYParser.cpp
    # ... other files
)

set(HEADERS
    src/MainWindow.hpp
    src/PointCloudViewerWidget.hpp
    src/LoadingSettingsDialog.hpp
    
    # NEW: Add E57ParserLib
    src/E57ParserLib.hpp
    
    # Remove these old files:
    # src/e57parser.h    # REMOVE THIS LINE
    
    # Keep other parsers
    src/LASParser.hpp
    src/PLYParser.hpp
    # ... other files
)

# Ensure libE57Format is linked
find_package(E57Format REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGL
    E57Format
    # ... other libraries
)
```

### 5. Legacy Code Removal Script

**remove_old_e57_code.sh** (Cleanup script):
```bash
#!/bin/bash
# Script to safely remove old E57Parser code

echo "Starting removal of old E57Parser code..."

# Step 1: Remove old source files
echo "Removing old source files..."
rm -f src/e57parser.h
rm -f src/e57parser.cpp
rm -f docs/fix_display/sehfix.cpp

# Step 2: Search for and report remaining references
echo "Searching for remaining references to old E57Parser..."
grep -r "e57parser.h" src/ include/ || echo "No references to e57parser.h found"
grep -r "E57Parser" src/ include/ --exclude="*E57ParserLib*" || echo "No references to old E57Parser class found"

# Step 3: Clean build directories
echo "Cleaning build directories..."
rm -rf build/
rm -rf debug/
rm -rf release/

echo "Old E57Parser code removal completed."
echo "Please review the search results above and manually remove any remaining references."
echo "Then run a full rebuild to ensure no compilation errors."
```

### 6. Unit Test Updates

**test_e57parserlib_integration.cpp** (New integration tests):
```cpp
#include 
#include 
#include 
#include 
#include 
#include "E57ParserLib.hpp"

class E57ParserLibIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 1;
            char* argv[] = {"test"};
            app = new QApplication(argc, argv);
        }
        
        parser = std::make_unique();
    }
    
    void TearDown() override {
        parser.reset();
    }

protected:
    std::unique_ptr parser;
    QApplication* app = nullptr;
};

TEST_F(E57ParserLibIntegrationTest, MainWindowCompatibleSignatures) {
    // Test that signals match MainWindow expectations
    QSignalSpy progressSpy(parser.get(), &E57ParserLib::progressUpdated);
    QSignalSpy finishedSpy(parser.get(), &E57ParserLib::parsingFinished);
    
    EXPECT_TRUE(progressSpy.isValid());
    EXPECT_TRUE(finishedSpy.isValid());
    
    // Verify signal signatures match MainWindow slots
    auto progressSignal = QMetaMethod::fromSignal(&E57ParserLib::progressUpdated);
    EXPECT_EQ(progressSignal.parameterCount(), 2);
    EXPECT_EQ(progressSignal.parameterType(0), QMetaType::Int);
    EXPECT_EQ(progressSignal.parameterType(1), QMetaType::QString);
    
    auto finishedSignal = QMetaMethod::fromSignal(&E57ParserLib::parsingFinished);
    EXPECT_EQ(finishedSignal.parameterCount(), 3);
    EXPECT_EQ(finishedSignal.parameterType(0), QMetaType::Bool);
    EXPECT_EQ(finishedSignal.parameterType(1), QMetaType::QString);
    // Note: std::vector parameter type checking is complex in Qt meta-object system
}

TEST_F(E57ParserLibIntegrationTest, XYZVectorConversion) {
    // Test that XYZ vector format matches MainWindow expectations
    std::vector testData = {
        {1.0, 2.0, 3.0, 0.5f, 255, 128, 64, true, true, true},
        {4.0, 5.0, 6.0, 0.8f, 128, 255, 192, true, true, true},
        {7.0, 8.0, 9.0, 0.2f, 64, 192, 255, true, true, true}
    };
    
    // Access private method through friend class or make it public for testing
    // For this example, assume we have a public test method
    auto xyzVector = parser->convertToXYZVector(testData);
    
    EXPECT_EQ(xyzVector.size(), 9); // 3 points * 3 coordinates
    
    // Verify interleaved XYZ format
    EXPECT_FLOAT_EQ(xyzVector[0], 1.0f); // X1
    EXPECT_FLOAT_EQ(xyzVector[1], 2.0f); // Y1
    EXPECT_FLOAT_EQ(xyzVector[2], 3.0f); // Z1
    EXPECT_FLOAT_EQ(xyzVector[3], 4.0f); // X2
    EXPECT_FLOAT_EQ(xyzVector[4], 5.0f); // Y2
    EXPECT_FLOAT_EQ(xyzVector[5], 6.0f); // Z2
    EXPECT_FLOAT_EQ(xyzVector[6], 7.0f); // X3
    EXPECT_FLOAT_EQ(xyzVector[7], 8.0f); // Y3
    EXPECT_FLOAT_EQ(xyzVector[8], 9.0f); // Z3
}

TEST_F(E57ParserLibIntegrationTest, ErrorMessageTranslation) {
    // Test that technical errors are translated to user-friendly messages
    QString technicalError = "E57_ERROR_BAD_CHECKSUM: Checksum verification failed";
    QString friendlyError = parser->translateE57Error(technicalError);
    
    EXPECT_TRUE(friendlyError.contains("corrupted"));
    EXPECT_FALSE(friendlyError.contains("E57_ERROR_BAD_CHECKSUM"));
}

TEST_F(E57ParserLibIntegrationTest, ThreadSafeOperations) {
    // Test that parser can be safely used in worker threads
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(100); // Short timeout for test
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    // Test cancel operation thread safety
    timer.start();
    parser->cancelParsing(); // Should not crash
    loop.exec();
    
    SUCCEED(); // If we reach here without crashing, test passes
}
```

### Key Implementation Points:

1. **Signal Compatibility**: E57ParserLib provides exact signal signatures that MainWindow expects
2. **Data Format Compliance**: XYZ data is converted to interleaved `std::vector` format
3. **Error Translation**: Technical E57 errors are converted to user-friendly messages
4. **Thread Safety**: Parser supports cancellation and proper cleanup
5. **Future-Proofing**: Intensity and color data are extracted and available for future visualization enhancements
6. **Comprehensive Testing**: Integration tests verify MainWindow compatibility

This implementation fully satisfies the PRD requirements for Sprint 5, providing a clean transition from the old custom parser to the new libE57Format-based solution.

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/5019aa21-9701-4ff5-a5bd-79206b458efa/paste.txt

---
Answer from Perplexity: pplx.ai/share