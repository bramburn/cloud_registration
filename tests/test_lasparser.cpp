#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include <QtEndian>
#include <QFileInfo>
#include <QDir>
#include "lasparser.h"

class LasParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize Qt application for testing
        static QCoreApplication* staticApp = nullptr;
        if (!QCoreApplication::instance() && !staticApp) {
            int argc = 1;
            static char appName[] = "LasParserTest";
            static char* argv[] = {appName, nullptr};
            staticApp = new QCoreApplication(argc, argv);
        }

        parser = new LasParser();

        // Set up paths for real test file
        QString currentDir = QDir::currentPath();
        qDebug() << "Current directory:" << currentDir;

        // Try different possible paths for the sample file from tests/ directory
        QStringList possiblePaths = {
            "../sample/S2max-Power line202503.las",
            "../../sample/S2max-Power line202503.las",
            "sample/S2max-Power line202503.las"
        };

        for (const QString& path : possiblePaths) {
            if (QFileInfo::exists(path)) {
                realLasFile = path;
                qDebug() << "Found real LAS file at:" << realLasFile;
                break;
            }
        }

        if (realLasFile.isEmpty()) {
            qDebug() << "Warning: Real LAS file not found, will skip real file tests";
        }
    }

    void TearDown() override
    {
        delete parser;
        // Don't delete static app as it might be used by other tests
    }

    // Helper function to create a mock LAS file with Format 0
    QString createMockLasFile(uint8_t pointFormat = 0, uint32_t numPoints = 100, uint8_t versionMinor = 2)
    {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(false); // Keep file for testing

        if (!tempFile->open()) {
            return QString();
        }

        QDataStream stream(tempFile);
        stream.setByteOrder(QDataStream::LittleEndian);

        // Write LAS header
        // File signature "LASF"
        tempFile->write("LASF", 4);

        // File source ID (2 bytes)
        stream << static_cast<quint16>(0);

        // Global encoding (2 bytes)
        stream << static_cast<quint16>(0);

        // GUID (16 bytes) - write zeros
        for (int i = 0; i < 16; ++i) {
            stream << static_cast<quint8>(0);
        }

        // Version major/minor (Sprint 1.3: Support for 1.2, 1.3, 1.4)
        stream << static_cast<quint8>(1); // Major
        stream << static_cast<quint8>(versionMinor); // Minor (2, 3, or 4)

        // System identifier (32 bytes) - write zeros
        for (int i = 0; i < 32; ++i) {
            stream << static_cast<quint8>(0);
        }

        // Generating software (32 bytes) - write zeros
        for (int i = 0; i < 32; ++i) {
            stream << static_cast<quint8>(0);
        }

        // Creation day/year
        stream << static_cast<quint16>(1);    // Day
        stream << static_cast<quint16>(2024); // Year

        // Header size (Sprint 1.3: Different sizes for different versions)
        uint16_t headerSize = 227; // LAS 1.2
        if (versionMinor == 3) headerSize = 235; // LAS 1.3
        else if (versionMinor == 4) headerSize = 375; // LAS 1.4
        stream << headerSize;

        // Point data offset
        stream << static_cast<quint32>(headerSize);

        // Number of VLRs
        stream << static_cast<quint32>(0);

        // Point data format
        stream << static_cast<quint8>(pointFormat);

        // Point data record length
        uint16_t recordLength = 20; // Format 0
        if (pointFormat == 1) recordLength = 28;
        else if (pointFormat == 2) recordLength = 26;
        else if (pointFormat == 3) recordLength = 34;
        stream << recordLength;

        // Number of point records
        stream << numPoints;

        // Number of points by return (5 * 4 bytes)
        for (int i = 0; i < 5; ++i) {
            stream << static_cast<quint32>(0);
        }

        // Scale factors
        stream << 0.01; // X scale
        stream << 0.01; // Y scale
        stream << 0.01; // Z scale

        // Offsets
        stream << 0.0; // X offset
        stream << 0.0; // Y offset
        stream << 0.0; // Z offset

        // Min/Max values
        stream << 100.0; // Max X
        stream << 0.0;   // Min X
        stream << 100.0; // Max Y
        stream << 0.0;   // Min Y
        stream << 100.0; // Max Z
        stream << 0.0;   // Min Z

        // Write point data
        for (uint32_t i = 0; i < numPoints; ++i) {
            // X, Y, Z coordinates (scaled integers)
            stream << static_cast<qint32>(i * 100);      // X
            stream << static_cast<qint32>(i * 100 + 50); // Y
            stream << static_cast<qint32>(i * 10);       // Z

            // Fill remaining bytes based on format
            int remainingBytes = recordLength - 12; // 12 bytes for XYZ
            for (int j = 0; j < remainingBytes; ++j) {
                stream << static_cast<quint8>(0);
            }
        }

        QString fileName = tempFile->fileName();
        tempFile->close();
        delete tempFile;

        return fileName;
    }

    // Helper function to create an invalid file
    QString createInvalidFile()
    {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(false);

        if (!tempFile->open()) {
            return QString();
        }

        // Write some random data
        tempFile->write("This is not a LAS file");

        QString fileName = tempFile->fileName();
        tempFile->close();
        delete tempFile;

        return fileName;
    }

    QCoreApplication* app = nullptr;
    LasParser* parser = nullptr;
    QString realLasFile; // Path to real LAS test file
};

