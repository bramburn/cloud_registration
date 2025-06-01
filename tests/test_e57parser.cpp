#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include <QSignalSpy>
#include <QDomDocument>
#include <QDebug>
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

TEST_F(E57ParserTest, InvalidFileNoMockData)
{
    // Test parsing with an invalid file (should NOT generate mock data in Sprint 1.1)
    QString invalidFile = createInvalidFile();
    ASSERT_FALSE(invalidFile.isEmpty());

    // Set up signal spy to capture parsing results
    QSignalSpy spy(parser, &E57Parser::parsingFinished);

    std::vector<float> points = parser->parse(invalidFile);

    // Should NOT have generated mock data - should return empty vector
    EXPECT_TRUE(points.empty());

    // Should have error message
    EXPECT_FALSE(parser->getLastError().isEmpty());

    // Should have emitted parsingFinished with failure
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_FALSE(arguments.at(0).toBool()); // success should be false

    // Clean up
    QFile::remove(invalidFile);
}

TEST_F(E57ParserTest, ValidE57FileHeaderParsing)
{
    QString mockFile = createMockE57File();
    ASSERT_FALSE(mockFile.isEmpty());

    // Test that the file is recognized as valid E57
    bool isValid = parser->isValidE57File(mockFile);
    EXPECT_TRUE(isValid);

    // Test parsing - this will fail at XML stage since our mock file doesn't have proper XML
    // but it should NOT generate mock data
    QSignalSpy spy(parser, &E57Parser::parsingFinished);
    std::vector<float> points = parser->parse(mockFile);

    // Should NOT have generated mock data - should return empty vector due to missing XML
    EXPECT_TRUE(points.empty());

    // Should have error message about XML parsing
    EXPECT_FALSE(parser->getLastError().isEmpty());

    // Clean up
    QFile::remove(mockFile);
}

TEST_F(E57ParserTest, RealE57FileTest)
{
    // Test with the real E57 test file if it exists
    QString testFile = "test_data/test_real_points.e57";

    if (QFile::exists(testFile)) {
        QSignalSpy spy(parser, &E57Parser::parsingFinished);
        std::vector<float> points = parser->parse(testFile);

        // Should have successfully parsed real E57 data
        EXPECT_FALSE(points.empty());
        EXPECT_EQ(points.size() % 3, 0); // Should be divisible by 3 (X, Y, Z)

        // Should have exactly 3 points (9 floats)
        EXPECT_EQ(points.size(), 9);

        // Should have emitted parsingFinished with success
        EXPECT_EQ(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        EXPECT_TRUE(arguments.at(0).toBool()); // success should be true

        // Verify the actual coordinates (1,2,3), (4,5,6), (7,8,9)
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
    } else {
        // Skip test if file doesn't exist
        GTEST_SKIP() << "Test file " << testFile.toStdString() << " not found";
    }
}

TEST_F(E57ParserTest, ErrorHandling)
{
    // Test with a non-existent file - should return empty vector, not throw exception
    std::vector<float> points = parser->parse("/non/existent/file.e57");

    // Should return empty vector
    EXPECT_TRUE(points.empty());

    // Should have error message
    EXPECT_FALSE(parser->getLastError().isEmpty());
}

// Sprint 1.2: CompressedVector parsing tests
TEST_F(E57ParserTest, CompressedVectorParsing)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <CompressedVectorNode recordCount="1000" fileOffset="2048">
                    <prototype>
                        <cartesianX type="Float" precision="single"/>
                        <cartesianY type="Float" precision="single"/>
                        <cartesianZ type="Float" precision="single"/>
                    </prototype>
                </CompressedVectorNode>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_TRUE(result);
    if (!result) {
        qDebug() << "CompressedVector parsing failed:" << parser->getLastError();
    }
}

TEST_F(E57ParserTest, CompressedVectorMissingRecordCount)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <CompressedVectorNode fileOffset="2048">
                    <prototype>
                        <cartesianX type="Float" precision="single"/>
                        <cartesianY type="Float" precision="single"/>
                        <cartesianZ type="Float" precision="single"/>
                    </prototype>
                </CompressedVectorNode>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    EXPECT_TRUE(parser->getLastError().contains("recordCount"));
    EXPECT_TRUE(parser->getLastError().contains("E57_ERROR_MISSING_RECORDCOUNT"));
}

TEST_F(E57ParserTest, CompressedVectorMissingCodecs)
{
    QString testXml = R"(
        <points type="CompressedVector">
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    EXPECT_TRUE(parser->getLastError().contains("codecs"));
    EXPECT_TRUE(parser->getLastError().contains("E57_ERROR_BAD_CODECS"));
}

// Sprint 1.2: Enhanced error reporting tests
TEST_F(E57ParserTest, DetailedErrorReporting)
{
    QString testXml = R"(
        <points type="Vector">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <!-- Missing cartesianZ -->
            </prototype>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("cartesianZ"));
    EXPECT_TRUE(error.contains("E57_ERROR_MISSING_COORDINATES"));
    EXPECT_TRUE(error.contains("prototype"));
}

TEST_F(E57ParserTest, DetailedErrorWithElementContext)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="invalid">
            <codecs>
                <CompressedVectorNode recordCount="not_a_number">
                </CompressedVectorNode>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("CompressedVectorNode"));
    EXPECT_TRUE(error.contains("not_a_number"));
}

// Sprint 2.1: Codec handling tests
TEST_F(E57ParserTest, BitPackCodecIdentificationExplicit)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <bitPackCodec/>
                </vector>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_TRUE(result);
    if (!result) {
        qDebug() << "BitPack codec identification failed:" << parser->getLastError();
    }
}

TEST_F(E57ParserTest, BitPackCodecIdentificationDefault)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <!-- Empty vector = default bitPackCodec -->
                </vector>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_TRUE(result);
    if (!result) {
        qDebug() << "Default BitPack codec identification failed:" << parser->getLastError();
    }
}

TEST_F(E57ParserTest, UnsupportedCodecRejection)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <zLibCodec/>
                </vector>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("Unsupported E57 compression codec") ||
                error.contains("zLibCodec"));
}

TEST_F(E57ParserTest, FieldDescriptorParsing)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="50">
            <prototype>
                <cartesianX type="Float" precision="single" minimum="-10.0" maximum="10.0"/>
                <cartesianY type="Float" precision="double" minimum="-5.0" maximum="5.0"/>
                <cartesianZ type="ScaledInteger" precision="16" scale="0.001" offset="100.0"/>
            </prototype>
            <codecs>
                <vector>
                    <bitPackCodec/>
                </vector>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_TRUE(result);
    if (!result) {
        qDebug() << "Field descriptor parsing failed:" << parser->getLastError();
    }
}

TEST_F(E57ParserTest, MissingPrototypeInCompressedVector)
{
    QString testXml = R"(
        <points type="CompressedVector" recordCount="100">
            <!-- Missing prototype -->
            <codecs>
                <vector>
                    <bitPackCodec/>
                </vector>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("prototype") || error.contains("E57_ERROR_MISSING_PROTOTYPE"));
}

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
