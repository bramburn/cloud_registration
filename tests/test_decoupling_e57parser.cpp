#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QCoreApplication>
#include <QWidget>
#include <QTimer>
#include <memory>
#include "../src/IE57Parser.h"
#include "../src/e57parserlib.h"
#include "../src/mainwindow.h"

/**
 * @brief Mock implementation of IE57Parser for testing
 * 
 * This mock parser allows us to test the MainWindow and other components
 * independently of the actual E57 parsing logic, demonstrating the value
 * of the decoupling effort.
 */
class MockE57Parser : public IE57Parser {
    Q_OBJECT

public:
    explicit MockE57Parser(QObject *parent = nullptr) : IE57Parser(parent) {}

    // Mock data for testing
    bool m_isOpen = false;
    QString m_lastError;
    std::vector<float> m_mockPoints;
    bool m_shouldSucceed = true;
    int m_mockScanCount = 1;
    QString m_mockGuid = "test-guid-12345";
    std::pair<int, int> m_mockVersion = {1, 0};

    // Interface implementation
    void startParsing(const QString& filePath, const LoadingSettings& settings = LoadingSettings()) override {
        Q_UNUSED(filePath)
        Q_UNUSED(settings)
        
        // Simulate parsing with a timer
        QTimer::singleShot(100, this, [this]() {
            emit progressUpdated(50, "Parsing mock data");
            
            QTimer::singleShot(100, this, [this]() {
                emit progressUpdated(100, "Parsing complete");
                
                if (m_shouldSucceed) {
                    emit parsingFinished(true, "Mock parsing successful", m_mockPoints);
                } else {
                    emit parsingFinished(false, "Mock parsing failed", std::vector<float>());
                }
            });
        });
    }

    void cancelParsing() override {
        emit parsingFinished(false, "Parsing cancelled", std::vector<float>());
    }

    QString getLastError() const override {
        return m_lastError;
    }

    bool isValidE57File(const QString& filePath) override {
        Q_UNUSED(filePath)
        return m_shouldSucceed;
    }

    int getScanCount(const QString& filePath) override {
        Q_UNUSED(filePath)
        return m_mockScanCount;
    }

    bool openFile(const std::string& filePath) override {
        Q_UNUSED(filePath)
        m_isOpen = m_shouldSucceed;
        if (!m_shouldSucceed) {
            m_lastError = "Mock file open failed";
        }
        return m_isOpen;
    }

    void closeFile() override {
        m_isOpen = false;
        m_lastError.clear();
    }

    bool isOpen() const override {
        return m_isOpen;
    }

    std::string getGuid() const override {
        return m_mockGuid.toStdString();
    }

    std::pair<int, int> getVersion() const override {
        return m_mockVersion;
    }

    int getScanCount() const override {
        return m_mockScanCount;
    }

    ScanMetadata getScanMetadata(int scanIndex) const override {
        ScanMetadata metadata;
        metadata.index = scanIndex;
        metadata.name = QString("Mock Scan %1").arg(scanIndex).toStdString();
        metadata.guid = QString("scan-guid-%1").arg(scanIndex).toStdString();
        metadata.pointCount = m_mockPoints.size() / 3;
        metadata.isLoaded = true;
        metadata.hasIntensity = false;
        metadata.hasColor = false;
        return metadata;
    }

    std::vector<float> extractPointData() override {
        return m_mockPoints;
    }

    std::vector<float> extractPointData(int scanIndex) override {
        Q_UNUSED(scanIndex)
        return m_mockPoints;
    }

    std::vector<PointData> extractEnhancedPointData(int scanIndex = 0) override {
        Q_UNUSED(scanIndex)
        std::vector<PointData> points;
        for (size_t i = 0; i < m_mockPoints.size(); i += 3) {
            PointData point;
            point.x = m_mockPoints[i];
            point.y = m_mockPoints[i + 1];
            point.z = m_mockPoints[i + 2];
            point.isValid = true;
            points.push_back(point);
        }
        return points;
    }

    int64_t getPointCount(int scanIndex = 0) const override {
        Q_UNUSED(scanIndex)
        return m_mockPoints.size() / 3;
    }

    // Helper methods for testing
    void setMockPoints(const std::vector<float>& points) {
        m_mockPoints = points;
    }

    void setShouldSucceed(bool succeed) {
        m_shouldSucceed = succeed;
    }

    void setMockScanCount(int count) {
        m_mockScanCount = count;
    }
};

/**
 * @brief Test fixture for E57 parser decoupling tests
 */
