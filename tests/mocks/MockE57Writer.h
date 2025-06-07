#ifndef MOCKE57WRITER_H
#define MOCKE57WRITER_H

#include <gmock/gmock.h>
#include "../../src/IE57Writer.h"
#include <vector>

/**
 * @brief MockE57Writer - Mock implementation of IE57Writer for testing
 * 
 * This mock class implements the IE57Writer interface using Google Mock's
 * MOCK_METHOD macro. It allows tests to verify that methods like createFile
 * and writePoints are called with correct parameters during export operations,
 * without actually writing files to disk.
 * 
 * Sprint 5 Testing Requirements:
 * - Enables unit testing of file export logic without disk I/O
 * - Allows verification of correct method call sequences
 * - Provides controllable success/failure scenarios for testing error handling
 * - Verifies correct parameter passing during export operations
 */
class MockE57Writer : public IE57Writer {
    Q_OBJECT

public:
    explicit MockE57Writer(QObject *parent = nullptr) : IE57Writer(parent) {}
    virtual ~MockE57Writer() = default;

    // Core file operations
    MOCK_METHOD(bool, createFile, (const QString& filePath), (override));
    MOCK_METHOD(bool, closeFile, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));

    // Scan operations
    MOCK_METHOD(bool, addScan, (const ScanMetadata& metadata), (override));
    MOCK_METHOD(bool, writePoints, (const std::vector<Point3D>& points, const ExportOptions& options), (override));
    MOCK_METHOD(bool, writeScanData, (const ScanData& scanData), (override));

    // Metadata and status
    MOCK_METHOD(QString, getLastError, (), (const, override));
    MOCK_METHOD(QString, getCurrentFilePath, (), (const, override));
    MOCK_METHOD(int, getScanCount, (), (const, override));
    MOCK_METHOD(bool, setFileMetadata, (const QString& guid, const QString& description, const QString& creationDateTime), (override));

    // Helper methods for tests to emit signals
    void emitFileCreationStarted(const QString& filePath) {
        emit fileCreationStarted(filePath);
    }

    void emitFileCreationFinished(bool success, const QString& message) {
        emit fileCreationFinished(success, message);
    }

    void emitScanAdded(int scanIndex, const QString& scanName) {
        emit scanAdded(scanIndex, scanName);
    }

    void emitProgressUpdated(int percentage, int64_t pointsWritten) {
        emit progressUpdated(percentage, pointsWritten);
    }

    void emitErrorOccurred(const QString& errorMessage) {
        emit errorOccurred(errorMessage);
    }

    // Test helper methods to set up common scenarios
    void setupSuccessfulWriting(const QString& filePath = "test_output.e57") {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, createFile(_))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, isOpen())
            .WillByDefault(Return(true));
        
        ON_CALL(*this, addScan(_))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, writePoints(_, _))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, writeScanData(_))
            .WillByDefault(Return(true));
        
        ON_CALL(*this, closeFile())
            .WillByDefault(Return(true));
        
        ON_CALL(*this, getCurrentFilePath())
            .WillByDefault(Return(filePath));
        
        ON_CALL(*this, getLastError())
            .WillByDefault(Return(QString()));
        
        ON_CALL(*this, getScanCount())
            .WillByDefault(Return(1));
    }

    void setupFailedWriting(const QString& errorMessage = "Mock writing error") {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, createFile(_))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, isOpen())
            .WillByDefault(Return(false));
        
        ON_CALL(*this, addScan(_))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, writePoints(_, _))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, writeScanData(_))
            .WillByDefault(Return(false));
        
        ON_CALL(*this, closeFile())
            .WillByDefault(Return(false));
        
        ON_CALL(*this, getCurrentFilePath())
            .WillByDefault(Return(QString()));
        
        ON_CALL(*this, getLastError())
            .WillByDefault(Return(errorMessage));
        
        ON_CALL(*this, getScanCount())
            .WillByDefault(Return(0));
    }

    void setupPartialFailure(bool createSuccess = true, 
                           bool addScanSuccess = true, 
                           bool writePointsSuccess = false) {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, createFile(_))
            .WillByDefault(Return(createSuccess));
        
        ON_CALL(*this, addScan(_))
            .WillByDefault(Return(addScanSuccess));
        
        ON_CALL(*this, writePoints(_, _))
            .WillByDefault(Return(writePointsSuccess));
        
        ON_CALL(*this, isOpen())
            .WillByDefault(Return(createSuccess));
        
        if (!writePointsSuccess) {
            ON_CALL(*this, getLastError())
                .WillByDefault(Return("Failed to write points"));
        }
    }

    // Create test data helpers
    static ScanMetadata createTestScanMetadata(const QString& name = "Test Scan",
                                             const QString& description = "Test scan for unit testing") {
        ScanMetadata metadata;
        metadata.name = name;
        metadata.description = description;
        metadata.pointCount = 100;
        metadata.acquisitionDateTime = "2024-01-01T12:00:00Z";
        metadata.sensorVendor = "Test Vendor";
        metadata.sensorModel = "Test Model";
        metadata.sensorSerialNumber = "TEST123";
        metadata.temperatureCelsius = 20.0;
        metadata.relativeHumidity = 50.0;
        metadata.atmosphericPressure = 1013.25;
        
        // Set identity pose
        metadata.pose.translation[0] = 0.0;
        metadata.pose.translation[1] = 0.0;
        metadata.pose.translation[2] = 0.0;
        metadata.pose.rotation[0] = 1.0; // w
        metadata.pose.rotation[1] = 0.0; // x
        metadata.pose.rotation[2] = 0.0; // y
        metadata.pose.rotation[3] = 0.0; // z
        
        return metadata;
    }

    static std::vector<Point3D> createTestPoints(int numPoints = 10) {
        std::vector<Point3D> points;
        points.reserve(numPoints);
        
        for (int i = 0; i < numPoints; ++i) {
            points.emplace_back(static_cast<double>(i), 
                              static_cast<double>(i + 1), 
                              static_cast<double>(i + 2));
        }
        
        return points;
    }

    static ExportOptions createTestExportOptions(bool includeIntensity = false,
                                               bool includeColor = false,
                                               bool compressData = true) {
        ExportOptions options;
        options.includeIntensity = includeIntensity;
        options.includeColor = includeColor;
        options.compressData = compressData;
        options.coordinateScaleFactor = 0.0001; // 0.1mm precision
        options.coordinateSystem = "CARTESIAN";
        return options;
    }

    static ScanData createTestScanData(int numPoints = 10,
                                     bool includeIntensity = false,
                                     bool includeColor = false) {
        ScanData scanData;
        scanData.metadata = createTestScanMetadata();
        scanData.points = createTestPoints(numPoints);
        
        if (includeIntensity) {
            scanData.intensities.reserve(numPoints);
            for (int i = 0; i < numPoints; ++i) {
                scanData.intensities.push_back(static_cast<float>(i * 0.1));
            }
        }
        
        if (includeColor) {
            scanData.colors.reserve(numPoints * 3);
            for (int i = 0; i < numPoints; ++i) {
                scanData.colors.push_back(static_cast<uint8_t>(i % 256));      // R
                scanData.colors.push_back(static_cast<uint8_t>((i + 1) % 256)); // G
                scanData.colors.push_back(static_cast<uint8_t>((i + 2) % 256)); // B
            }
        }
        
        return scanData;
    }
};

#endif // MOCKE57WRITER_H
