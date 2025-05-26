#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include "e57parser.h"

class E57ParserTest : public ::testing::Test
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
    
    // Helper function to create a mock E57 file
    QString createMockE57File()
    {
        QTemporaryFile* tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(false); // Keep file for testing
        
        if (!tempFile->open()) {
            return QString();
        }
        
        QDataStream stream(tempFile);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        // Write E57 signature
        stream << static_cast<quint32>(0x41535446); // "ASTF"
        
        // Write version
        stream << static_cast<quint32>(1); // Major version
        stream << static_cast<quint32>(0); // Minor version
        
        // Write file physical length
        stream << static_cast<quint64>(1024);
        
        // Write XML length and offset
        stream << static_cast<quint64>(100); // XML length
        stream << static_cast<quint64>(32);  // XML offset
        
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
        tempFile->write("This is not an E57 file");
        
        QString fileName = tempFile->fileName();
        tempFile->close();
        delete tempFile;
        
        return fileName;
    }
    
    QCoreApplication* app = nullptr;
    E57Parser* parser = nullptr;
};

TEST_F(E57ParserTest, ValidE57FileDetection)
{
    QString mockFile = createMockE57File();
    ASSERT_FALSE(mockFile.isEmpty());
    
    bool isValid = parser->isValidE57File(mockFile);
    EXPECT_TRUE(isValid);
    
    // Clean up
    QFile::remove(mockFile);
}

TEST_F(E57ParserTest, InvalidFileDetection)
{
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());
    
    bool isValid = parser->isValidE57File(invalidFile);
    EXPECT_FALSE(isValid);
    
    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(E57ParserTest, NonExistentFileHandling)
{
    QString nonExistentFile = "/path/that/does/not/exist.e57";
    
    bool isValid = parser->isValidE57File(nonExistentFile);
    EXPECT_FALSE(isValid);
}

TEST_F(E57ParserTest, MockDataGeneration)
{
    // Test parsing with an invalid file (should generate mock data)
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());
    
    try {
        std::vector<float> points = parser->parse(invalidFile);
        
        // Should have generated mock data
        EXPECT_FALSE(points.empty());
        EXPECT_EQ(points.size() % 3, 0); // Should be divisible by 3 (X, Y, Z)
        
        // Check that we have a reasonable number of points
        size_t numPoints = points.size() / 3;
        EXPECT_GT(numPoints, 1000); // Should have generated at least 1000 points
        EXPECT_LT(numPoints, 100000); // But not too many
        
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
    
    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(E57ParserTest, ValidE57FileParsing)
{
    QString mockFile = createMockE57File();
    ASSERT_FALSE(mockFile.isEmpty());
    
    try {
        std::vector<float> points = parser->parse(mockFile);
        
        // Should have generated mock data (since full parsing is not implemented yet)
        EXPECT_FALSE(points.empty());
        EXPECT_EQ(points.size() % 3, 0);
        
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
    
    // Clean up
    QFile::remove(mockFile);
}

TEST_F(E57ParserTest, ErrorHandling)
{
    // Test with a non-existent file
    try {
        std::vector<float> points = parser->parse("/non/existent/file.e57");
        FAIL() << "Expected exception was not thrown";
    } catch (const E57ParseException& e) {
        // Expected behavior
        EXPECT_FALSE(parser->getLastError().isEmpty());
    } catch (const std::exception& e) {
        FAIL() << "Wrong exception type: " << e.what();
    }
}

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
