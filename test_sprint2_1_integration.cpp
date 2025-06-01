#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTimer>
#include "src/e57parserlib.h"

class Sprint21IntegrationTest : public QObject
{
    Q_OBJECT

public:
    Sprint21IntegrationTest() = default;
    
    void runTests()
    {
        qDebug() << "=== Sprint 2.1 Integration Test ===";
        
        // Test 1: Load E57 file with bitPackCodec
        testBitPackCodecFile();
        
        // Test 2: Load E57 file with unsupported codec
        testUnsupportedCodecFile();
        
        qDebug() << "\n=== Sprint 2.1 Integration Test Complete ===";
        QCoreApplication::quit();
    }

private slots:
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points)
    {
        m_lastSuccess = success;
        m_lastMessage = message;
        m_lastPoints = points;
        m_parsingComplete = true;
        
        qDebug() << "Parsing finished - Success:" << success;
        qDebug() << "Message:" << message;
        qDebug() << "Points count:" << (points.size() / 3);
        
        if (success && !points.empty()) {
            qDebug() << "Sample points:";
            for (size_t i = 0; i < points.size() && i < 9; i += 3) {
                qDebug() << "  Point" << (i/3 + 1) << ":" << points[i] << points[i+1] << points[i+2];
            }
        }
    }

private:
    void testBitPackCodecFile()
    {
        qDebug() << "\n--- Test 1: BitPack Codec File Loading ---";
        
        QString testFile = "test_data/e57_bitpack_codec_test_fixed.e57";
        QFileInfo fileInfo(testFile);
        
        if (!fileInfo.exists()) {
            qDebug() << "Test file not found:" << testFile;
            qDebug() << "Test 1 Result: SKIP (file not found)";
            return;
        }
        
        E57Parser parser;
        connect(&parser, &E57Parser::parsingFinished,
                this, &Sprint21IntegrationTest::onParsingFinished);
        
        m_parsingComplete = false;
        parser.startParsing(testFile);
        
        // Wait for parsing to complete
        QTimer timer;
        timer.setSingleShot(true);
        timer.start(5000); // 5 second timeout
        
        while (!m_parsingComplete && timer.isActive()) {
            QCoreApplication::processEvents();
        }
        
        if (!m_parsingComplete) {
            qDebug() << "Test 1 Result: FAIL (timeout)";
            return;
        }
        
        // Validate results
        bool testPassed = m_lastSuccess &&
                         m_lastPoints.size() == 9 && // 3 points * 3 coordinates
                         (m_lastMessage.contains("bitPackCodec") || m_lastMessage.contains("compression") || m_lastMessage.contains("Successfully loaded"));
        
        qDebug() << "Test 1 Result:" << (testPassed ? "PASS" : "FAIL");
        
        if (testPassed) {
            // Validate specific point values
            bool pointsCorrect = (m_lastPoints[0] == 1.0f && m_lastPoints[1] == 2.0f && m_lastPoints[2] == 3.0f &&
                                 m_lastPoints[3] == 4.0f && m_lastPoints[4] == 5.0f && m_lastPoints[5] == 6.0f &&
                                 m_lastPoints[6] == 7.0f && m_lastPoints[7] == 8.0f && m_lastPoints[8] == 9.0f);
            
            qDebug() << "Point values validation:" << (pointsCorrect ? "PASS" : "FAIL");
        }
    }
    
    void testUnsupportedCodecFile()
    {
        qDebug() << "\n--- Test 2: Unsupported Codec File Loading ---";
        
        QString testFile = "test_data/e57_unsupported_codec_test_fixed.e57";
        QFileInfo fileInfo(testFile);
        
        if (!fileInfo.exists()) {
            qDebug() << "Test file not found:" << testFile;
            qDebug() << "Test 2 Result: SKIP (file not found)";
            return;
        }
        
        E57Parser parser;
        connect(&parser, &E57Parser::parsingFinished,
                this, &Sprint21IntegrationTest::onParsingFinished);
        
        m_parsingComplete = false;
        parser.startParsing(testFile);
        
        // Wait for parsing to complete
        QTimer timer;
        timer.setSingleShot(true);
        timer.start(5000); // 5 second timeout
        
        while (!m_parsingComplete && timer.isActive()) {
            QCoreApplication::processEvents();
        }
        
        if (!m_parsingComplete) {
            qDebug() << "Test 2 Result: FAIL (timeout)";
            return;
        }
        
        // Validate results - should fail with unsupported codec error
        bool testPassed = !m_lastSuccess && 
                         m_lastPoints.empty() &&
                         (m_lastMessage.contains("Unsupported") || m_lastMessage.contains("codec"));
        
        qDebug() << "Test 2 Result:" << (testPassed ? "PASS" : "FAIL") << "(Should fail)";
    }

private:
    bool m_lastSuccess = false;
    QString m_lastMessage;
    std::vector<float> m_lastPoints;
    bool m_parsingComplete = false;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    Sprint21IntegrationTest test;
    
    // Start tests after event loop starts
    QTimer::singleShot(100, &test, &Sprint21IntegrationTest::runTests);
    
    return app.exec();
}

#include "test_sprint2_1_integration.moc"