TEST_F(LasParserTest, ValidLasFileDetection)
{
    QString mockFile = createMockLasFile();
    ASSERT_FALSE(mockFile.isEmpty());

    bool isValid = parser->isValidLasFile(mockFile);
    EXPECT_TRUE(isValid);

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(LasParserTest, InvalidFileDetection)
{
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    bool isValid = parser->isValidLasFile(invalidFile);
    EXPECT_FALSE(isValid);

    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(LasParserTest, NonExistentFileHandling)
{
    QString nonExistentFile = "/path/that/does/not/exist.las";

    bool isValid = parser->isValidLasFile(nonExistentFile);
    EXPECT_FALSE(isValid);
}

TEST_F(LasParserTest, Format0Parsing)
{
    QString mockFile = createMockLasFile(0, 10); // Format 0, 10 points
    ASSERT_FALSE(mockFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(mockFile);

        // Should have 30 values (10 points * 3 coordinates)
        EXPECT_EQ(points.size(), 30);
        EXPECT_EQ(points.size() % 3, 0);

        // Check first point coordinates (scaled and offset applied)
        EXPECT_FLOAT_EQ(points[0], 0.0f);   // X: 0 * 0.01 + 0.0
        EXPECT_FLOAT_EQ(points[1], 0.5f);   // Y: 50 * 0.01 + 0.0
        EXPECT_FLOAT_EQ(points[2], 0.0f);   // Z: 0 * 0.01 + 0.0

        // Check second point coordinates
        EXPECT_FLOAT_EQ(points[3], 1.0f);   // X: 100 * 0.01 + 0.0
        EXPECT_FLOAT_EQ(points[4], 1.5f);   // Y: 150 * 0.01 + 0.0
        EXPECT_FLOAT_EQ(points[5], 0.1f);   // Z: 10 * 0.01 + 0.0

    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(LasParserTest, Format1Parsing)
{
    QString mockFile = createMockLasFile(1, 5); // Format 1, 5 points
    ASSERT_FALSE(mockFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(mockFile);

        // Should have 15 values (5 points * 3 coordinates)
        EXPECT_EQ(points.size(), 15);
        EXPECT_EQ(points.size() % 3, 0);

    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(LasParserTest, ErrorHandling)
{
    // Test with a non-existent file
    try {
        std::vector<float> points = parser->parse("/non/existent/file.las");
        FAIL() << "Expected exception was not thrown";
    } catch (const LasParseException&) {
        // Expected behavior
        EXPECT_FALSE(parser->getLastError().isEmpty());
    } catch (const std::exception& e) {
        FAIL() << "Wrong exception type: " << e.what();
    }
}

TEST_F(LasParserTest, InvalidHeaderHandling)
{
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(invalidFile);
        FAIL() << "Expected exception was not thrown";
    } catch (const LasParseException&) {
        // Expected behavior
        EXPECT_FALSE(parser->getLastError().isEmpty());
    } catch (const std::exception& e) {
        FAIL() << "Wrong exception type: " << e.what();
    }

    // Clean up
    QFile::remove(invalidFile);
}

// Sprint 1.3: Enhanced LAS format support tests

TEST_F(LasParserTest, LAS13VersionSupport)
{
    // Test Case 1.3.2.A: LAS 1.3 header validation
    QString mockFile = createMockLasFile(0, 10, 3); // LAS 1.3, PDRF 0
    ASSERT_FALSE(mockFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(mockFile);
        EXPECT_EQ(points.size(), 30); // 10 points * 3 coordinates
        EXPECT_EQ(parser->getVersionMajor(), 1);
        EXPECT_EQ(parser->getVersionMinor(), 3);
        EXPECT_EQ(parser->getPointDataFormat(), 0);
    } catch (const std::exception& e) {
        FAIL() << "LAS 1.3 parsing failed: " << e.what();
    }

    QFile::remove(mockFile);
}

TEST_F(LasParserTest, LAS14VersionSupport)
{
    // Test Case 1.3.2.B: LAS 1.4 header validation
    QString mockFile = createMockLasFile(1, 5, 4); // LAS 1.4, PDRF 1
    ASSERT_FALSE(mockFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(mockFile);
        EXPECT_EQ(points.size(), 15); // 5 points * 3 coordinates
        EXPECT_EQ(parser->getVersionMajor(), 1);
        EXPECT_EQ(parser->getVersionMinor(), 4);
        EXPECT_EQ(parser->getPointDataFormat(), 1);
    } catch (const std::exception& e) {
        FAIL() << "LAS 1.4 parsing failed: " << e.what();
    }

    QFile::remove(mockFile);
}

TEST_F(LasParserTest, UnsupportedVersionHandling)
{
    // Test unsupported LAS version (e.g., 1.5)
    QString mockFile = createMockLasFile(0, 10, 5); // LAS 1.5 (unsupported)
    ASSERT_FALSE(mockFile.isEmpty());

    try {
        std::vector<float> points = parser->parse(mockFile);
        FAIL() << "Expected exception for unsupported LAS version";
    } catch (const LasParseException&) {
        // Expected behavior
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("Unsupported LAS version"));
        EXPECT_TRUE(error.contains("1.5"));
    }

    QFile::remove(mockFile);
}

TEST_F(LasParserTest, RecordLengthValidation)
{
    // Test Case 1.3.1.I: Record length mismatch error handling
    QString mockFile = createMockLasFile(0, 10, 2); // LAS 1.2, PDRF 0
    ASSERT_FALSE(mockFile.isEmpty());

    // Manually corrupt the record length in the file
    QFile file(mockFile);
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    file.seek(105); // Position of point data record length field
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint16>(28); // Wrong length for PDRF 0 (should be 20)
    file.close();

    try {
        std::vector<float> points = parser->parse(mockFile);
        FAIL() << "Expected exception for record length mismatch";
    } catch (const LasParseException&) {
        // Expected behavior
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("Point data record length mismatch"));
        EXPECT_TRUE(error.contains("Expected 20"));
        EXPECT_TRUE(error.contains("got 28"));
    }

    QFile::remove(mockFile);
}

TEST_F(LasParserTest, ScaleFactorValidation)
{
    // Test zero scale factor handling
    QString mockFile = createMockLasFile(0, 10, 2); // LAS 1.2, PDRF 0
    ASSERT_FALSE(mockFile.isEmpty());

    // Manually set X scale factor to zero
    QFile file(mockFile);
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    file.seek(131); // Position of X scale factor
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << 0.0; // Zero scale factor
    file.close();

    try {
        std::vector<float> points = parser->parse(mockFile);
        FAIL() << "Expected exception for zero scale factor";
    } catch (const LasParseException&) {
        // Expected behavior
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("Scale factor for X axis is zero"));
    }

    QFile::remove(mockFile);
}

TEST_F(LasParserTest, AllPDRFSupport)
{
    // Test Case 1.3.1.E-H: All supported PDRFs (0-3)
    for (int pdrf = 0; pdrf <= 3; ++pdrf) {
        QString mockFile = createMockLasFile(pdrf, 5, 2); // LAS 1.2
        ASSERT_FALSE(mockFile.isEmpty()) << "Failed to create mock file for PDRF " << pdrf;

        try {
            std::vector<float> points = parser->parse(mockFile);
            EXPECT_EQ(points.size(), 15) << "Wrong point count for PDRF " << pdrf; // 5 points * 3 coordinates
            EXPECT_EQ(parser->getPointDataFormat(), pdrf) << "Wrong PDRF reported";

            // Validate coordinates are within expected range
            for (size_t i = 0; i < points.size(); i += 3) {
                EXPECT_GE(points[i], -1000.0f) << "X coordinate out of range for PDRF " << pdrf;
                EXPECT_LE(points[i], 1000.0f) << "X coordinate out of range for PDRF " << pdrf;
            }
        } catch (const std::exception& e) {
            FAIL() << "PDRF " << pdrf << " parsing failed: " << e.what();
        }

        QFile::remove(mockFile);
    }
}

// Sprint 1.3: Real LAS file testing
TEST_F(LasParserTest, RealLasFileParsing)
{
    if (realLasFile.isEmpty()) {
        GTEST_SKIP() << "Real LAS file not available, skipping test";
        return;
    }

    qDebug() << "=== Testing Real LAS File ===";
    qDebug() << "File path:" << realLasFile;

    // First, validate it's a valid LAS file
    EXPECT_TRUE(parser->isValidLasFile(realLasFile)) << "Real LAS file should be valid";

    try {
        // Parse the real file
        std::vector<float> points = parser->parse(realLasFile);

        // Basic validation
        EXPECT_GT(points.size(), 0) << "Real LAS file should contain points";
        EXPECT_EQ(points.size() % 3, 0) << "Point count should be multiple of 3 (XYZ)";

        // Log file information
        qDebug() << "Real LAS file info:";
        qDebug() << "  Version:" << parser->getVersionMajor() << "." << parser->getVersionMinor();
        qDebug() << "  Point Data Format:" << parser->getPointDataFormat();
        qDebug() << "  Point count:" << (points.size() / 3);
        qDebug() << "  Header size:" << parser->getHeaderSize();
        qDebug() << "  Record length:" << parser->getPointDataRecordLength();

        // Validate version is supported (1.2, 1.3, or 1.4)
        EXPECT_EQ(parser->getVersionMajor(), 1) << "Major version should be 1";
        EXPECT_GE(parser->getVersionMinor(), 2) << "Minor version should be >= 2";
        EXPECT_LE(parser->getVersionMinor(), 4) << "Minor version should be <= 4";

        // Validate PDRF is supported (0-3)
        EXPECT_GE(parser->getPointDataFormat(), 0) << "PDRF should be >= 0";
        EXPECT_LE(parser->getPointDataFormat(), 3) << "PDRF should be <= 3";

        // Sample coordinate validation (check first few points)
        if (points.size() >= 9) {
            qDebug() << "Sample coordinates:";
            qDebug() << "  Point 1:" << points[0] << points[1] << points[2];
            qDebug() << "  Point 2:" << points[3] << points[4] << points[5];
            qDebug() << "  Point 3:" << points[6] << points[7] << points[8];

            // Basic sanity checks - coordinates shouldn't be extreme values
            for (int i = 0; i < 9; i += 3) {
                EXPECT_GT(points[i], -1e6) << "X coordinate seems unreasonable";
                EXPECT_LT(points[i], 1e6) << "X coordinate seems unreasonable";
                EXPECT_GT(points[i+1], -1e6) << "Y coordinate seems unreasonable";
                EXPECT_LT(points[i+1], 1e6) << "Y coordinate seems unreasonable";
                EXPECT_GT(points[i+2], -1e6) << "Z coordinate seems unreasonable";
                EXPECT_LT(points[i+2], 1e6) << "Z coordinate seems unreasonable";
            }
        }

    } catch (const std::exception& e) {
        FAIL() << "Real LAS file parsing failed: " << e.what()
               << "\nLast error: " << parser->getLastError().toStdString();
    }
}

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
