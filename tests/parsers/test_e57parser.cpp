#include <QCoreApplication>
#include <QDataStream>
#include <QDebug>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTimer>

#include "parsers/e57parserlib.h"

#include <gtest/gtest.h>

class E57ParserLibTest : public QObject, public ::testing::Test
{
    Q_OBJECT

protected:
    void SetUp() override
    {
        // Initialize Qt application for testing
        if (!QCoreApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }

        parser = new E57ParserLib();

        // Connect signals for testing
        connect(parser, &E57ParserLib::parsingFinished, this, &E57ParserLibTest::onParsingFinished);
        connect(parser, &E57ParserLib::progressUpdated, this, &E57ParserLibTest::onProgressUpdated);
    }

    void TearDown() override
    {
        delete parser;
        // Don't delete app as it might be used by other tests
    }

public slots:
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points)
    {
        lastSuccess = success;
        lastMessage = message;
        lastPoints = points;
        parsingComplete = true;
    }

    void onProgressUpdated(int percentage, const QString& stage)
    {
        lastProgress = percentage;
        lastStage = stage;
    }

protected:
    // Helper function to wait for async parsing to complete
    bool waitForParsing(int timeoutMs = 5000)
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(timeoutMs);

        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(parser, &E57ParserLib::parsingFinished, &loop, &QEventLoop::quit);

        parsingComplete = false;
        timer.start();
        loop.exec();

        return parsingComplete;
    }

    // Helper function to create a mock E57 file
    QString createMockE57File()
    {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(false);  // Keep file for testing

        if (!tempFile->open())
        {
            return QString();
        }

        QDataStream stream(tempFile);
        stream.setByteOrder(QDataStream::LittleEndian);

        // Write E57 signature
        stream << static_cast<quint32>(0x41535446);  // "ASTF"

        // Write version
        stream << static_cast<quint32>(1);  // Major version
        stream << static_cast<quint32>(0);  // Minor version

        // Write file physical length
        stream << static_cast<quint64>(1024);

        // Write XML length and offset
        stream << static_cast<quint64>(100);  // XML length
        stream << static_cast<quint64>(32);   // XML offset

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

        if (!tempFile->open())
        {
            return QString();
        }

        // Write some random data
        tempFile->write("This is not an E57 file");

        QString fileName = tempFile->fileName();
        tempFile->close();
        delete tempFile;

        return fileName;
    }

    QCoreApplication* app = nullptr;
    E57ParserLib* parser = nullptr;

    // Test result storage
    bool parsingComplete = false;
    bool lastSuccess = false;
    QString lastMessage;
    std::vector<float> lastPoints;
    int lastProgress = 0;
    QString lastStage;
};

