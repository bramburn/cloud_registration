#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QSignalSpy>
#include "e57parser.h"

class Sprint12CompressedVectorTest : public ::testing::Test
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

// Sprint 1.2: Test CompressedVector parsing with valid XML
TEST_F(Sprint12CompressedVectorTest, ValidCompressedVectorParsing)
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

// Sprint 1.2: Test CompressedVector with missing recordCount (should fail with detailed error)
TEST_F(Sprint12CompressedVectorTest, CompressedVectorMissingRecordCount)
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
    if (!result) {
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("recordCount"));
        EXPECT_TRUE(error.contains("E57_ERROR_MISSING_RECORDCOUNT"));
        qDebug() << "Expected error message:" << error;
    }
}

// Sprint 1.2: Test CompressedVector with missing codecs (should fail with detailed error)
TEST_F(Sprint12CompressedVectorTest, CompressedVectorMissingCodecs)
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
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("codecs"));
    EXPECT_TRUE(error.contains("E57_ERROR_BAD_CODECS"));
    qDebug() << "Expected error message:" << error;
}

// Sprint 1.2: Test CompressedVector with invalid recordCount value
TEST_F(Sprint12CompressedVectorTest, CompressedVectorInvalidRecordCount)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <CompressedVectorNode recordCount="not_a_number" fileOffset="2048">
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
    if (!result) {
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("not_a_number"));
        EXPECT_TRUE(error.contains("E57_ERROR_INVALID_RECORDCOUNT"));
        qDebug() << "Expected error message:" << error;
    }
}

// Sprint 1.2: Test CompressedVector with missing XYZ coordinates
TEST_F(Sprint12CompressedVectorTest, CompressedVectorMissingCoordinates)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <CompressedVectorNode recordCount="1000" fileOffset="2048">
                    <prototype>
                        <cartesianX type="Float" precision="single"/>
                        <!-- Missing cartesianY and cartesianZ -->
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
    if (!result) {
        QString error = parser->getLastError();
        EXPECT_TRUE(error.contains("cartesianY") || error.contains("cartesianZ"));
        EXPECT_TRUE(error.contains("E57_ERROR_MISSING_COORDINATES"));
        qDebug() << "Expected error message:" << error;
    }
}

// Sprint 1.2: Test enhanced error reporting with element context
TEST_F(Sprint12CompressedVectorTest, DetailedErrorReporting)
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
    qDebug() << "Detailed error message:" << error;
}

// Sprint 1.2: Test CompressedVector with alternative VectorNode naming
TEST_F(Sprint12CompressedVectorTest, CompressedVectorAlternativeNaming)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <VectorNode recordCount="500" fileOffset="1024">
                    <prototype>
                        <cartesianX type="Float" precision="single"/>
                        <cartesianY type="Float" precision="single"/>
                        <cartesianZ type="Float" precision="single"/>
                    </prototype>
                </VectorNode>
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_TRUE(result);
    if (!result) {
        qDebug() << "Alternative VectorNode parsing failed:" << parser->getLastError();
    }
}

// Sprint 1.2: Test error handling for missing VectorNode
TEST_F(Sprint12CompressedVectorTest, CompressedVectorMissingVectorNode)
{
    QString testXml = R"(
        <points type="CompressedVector">
            <codecs>
                <!-- No CompressedVectorNode or VectorNode -->
            </codecs>
        </points>
    )";

    QDomDocument doc;
    ASSERT_TRUE(doc.setContent(testXml));

    QDomElement pointsElement = doc.documentElement();
    bool result = parser->parseData3D(pointsElement);

    EXPECT_FALSE(result);
    QString error = parser->getLastError();
    EXPECT_TRUE(error.contains("CompressedVectorNode") || error.contains("VectorNode"));
    EXPECT_TRUE(error.contains("E57_ERROR_MISSING_VECTORNODE"));
    qDebug() << "Expected error message:" << error;
}

// Main function for running tests
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
