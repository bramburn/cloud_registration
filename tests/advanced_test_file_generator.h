#ifndef ADVANCED_TEST_FILE_GENERATOR_H
#define ADVANCED_TEST_FILE_GENERATOR_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QRandomGenerator>
#include <vector>

/**
 * @brief Advanced Test File Generator for Sprint 2.4
 * 
 * Generates complex test files for comprehensive testing scenarios including:
 * - Very large point clouds (20M+ points)
 * - Multi-scan E57 files
 * - Extreme coordinate ranges
 * - Files with many VLRs
 * - Corrupted files for error handling tests
 * 
 * Implements Task 2.4.1.1 from Sprint 2.4 requirements.
 */
class AdvancedTestFileGenerator : public QObject
{
    Q_OBJECT

public:
    enum class TestScenario {
        VeryLargePointCloud,     // 20M+ points
        MultipleDataSections,    // E57 with multiple data3D
        ExtremeCoordinates,      // Unusual scale/offset values
        ManyVLRs,               // LAS files with numerous VLRs
        CorruptedHeaders,       // Intentionally malformed files
        MemoryStressTest,       // Files approaching memory limits
        EdgeCasePDRF           // Unusual but valid PDRF configurations
    };

    explicit AdvancedTestFileGenerator(QObject *parent = nullptr);
    
    bool generateTestFile(TestScenario scenario, const QString &outputPath);
    QJsonObject generateTestMetadata(TestScenario scenario, const QString &filePath);
    
    // Specific generators for each scenario
    bool generateVeryLargeE57(const QString &filePath, int pointCount = 25000000);
    bool generateMultiScanE57(const QString &filePath, int scanCount = 5);
    bool generateExtremeCoordinatesLAS(const QString &filePath);
    bool generateManyVLRsLAS(const QString &filePath, int vlrCount = 100);
    bool generateCorruptedE57(const QString &filePath, const QString &corruptionType);
    
    // Memory stress test generators
    bool generateMemoryStressE57(const QString &filePath);
    bool generateEdgeCasePDRFLAS(const QString &filePath);

signals:
    void generationProgress(int percentage, const QString &status);
    void generationCompleted(const QString &filePath, bool success);

private:
    struct TestFileMetadata {
        QString scenario;
        QString filePath;
        qint64 expectedFileSize;
        int expectedPointCount;
        QStringList expectedIssues;
        bool shouldLoad;
        QString description;
    };
    
    void generateRandomPointData(std::vector<float> &points, int count, 
                                double xMin, double xMax, 
                                double yMin, double yMax, 
                                double zMin, double zMax);
    
    // E57 file structure helpers
    QString generateE57XmlHeader(int pointCount, qint64 binaryOffset);
    QString generateMultiScanE57Xml(int scanCount, const QList<int> &pointCounts);
    QByteArray generateE57BinaryData(const std::vector<float> &points);
    
    // LAS file structure helpers
    QByteArray generateLASHeader(int pointCount, double xScale, double yScale, double zScale,
                                double xOffset, double yOffset, double zOffset, 
                                int vlrCount = 0);
    QByteArray generateLASVLRs(int count);
    QByteArray generateLASPointData(const std::vector<float> &points, int pdrf);
    
    // Corruption helpers
    QByteArray corruptData(const QByteArray &data, const QString &corruptionType);
    
    // Progress tracking
    void updateProgress(int percentage, const QString &status);
    
    QRandomGenerator *m_randomGenerator;
    QString m_outputDirectory;
};

#endif // ADVANCED_TEST_FILE_GENERATOR_H
