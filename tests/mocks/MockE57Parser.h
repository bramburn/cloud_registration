#ifndef MOCKE57PARSER_H
#define MOCKE57PARSER_H

#include <string>
#include <vector>

#include "../../src/IE57Parser.h"

#include <gmock/gmock.h>

/**
 * @brief MockE57Parser - Mock implementation of IE57Parser for testing
 *
 * This mock class implements the IE57Parser interface using Google Mock's
 * MOCK_METHOD macro. It allows tests to simulate file loading success/failure,
 * return predefined point cloud data, and verify that methods are called
 * with correct parameters.
 *
 * Sprint 5 Testing Requirements:
 * - Enables unit testing of MainPresenter without file system dependencies
 * - Allows simulation of various parsing scenarios (success, failure, errors)
 * - Provides controllable test data for consistent test results
 * - Verifies correct interaction patterns between components
 */
class MockE57Parser : public IE57Parser
{
    Q_OBJECT

public:
    explicit MockE57Parser(QObject* parent = nullptr) : IE57Parser(parent) {}
    virtual ~MockE57Parser() = default;

    // Main entry point for MainWindow integration
    MOCK_METHOD(void, startParsing, (const QString& filePath, const LoadingSettings& settings), (override));
    MOCK_METHOD(void, cancelParsing, (), (override));
    MOCK_METHOD(QString, getLastError, (), (const, override));

    // Utility methods for MainWindow
    MOCK_METHOD(bool, isValidE57File, (const QString& filePath), (override));
    MOCK_METHOD(int, getScanCount, (const QString& filePath), (override));

    // Core file operations
    MOCK_METHOD(bool, openFile, (const std::string& filePath), (override));
    MOCK_METHOD(void, closeFile, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));

    // Metadata access
    MOCK_METHOD(std::string, getGuid, (), (const, override));
    MOCK_METHOD(std::pair<int, int>, getVersion, (), (const, override));
    MOCK_METHOD(int, getScanCount, (), (const, override));
    MOCK_METHOD(ScanMetadata, getScanMetadata, (int scanIndex), (override));

    // Point data extraction
    MOCK_METHOD(std::vector<float>, extractPointData, (int scanIndex), (override));
    MOCK_METHOD(std::vector<PointData>, extractEnhancedPointData, (int scanIndex), (override));
    MOCK_METHOD(int64_t, getPointCount, (int scanIndex), (const, override));

    // Helper methods for tests to emit signals
    void emitProgressUpdated(int percentage, const QString& stage)
    {
        emit progressUpdated(percentage, stage);
    }

    void emitParsingFinished(bool success, const QString& message, const std::vector<float>& points)
    {
        emit parsingFinished(success, message, points);
    }

    void emitScanMetadataAvailable(int scanCount, const QStringList& scanNames)
    {
        emit scanMetadataAvailable(scanCount, scanNames);
    }

    void emitIntensityDataExtracted(const std::vector<float>& intensityValues)
    {
        emit intensityDataExtracted(intensityValues);
    }

    void emitColorDataExtracted(const std::vector<uint8_t>& colorValues)
    {
        emit colorDataExtracted(colorValues);
    }

    // Test helper methods to set up common scenarios
    void setupSuccessfulParsing(const std::vector<float>& testPoints = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f})
    {
        using ::testing::_;
        using ::testing::DoAll;
        using ::testing::InvokeWithoutArgs;
        using ::testing::Return;

        ON_CALL(*this, openFile(_)).WillByDefault(Return(true));

        ON_CALL(*this, isOpen()).WillByDefault(Return(true));

        ON_CALL(*this, extractPointData(_)).WillByDefault(Return(testPoints));

        ON_CALL(*this, getLastError()).WillByDefault(Return(QString()));

        ON_CALL(*this, getScanCount()).WillByDefault(Return(1));

        ON_CALL(*this, getPointCount(_)).WillByDefault(Return(testPoints.size() / 3));
    }

    void setupFailedParsing(const QString& errorMessage = "Mock parsing error")
    {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, openFile(_)).WillByDefault(Return(false));

        ON_CALL(*this, isOpen()).WillByDefault(Return(false));

        ON_CALL(*this, extractPointData(_)).WillByDefault(Return(std::vector<float>()));

        ON_CALL(*this, getLastError()).WillByDefault(Return(errorMessage));

        ON_CALL(*this, getScanCount()).WillByDefault(Return(0));

        ON_CALL(*this, getPointCount(_)).WillByDefault(Return(0));
    }

    void setupValidFile(const QString& filePath, bool isValid = true)
    {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, isValidE57File(filePath)).WillByDefault(Return(isValid));
    }

    void
    setupMetadata(const std::string& guid = "test-guid", const std::pair<int, int>& version = {1, 0}, int scanCount = 1)
    {
        using ::testing::Return;

        ON_CALL(*this, getGuid()).WillByDefault(Return(guid));

        ON_CALL(*this, getVersion()).WillByDefault(Return(version));

        ON_CALL(*this, getScanCount()).WillByDefault(Return(scanCount));
    }

    void setupScanMetadata(int scanIndex, const ScanMetadata& metadata)
    {
        using ::testing::Return;

        ON_CALL(*this, getScanMetadata(scanIndex)).WillByDefault(Return(metadata));
    }

    // Create test scan metadata
    static ScanMetadata
    createTestScanMetadata(int index = 0, const QString& name = "Test Scan", int64_t pointCount = 100)
    {
        ScanMetadata metadata;
        metadata.index = index;
        metadata.name = name;
        metadata.pointCount = pointCount;
        metadata.description = "Test scan for unit testing";
        return metadata;
    }

    // Create test point data
    static std::vector<float> createTestPointData(int numPoints = 10)
    {
        std::vector<float> points;
        points.reserve(numPoints * 3);

        for (int i = 0; i < numPoints; ++i)
        {
            points.push_back(static_cast<float>(i));      // x
            points.push_back(static_cast<float>(i + 1));  // y
            points.push_back(static_cast<float>(i + 2));  // z
        }

        return points;
    }

    // Create test enhanced point data
    static std::vector<PointData> createTestEnhancedPointData(int numPoints = 10)
    {
        std::vector<PointData> points;
        points.reserve(numPoints);

        for (int i = 0; i < numPoints; ++i)
        {
            PointData point;
            point.x = static_cast<float>(i);
            point.y = static_cast<float>(i + 1);
            point.z = static_cast<float>(i + 2);
            point.intensity = static_cast<float>(i * 0.1);
            point.colorRed = static_cast<uint8_t>(i % 256);
            point.colorGreen = static_cast<uint8_t>((i + 1) % 256);
            point.colorBlue = static_cast<uint8_t>((i + 2) % 256);
            points.push_back(point);
        }

        return points;
    }
};

#endif  // MOCKE57PARSER_H
