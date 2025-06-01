#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QSignalSpy>
#include "e57parser.h"

class Sprint12IntegrationTest : public ::testing::Test
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

        parser = new E57Parser();
    }

    void TearDown() override
    {
        delete parser;
        // Don't delete app as it might be used by other tests
    }

    QCoreApplication* app = nullptr;
    E57Parser* parser = nullptr;
};

// Sprint 1.2: Test loading CompressedVector E57 file with uncompressed data
TEST_F(Sprint12IntegrationTest, LoadCompressedVectorUncompressedData)
{
    QString testFile = "test_data/compressedvector_uncompressed_data.e57";
    
    if (!QFile::exists(testFile)) {
        GTEST_SKIP() << "Test file " << testFile.toStdString() << " not found";
        return;
    }

    qDebug() << "Testing CompressedVector E57 file:" << testFile;

    // Test file validation
    bool isValid = parser->isValidE57File(testFile);
    EXPECT_TRUE(isValid) << "File should be recognized as valid E57";

    if (!isValid) {
        qDebug() << "File validation failed:" << parser->getLastError();
        return;
    }

    // Set up signal spy to capture parsing results
    QSignalSpy progressSpy(parser, &E57Parser::progressUpdated);
    QSignalSpy finishedSpy(parser, &E57Parser::parsingFinished);

    // Attempt to parse the file
    std::vector<float> points = parser->parse(testFile);

    // Check if parsing was successful
    if (points.empty()) {
        qDebug() << "Parsing failed with error:" << parser->getLastError();
        
        // For Sprint 1.2, we expect this to fail at binary extraction stage
        // since we're testing the XML parsing and CompressedVector detection
        EXPECT_FALSE(parser->getLastError().isEmpty());
        EXPECT_TRUE(parser->getLastError().contains("binary") || 
                   parser->getLastError().contains("extract") ||
                   parser->getLastError().contains("offset"));
    } else {
        // If parsing succeeded, verify the data
        EXPECT_FALSE(points.empty());
        EXPECT_EQ(points.size() % 3, 0); // Should be divisible by 3 (X, Y, Z)
        
        qDebug() << "Successfully parsed" << (points.size() / 3) << "points";
        
        // Verify the expected test data (1,2,3), (4,5,6), (7,8,9)
        if (points.size() >= 9) {
            EXPECT_FLOAT_EQ(points[0], 1.0f);
            EXPECT_FLOAT_EQ(points[1], 2.0f);
            EXPECT_FLOAT_EQ(points[2], 3.0f);
            EXPECT_FLOAT_EQ(points[3], 4.0f);
            EXPECT_FLOAT_EQ(points[4], 5.0f);
            EXPECT_FLOAT_EQ(points[5], 6.0f);
            EXPECT_FLOAT_EQ(points[6], 7.0f);
            EXPECT_FLOAT_EQ(points[7], 8.0f);
            EXPECT_FLOAT_EQ(points[8], 9.0f);
        }
    }

    // Verify signals were emitted
    EXPECT_GE(finishedSpy.count(), 1) << "parsingFinished signal should be emitted";
}

// Sprint 1.2: Test loading malformed CompressedVector E57 file (should fail with detailed errors)
TEST_F(Sprint12IntegrationTest, LoadMalformedCompressedVector)
{
    QString testFile = "test_data/malformed_compressedvector.e57";
    
    if (!QFile::exists(testFile)) {
        GTEST_SKIP() << "Test file " << testFile.toStdString() << " not found";
        return;
    }

    qDebug() << "Testing malformed CompressedVector E57 file:" << testFile;

    // Test file validation (should still pass basic E57 validation)
    bool isValid = parser->isValidE57File(testFile);
    EXPECT_TRUE(isValid) << "File should be recognized as valid E57 (header-wise)";

    // Set up signal spy to capture parsing results
    QSignalSpy finishedSpy(parser, &E57Parser::parsingFinished);

    // Attempt to parse the file (should fail)
    std::vector<float> points = parser->parse(testFile);

    // Should fail with detailed error message
    EXPECT_TRUE(points.empty()) << "Parsing should fail for malformed file";
    EXPECT_FALSE(parser->getLastError().isEmpty()) << "Should have detailed error message";

    QString error = parser->getLastError();
    qDebug() << "Expected detailed error:" << error;

    // Should contain error code and context
    EXPECT_TRUE(error.contains("E57_ERROR_") || error.contains("invalid")) 
        << "Error should contain error code or indicate invalid data";

    // Verify parsingFinished signal was emitted with failure
    EXPECT_GE(finishedSpy.count(), 1);
    if (finishedSpy.count() > 0) {
        QList<QVariant> arguments = finishedSpy.first();
        EXPECT_FALSE(arguments.at(0).toBool()) << "Success flag should be false";
    }
}

// Sprint 1.2: Test error reporting quality
TEST_F(Sprint12IntegrationTest, ErrorReportingQuality)
{
    QString testFile = "test_data/malformed_compressedvector.e57";
    
    if (!QFile::exists(testFile)) {
        GTEST_SKIP() << "Test file " << testFile.toStdString() << " not found";
        return;
    }

    // Parse the malformed file
    std::vector<float> points = parser->parse(testFile);
    EXPECT_TRUE(points.empty());

    QString error = parser->getLastError();
    EXPECT_FALSE(error.isEmpty());

    qDebug() << "=== Sprint 1.2 Error Reporting Quality Test ===";
    qDebug() << "Error message:" << error;

    // Sprint 1.2 Acceptance Criteria: Detailed error messages
    // Should include context about what went wrong
    bool hasContext = error.contains("CompressedVector") || 
                     error.contains("recordCount") || 
                     error.contains("coordinates") ||
                     error.contains("prototype") ||
                     error.contains("codecs");
    
    EXPECT_TRUE(hasContext) << "Error should include context about the parsing failure";

    // Should include error code for categorization
    bool hasErrorCode = error.contains("E57_ERROR_");
    EXPECT_TRUE(hasErrorCode) << "Error should include error code for categorization";

    qDebug() << "Error context check:" << hasContext;
    qDebug() << "Error code check:" << hasErrorCode;
}

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
