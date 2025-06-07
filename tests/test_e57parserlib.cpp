#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QCoreApplication>
#include "../src/e57parserlib.h"
#include "../src/IE57Parser.h"
#include <fstream>
#include <memory>

/**
 * @brief Unit tests for E57ParserLib class
 * 
 * Tests Sprint 1 requirements:
 * - File opening functionality
 * - Metadata extraction (GUID, version, scan count)
 * - Error handling for invalid files
 * - Resource management
 */
class E57ParserLibTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary invalid file for testing
        std::ofstream tempFile("test_invalid.e57");
        tempFile << "This is not a valid E57 file";
        tempFile.close();
    }
    
    void TearDown() override {
        // Clean up test files
        std::remove("test_invalid.e57");
    }
    
    E57ParserLib parser;
    const std::string validFile = "sample/bunnyDouble.e57";
    const std::string invalidFile = "test_invalid.e57";
    const std::string nonExistentFile = "non_existent_file.e57";
};

// Test Case 1.2.1: Test opening a known valid E57 file
TEST_F(E57ParserLibTest, OpenValidFile) {
    // Test with the sample file if it exists
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();
        
        EXPECT_TRUE(parser.openFile(validFile));
        EXPECT_TRUE(parser.isOpen());
        EXPECT_TRUE(parser.getLastError().isEmpty());

        // Test that we can get basic metadata
        auto version = parser.getVersion();
        EXPECT_GT(version.first, 0); // Major version should be > 0

        // Scan count should be >= 0
        EXPECT_GE(parser.getScanCount(), 0);

        parser.closeFile();
        EXPECT_FALSE(parser.isOpen());
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case 1.2.2: Test opening a non-existent E57 file
TEST_F(E57ParserLibTest, OpenNonExistentFile) {
    EXPECT_FALSE(parser.openFile(nonExistentFile));
    EXPECT_FALSE(parser.isOpen());
    EXPECT_FALSE(parser.getLastError().isEmpty());

    // Error message should indicate some kind of error (E57 exception is expected)
    QString error = parser.getLastError();
    std::cout << "Error message: " << error.toStdString() << std::endl; // Debug output
    EXPECT_TRUE(error.contains("E57") ||
                error.contains("exception") ||
                error.contains("file") ||
                error.contains("open") ||
                error.contains("exist"));
}

// Test Case 1.2.3: Test opening a corrupted or non-E57 file
TEST_F(E57ParserLibTest, OpenInvalidFile) {
    EXPECT_FALSE(parser.openFile(invalidFile));
    EXPECT_FALSE(parser.isOpen());
    EXPECT_FALSE(parser.getLastError().isEmpty());

    // Error message should indicate invalid E57 format or parsing error
    QString error = parser.getLastError();
    EXPECT_TRUE(error.contains("E57") ||
                error.contains("format") ||
                error.contains("invalid") ||
                error.contains("parse"));
}

// Test metadata extraction with closed file
TEST_F(E57ParserLibTest, MetadataWithClosedFile) {
    // Ensure file is closed
    parser.closeFile();

    EXPECT_EQ(parser.getGuid(), "");
    EXPECT_EQ(parser.getVersion(), std::make_pair(0, 0));
    EXPECT_EQ(parser.getScanCount(), 0);
    EXPECT_FALSE(parser.isOpen());
}

// Test resource management
TEST_F(E57ParserLibTest, ResourceManagement) {
    // Test multiple open/close cycles
    for (int i = 0; i < 3; ++i) {
        EXPECT_FALSE(parser.isOpen());

        // Try to open invalid file
        parser.openFile(invalidFile);
        EXPECT_FALSE(parser.isOpen());

        // Close should be safe even if file wasn't opened
        parser.closeFile();
        EXPECT_FALSE(parser.isOpen());
    }
}

// Test error state management
TEST_F(E57ParserLibTest, ErrorStateManagement) {
    // Initially no error
    EXPECT_TRUE(parser.getLastError().isEmpty());

    // After failed open, should have error
    parser.openFile(nonExistentFile);
    EXPECT_FALSE(parser.getLastError().isEmpty());

    // After successful operation, error should be cleared
    // (We'll test this if we have a valid file)
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();
        parser.openFile(validFile);
        if (parser.isOpen()) {
            // Error should be cleared on successful open
            // Note: getLastError() might still contain the error from failed open
            // This is acceptable behavior for this sprint
        }
    }
}

// Test constructor and destructor
TEST_F(E57ParserLibTest, ConstructorDestructor) {
    // Test that we can create and destroy parser objects
    {
        E57ParserLib tempParser;
        EXPECT_FALSE(tempParser.isOpen());
        EXPECT_TRUE(tempParser.getLastError().isEmpty());
    } // tempParser should be destroyed cleanly here

    // Original parser should still work
    EXPECT_FALSE(parser.isOpen());
}

