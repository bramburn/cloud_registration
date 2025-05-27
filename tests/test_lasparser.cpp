#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include <QtEndian>
#include "lasparser.h"

class LasParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize Qt application for testing
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }

        parser = new LasParser();
    }

    void TearDown() override
    {
        delete parser;
        // Don't delete app as it might be used by other tests
    }

    // Helper function to create a mock LAS file with Format 0
    QString createMockLasFile(uint8_t pointFormat = 0, uint32_t numPoints = 100)
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

        // Version major/minor
        stream << static_cast<quint8>(1); // Major
        stream << static_cast<quint8>(2); // Minor

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

        // Header size
        stream << static_cast<quint16>(227);

        // Point data offset
        stream << static_cast<quint32>(227);

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

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