class E57ParserDecouplingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock parser with test data
        mockParser = new MockE57Parser();
        
        // Set up mock data - a simple cube
        std::vector<float> cubePoints = {
            0.0f, 0.0f, 0.0f,  // Point 1
            1.0f, 0.0f, 0.0f,  // Point 2
            1.0f, 1.0f, 0.0f,  // Point 3
            0.0f, 1.0f, 0.0f,  // Point 4
            0.0f, 0.0f, 1.0f,  // Point 5
            1.0f, 0.0f, 1.0f,  // Point 6
            1.0f, 1.0f, 1.0f,  // Point 7
            0.0f, 1.0f, 1.0f   // Point 8
        };
        mockParser->setMockPoints(cubePoints);
    }
    
    void TearDown() override {
        delete mockParser;
    }
    
    MockE57Parser* mockParser = nullptr;
};

// Test Case 1: Interface Polymorphism
TEST_F(E57ParserDecouplingTest, InterfacePolymorphism) {
    // Test that E57ParserLib can be used polymorphically through IE57Parser
    std::unique_ptr<IE57Parser> parser(new E57ParserLib());
    
    EXPECT_FALSE(parser->isOpen());
    EXPECT_TRUE(parser->getLastError().isEmpty());
    EXPECT_EQ(parser->getScanCount(), 0);
}

// Test Case 2: Mock Parser Basic Functionality
TEST_F(E57ParserDecouplingTest, MockParserBasicFunctionality) {
    EXPECT_FALSE(mockParser->isOpen());
    EXPECT_TRUE(mockParser->getLastError().isEmpty());
    
    // Test file operations
    EXPECT_TRUE(mockParser->openFile("mock_file.e57"));
    EXPECT_TRUE(mockParser->isOpen());
    EXPECT_EQ(mockParser->getScanCount(), 1);
    EXPECT_EQ(mockParser->getGuid(), "test-guid-12345");
    EXPECT_EQ(mockParser->getVersion(), std::make_pair(1, 0));
    
    // Test point data extraction
    auto points = mockParser->extractPointData();
    EXPECT_EQ(points.size(), 24); // 8 points * 3 coordinates
    EXPECT_EQ(mockParser->getPointCount(), 8);
    
    mockParser->closeFile();
    EXPECT_FALSE(mockParser->isOpen());
}

// Test Case 3: Mock Parser Signal Emission
TEST_F(E57ParserDecouplingTest, MockParserSignalEmission) {
    QSignalSpy progressSpy(mockParser, &IE57Parser::progressUpdated);
    QSignalSpy finishedSpy(mockParser, &IE57Parser::parsingFinished);
    
    mockParser->startParsing("test_file.e57");

    // Wait for signals to be emitted
    EXPECT_TRUE(finishedSpy.wait(1000)); // Wait up to 1 second for finished signal

    // Verify progress signals
    EXPECT_GE(progressSpy.count(), 1);
    
    // Verify finished signal
    EXPECT_EQ(finishedSpy.count(), 1);
    
    if (finishedSpy.count() > 0) {
        QList<QVariant> arguments = finishedSpy.at(0);
        bool success = arguments[0].toBool();
        QString message = arguments[1].toString();
        
        EXPECT_TRUE(success);
        EXPECT_EQ(message, "Mock parsing successful");
    }
}

// Test Case 4: Mock Parser Error Handling
TEST_F(E57ParserDecouplingTest, MockParserErrorHandling) {
    mockParser->setShouldSucceed(false);
    
    EXPECT_FALSE(mockParser->openFile("invalid_file.e57"));
    EXPECT_FALSE(mockParser->isOpen());
    EXPECT_FALSE(mockParser->getLastError().isEmpty());
    
    QSignalSpy finishedSpy(mockParser, &IE57Parser::parsingFinished);
    mockParser->startParsing("invalid_file.e57");

    EXPECT_TRUE(finishedSpy.wait(1000)); // Wait up to 1 second for finished signal

    EXPECT_EQ(finishedSpy.count(), 1);
    if (finishedSpy.count() > 0) {
        QList<QVariant> arguments = finishedSpy.at(0);
        bool success = arguments[0].toBool();
        EXPECT_FALSE(success);
    }
}

// Test Case 5: MainWindow Dependency Injection
TEST_F(E57ParserDecouplingTest, MainWindowDependencyInjection) {
    // Test that MainWindow can accept an injected parser
    std::unique_ptr<MainWindow> window(new MainWindow(mockParser));
    
    EXPECT_NE(window.get(), nullptr);
    // The mock parser should now be owned by the MainWindow
    // Note: We don't delete mockParser in TearDown when this test runs
}

#include "test_decoupling_e57parser.moc"