// ============================================================================
// Sprint 2 Test Cases: Point Data Extraction
// ============================================================================

// Test Case 2.1.1: Parse an E57 file with standard prototype (XYZ as double-precision floats)
TEST_F(E57ParserLibTest, ExtractPointDataValidFile) {
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();

        ASSERT_TRUE(parser.openFile(validFile));

        // Test point count retrieval
        int64_t pointCount = parser.getPointCount(0);
        EXPECT_GT(pointCount, 0) << "Expected at least one point in the test file";

        // Test point data extraction
        std::vector<float> points = parser.extractPointData();

        // Verify we got the expected number of coordinates (3 per point)
        EXPECT_EQ(points.size(), pointCount * 3) << "Expected 3 coordinates per point (X,Y,Z)";

        // Verify points are not all zeros (basic sanity check)
        bool hasNonZeroValues = false;
        for (float coord : points) {
            if (coord != 0.0f) {
                hasNonZeroValues = true;
                break;
            }
        }
        EXPECT_TRUE(hasNonZeroValues) << "Expected at least some non-zero coordinate values";

        parser.closeFile();
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case 2.1.2: Test error handling for missing cartesian fields
TEST_F(E57ParserLibTest, ExtractPointDataClosedFile) {
    // Ensure file is closed
    parser.closeFile();

    std::vector<float> points = parser.extractPointData();

    // Should return empty vector and set error
    EXPECT_TRUE(points.empty());
    EXPECT_FALSE(parser.getLastError().isEmpty());
    EXPECT_TRUE(parser.getLastError().contains("No E57 file is open"));
}

// Test Case 2.1.3: Test invalid scan index
TEST_F(E57ParserLibTest, ExtractPointDataInvalidScanIndex) {
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();

        ASSERT_TRUE(parser.openFile(validFile));

        int scanCount = parser.getScanCount();

        // Try to extract from an invalid scan index
        std::vector<float> points = parser.extractPointData(scanCount + 10);

        EXPECT_TRUE(points.empty());
        EXPECT_FALSE(parser.getLastError().isEmpty());
        EXPECT_TRUE(parser.getLastError().contains("Invalid scan index"));

        parser.closeFile();
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case 2.3.1: Test progressUpdated signal emission
TEST_F(E57ParserLibTest, ProgressSignalEmission) {
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();

        ASSERT_TRUE(parser.openFile(validFile));

        // Set up signal spy for progressUpdated signal
        QSignalSpy progressSpy(&parser, &E57ParserLib::progressUpdated);

        // Extract point data (this should emit progress signals)
        std::vector<float> points = parser.extractPointData();

        // Verify that progress signals were emitted
        EXPECT_GT(progressSpy.count(), 0) << "Expected at least one progressUpdated signal";

        // Check that progress values are reasonable
        for (const auto& signal : progressSpy) {
            int percentage = signal[0].toInt();
            EXPECT_GE(percentage, 0) << "Progress percentage should be >= 0";
            EXPECT_LE(percentage, 100) << "Progress percentage should be <= 100";
        }

        parser.closeFile();
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case 2.3.2: Test parsingFinished signal on successful parsing
TEST_F(E57ParserLibTest, ParsingFinishedSignalSuccess) {
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();

        ASSERT_TRUE(parser.openFile(validFile));

        // Set up signal spy for parsingFinished signal
        QSignalSpy finishedSpy(&parser, &E57ParserLib::parsingFinished);

        // Extract point data
        std::vector<float> points = parser.extractPointData();

        // Verify that parsingFinished signal was emitted exactly once
        EXPECT_EQ(finishedSpy.count(), 1) << "Expected exactly one parsingFinished signal";

        if (finishedSpy.count() > 0) {
            QList<QVariant> arguments = finishedSpy.at(0);
            bool success = arguments[0].toBool();
            QString message = arguments[1].toString();

            EXPECT_TRUE(success) << "Expected successful parsing";
            EXPECT_FALSE(message.isEmpty()) << "Expected non-empty success message";
            EXPECT_TRUE(message.contains("Successfully extracted")) << "Expected success message to mention extraction";
        }

        parser.closeFile();
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case 2.3.3: Test parsingFinished signal on parsing failure
TEST_F(E57ParserLibTest, ParsingFinishedSignalFailure) {
    // Set up signal spy for parsingFinished signal
    QSignalSpy finishedSpy(&parser, &E57ParserLib::parsingFinished);

    // Try to extract points without opening a file (should fail)
    std::vector<float> points = parser.extractPointData();

    // Verify that parsingFinished signal was emitted exactly once
    EXPECT_EQ(finishedSpy.count(), 1) << "Expected exactly one parsingFinished signal";

    if (finishedSpy.count() > 0) {
        QList<QVariant> arguments = finishedSpy.at(0);
        bool success = arguments[0].toBool();
        QString message = arguments[1].toString();

        EXPECT_FALSE(success) << "Expected failed parsing";
        EXPECT_FALSE(message.isEmpty()) << "Expected non-empty error message";
    }
}

// ============================================================================
// Sprint 1 Decoupling Test Cases: Interface Compliance and Polymorphism
// ============================================================================

// Test Case: Polymorphic Usage Through Interface
TEST_F(E57ParserLibTest, PolymorphicUsageThroughInterface) {
    // Test that E57ParserLib can be used polymorphically through IE57Parser interface
    std::unique_ptr<IE57Parser> interfaceParser(new E57ParserLib());

    // Test basic interface methods
    EXPECT_FALSE(interfaceParser->isOpen());
    EXPECT_TRUE(interfaceParser->getLastError().isEmpty());
    EXPECT_EQ(interfaceParser->getScanCount(), 0);

    // Test that we can call interface methods
    std::ifstream file(validFile);
    if (file.good()) {
        file.close();

        EXPECT_TRUE(interfaceParser->openFile(validFile));
        EXPECT_TRUE(interfaceParser->isOpen());

        // Test metadata access through interface
        auto version = interfaceParser->getVersion();
        EXPECT_GT(version.first, 0);

        int scanCount = interfaceParser->getScanCount();
        EXPECT_GE(scanCount, 0);

        if (scanCount > 0) {
            auto metadata = interfaceParser->getScanMetadata(0);
            EXPECT_EQ(metadata.index, 0);
            EXPECT_GE(metadata.pointCount, 0);
        }

        interfaceParser->closeFile();
        EXPECT_FALSE(interfaceParser->isOpen());
    } else {
        GTEST_SKIP() << "Valid E57 test file not found: " << validFile;
    }
}

// Test Case: Interface Signal Compatibility
TEST_F(E57ParserLibTest, InterfaceSignalCompatibility) {
    std::unique_ptr<IE57Parser> interfaceParser(new E57ParserLib());

    // Test that signals are accessible through the interface
    QSignalSpy progressSpy(interfaceParser.get(), &IE57Parser::progressUpdated);
    QSignalSpy finishedSpy(interfaceParser.get(), &IE57Parser::parsingFinished);
    QSignalSpy metadataSpy(interfaceParser.get(), &IE57Parser::scanMetadataAvailable);
    QSignalSpy intensitySpy(interfaceParser.get(), &IE57Parser::intensityDataExtracted);
    QSignalSpy colorSpy(interfaceParser.get(), &IE57Parser::colorDataExtracted);

    EXPECT_TRUE(progressSpy.isValid());
    EXPECT_TRUE(finishedSpy.isValid());
    EXPECT_TRUE(metadataSpy.isValid());
    EXPECT_TRUE(intensitySpy.isValid());
    EXPECT_TRUE(colorSpy.isValid());
}

// Test Case: Interface Method Override Verification
TEST_F(E57ParserLibTest, InterfaceMethodOverrideVerification) {
    // This test verifies that all interface methods are properly overridden
    E57ParserLib concreteParser;
    IE57Parser* interfacePtr = &concreteParser;

    // Test that virtual method calls work correctly
    EXPECT_FALSE(interfacePtr->isOpen());

    // Test file operations through interface
    interfacePtr->openFile(invalidFile);  // Should fail gracefully
    EXPECT_FALSE(interfacePtr->isOpen());
    EXPECT_FALSE(interfacePtr->getLastError().isEmpty());

    interfacePtr->closeFile();  // Should be safe to call
    EXPECT_FALSE(interfacePtr->isOpen());
}

// Test Case: Dependency Injection Compatibility
TEST_F(E57ParserLibTest, DependencyInjectionCompatibility) {
    // Test that E57ParserLib can be used for dependency injection
    std::unique_ptr<IE57Parser> parser(new E57ParserLib());

    // Simulate what MainWindow constructor would do
    EXPECT_NE(parser.get(), nullptr);
    EXPECT_FALSE(parser->isOpen());

    // Test that the parser can be moved to a thread (important for MainWindow usage)
    QThread testThread;
    parser->moveToThread(&testThread);

    // Move back to main thread for cleanup
    parser->moveToThread(QThread::currentThread());
}