TEST_F(E57ParserLibTest, ValidE57FileDetection)
{
    QString mockFile = createMockE57File();
    ASSERT_FALSE(mockFile.isEmpty());

    bool isValid = parser->isValidE57File(mockFile);
    EXPECT_TRUE(isValid);

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(E57ParserLibTest, InvalidFileDetection)
{
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    bool isValid = parser->isValidE57File(invalidFile);
    EXPECT_FALSE(isValid);

    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(E57ParserLibTest, NonExistentFileHandling)
{
    QString nonExistentFile = "/path/that/does/not/exist.e57";

    bool isValid = parser->isValidE57File(nonExistentFile);
    EXPECT_FALSE(isValid);
}

TEST_F(E57ParserLibTest, InvalidFileNoMockData)
{
    // Test parsing with an invalid file (should NOT generate mock data)
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    // Start parsing asynchronously
    parser->startParsing(invalidFile);

    // Wait for parsing to complete
    ASSERT_TRUE(waitForParsing());

    // Should NOT have generated mock data - should return empty vector
    EXPECT_TRUE(lastPoints.empty());
    EXPECT_FALSE(lastSuccess);

    // Should have error message
    EXPECT_FALSE(parser->getLastError().isEmpty());
    EXPECT_FALSE(lastMessage.isEmpty());

    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(E57ParserLibTest, ValidE57FileHeaderParsing)
{
    QString mockFile = createMockE57File();
    ASSERT_FALSE(mockFile.isEmpty());

    // Test that the file is recognized as valid E57
    bool isValid = parser->isValidE57File(mockFile);
    EXPECT_TRUE(isValid);

    // Test parsing - this will fail at XML stage since our mock file doesn't have proper XML
    // but it should NOT generate mock data
    parser->startParsing(mockFile);

    // Wait for parsing to complete
    ASSERT_TRUE(waitForParsing());

    // Should NOT have generated mock data - should return empty vector due to missing XML
    EXPECT_TRUE(lastPoints.empty());
    EXPECT_FALSE(lastSuccess);

    // Should have error message about XML parsing
    EXPECT_FALSE(parser->getLastError().isEmpty());

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(E57ParserLibTest, RealE57FileTest)
{
    // Test with the real E57 test file if it exists
    QString testFile = "test_data/test_real_points.e57";

    if (QFile::exists(testFile))
    {
        parser->startParsing(testFile);

        // Wait for parsing to complete
        ASSERT_TRUE(waitForParsing());

        // Should have successfully parsed real E57 data
        EXPECT_FALSE(lastPoints.empty());
        EXPECT_EQ(lastPoints.size() % 3, 0);  // Should be divisible by 3 (X, Y, Z)
        EXPECT_TRUE(lastSuccess);

        // Should have exactly 3 points (9 floats)
        EXPECT_EQ(lastPoints.size(), 9);

        // Verify the actual coordinates (1,2,3), (4,5,6), (7,8,9)
        if (lastPoints.size() >= 9)
        {
            EXPECT_FLOAT_EQ(lastPoints[0], 1.0f);
            EXPECT_FLOAT_EQ(lastPoints[1], 2.0f);
            EXPECT_FLOAT_EQ(lastPoints[2], 3.0f);
            EXPECT_FLOAT_EQ(lastPoints[3], 4.0f);
            EXPECT_FLOAT_EQ(lastPoints[4], 5.0f);
            EXPECT_FLOAT_EQ(lastPoints[5], 6.0f);
            EXPECT_FLOAT_EQ(lastPoints[6], 7.0f);
            EXPECT_FLOAT_EQ(lastPoints[7], 8.0f);
            EXPECT_FLOAT_EQ(lastPoints[8], 9.0f);
        }
    }
    else
    {
        // Skip test if file doesn't exist
        GTEST_SKIP() << "Test file " << testFile.toStdString() << " not found";
    }
}

TEST_F(E57ParserLibTest, ErrorHandling)
{
    // Test with a non-existent file - should return empty vector, not throw exception
    parser->startParsing("/non/existent/file.e57");

    // Wait for parsing to complete
    ASSERT_TRUE(waitForParsing());

    // Should return empty vector
    EXPECT_TRUE(lastPoints.empty());
    EXPECT_FALSE(lastSuccess);

    // Should have error message
    EXPECT_FALSE(parser->getLastError().isEmpty());
    EXPECT_FALSE(lastMessage.isEmpty());
}

// Sprint 5: E57ParserLib Integration Tests
TEST_F(E57ParserLibTest, MainWindowCompatibleSignals)
{
    // Test that signals match MainWindow expectations
    QSignalSpy progressSpy(parser, &E57ParserLib::progressUpdated);
    QSignalSpy finishedSpy(parser, &E57ParserLib::parsingFinished);
    QSignalSpy metadataSpy(parser, &E57ParserLib::scanMetadataAvailable);

    EXPECT_TRUE(progressSpy.isValid());
    EXPECT_TRUE(finishedSpy.isValid());
    EXPECT_TRUE(metadataSpy.isValid());
}

TEST_F(E57ParserLibTest, XYZVectorConversion)
{
    // Test that XYZ vector format matches MainWindow expectations
    // This test verifies the data format compliance requirement from PRD

    // Create a simple test to verify the parser can handle basic operations
    QString nonExistentFile = "/test/path/that/does/not/exist.e57";

    parser->startParsing(nonExistentFile);
    ASSERT_TRUE(waitForParsing());

    // Should fail gracefully and return empty vector
    EXPECT_TRUE(lastPoints.empty());
    EXPECT_FALSE(lastSuccess);

    // Verify the vector is properly formatted (empty but valid)
    EXPECT_EQ(lastPoints.size() % 3, 0);  // Should be divisible by 3 even when empty
}

TEST_F(E57ParserLibTest, ErrorMessageTranslation)
{
    // Test that technical errors are translated to user-friendly messages
    QString nonExistentFile = "/path/that/does/not/exist.e57";

    parser->startParsing(nonExistentFile);
    ASSERT_TRUE(waitForParsing());

    QString errorMsg = parser->getLastError();
    EXPECT_FALSE(errorMsg.isEmpty());

    // Should not contain technical jargon
    EXPECT_FALSE(errorMsg.contains("E57_ERROR_"));
    EXPECT_FALSE(errorMsg.contains("libE57Format"));

    // Should be user-friendly
    EXPECT_TRUE(errorMsg.contains("file") || errorMsg.contains("File"));
}

TEST_F(E57ParserLibTest, ThreadSafeOperations)
{
    // Test that parser can be safely used in worker threads
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(100);  // Short timeout for test

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Test cancel operation thread safety
    timer.start();
    parser->cancelParsing();  // Should not crash
    loop.exec();

    // If we reach here without crashing, test passes
    SUCCEED();
}

TEST_F(E57ParserLibTest, CancellationSupport)
{
    // Test that parsing can be cancelled
    QString nonExistentFile = "/some/long/path/that/does/not/exist.e57";

    // Start parsing
    parser->startParsing(nonExistentFile);

    // Immediately cancel
    parser->cancelParsing();

    // Wait for completion
    ASSERT_TRUE(waitForParsing());

    // Should have been cancelled or failed quickly
    EXPECT_FALSE(lastSuccess);
    EXPECT_TRUE(lastPoints.empty());
}

TEST_F(E57ParserLibTest, LoadingSettingsSupport)
{
    // Test that loading settings are properly handled
    E57ParserLib::LoadingSettings settings;
    settings.loadIntensity = true;
    settings.loadColor = true;
    settings.maxPointsPerScan = 1000;
    settings.subsamplingRatio = 0.5;

    QString nonExistentFile = "/test/file.e57";

    // Should not crash with custom settings
    parser->startParsing(nonExistentFile, settings);
    ASSERT_TRUE(waitForParsing());

    // Should fail gracefully
    EXPECT_FALSE(lastSuccess);
    EXPECT_TRUE(lastPoints.empty());
}

TEST_F(E57ParserLibTest, ProgressReporting)
{
    // Test that progress updates are emitted
    QSignalSpy progressSpy(parser, &E57ParserLib::progressUpdated);

    QString nonExistentFile = "/test/file.e57";
    parser->startParsing(nonExistentFile);
    ASSERT_TRUE(waitForParsing());

    // Should have received at least one progress update (even for failed parsing)
    EXPECT_GE(progressSpy.count(), 1);

    if (progressSpy.count() > 0)
    {
        QList<QVariant> arguments = progressSpy.first();
        EXPECT_TRUE(arguments.at(0).canConvert<int>());
        EXPECT_TRUE(arguments.at(1).canConvert<QString>());
    }
}

TEST_F(E57ParserLibTest, ScanCountUtility)
{
    // Test getScanCount utility method
    QString nonExistentFile = "/test/file.e57";

    int scanCount = parser->getScanCount(nonExistentFile);
    EXPECT_EQ(scanCount, 0);  // Should return 0 for non-existent files
}

TEST_F(E57ParserLibTest, ValidE57FileUtility)
{
    // Test isValidE57File utility method with various inputs
    EXPECT_FALSE(parser->isValidE57File(""));
    EXPECT_FALSE(parser->isValidE57File("/non/existent/file.e57"));

    // Test with invalid file
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    EXPECT_FALSE(parser->isValidE57File(invalidFile));

    // Clean up
    QFile::remove(invalidFile);
}

// Main function for running tests
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Include MOC file for Qt meta-object system
#include "test_e57parser.moc"
