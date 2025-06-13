#include <QTest>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDebug>

// Include Quality components
#include "quality/QualityAssessment.h"
#include "quality/PDFReportGenerator.h"

class QualityReportingTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Quality assessment tests
    void testQualityAssessment();
    void testPDFReportGenerator();
    void testQualityReportWorkflow();

private:
    std::vector<Point> createTestPointCloud(size_t numPoints = 1000);
    void verifyFileExists(const QString& filePath);
    void verifyFileSize(const QString& filePath, qint64 minSize);
    
    QTemporaryDir* m_tempDir;
    std::vector<Point> m_testPoints;
};

void QualityReportingTest::initTestCase()
{
    qDebug() << "Initializing Quality Reporting tests...";
    
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    m_testPoints = createTestPointCloud(1000);
    QVERIFY(!m_testPoints.empty());
    
    qDebug() << "Test directory:" << m_tempDir->path();
}

void QualityReportingTest::cleanupTestCase()
{
    qDebug() << "Cleaning up Quality Reporting tests...";
    delete m_tempDir;
}

void QualityReportingTest::testQualityAssessment()
{
    qDebug() << "Testing QualityAssessment...";
    
    QualityAssessment assessment;
    
    // Create test point clouds
    std::vector<Point> sourceCloud = createTestPointCloud(500);
    std::vector<Point> targetCloud = createTestPointCloud(500);
    std::vector<Point> transformedCloud = sourceCloud; // Perfect alignment for test
    
    // Test quality assessment
    QualityReport report = assessment.assessRegistrationQuality(sourceCloud, targetCloud, transformedCloud);
    
    QVERIFY(!report.metrics.qualityGrade.isEmpty());
    QVERIFY(report.metrics.confidenceScore >= 0.0 && report.metrics.confidenceScore <= 1.0);
    QVERIFY(report.metrics.rootMeanSquaredError >= 0.0);
    QVERIFY(!report.recommendations.isEmpty());
    
    // Test individual quality metrics
    QualityMetrics metrics = assessment.assessPointCloudQuality(sourceCloud);
    QVERIFY(metrics.totalPoints == sourceCloud.size());
    
    // Test overlap calculation
    double overlap = assessment.calculateOverlapPercentage(sourceCloud, targetCloud, 0.1);
    QVERIFY(overlap >= 0.0 && overlap <= 100.0);
    
    qDebug() << "QualityAssessment test passed";
}

void QualityReportingTest::testPDFReportGenerator()
{
    qDebug() << "Testing PDFReportGenerator...";
    
    PDFReportGenerator generator;
    
    // Create test quality report
    QualityReport report;
    report.projectName = "PDF Test Project";
    report.scanName = "Test Scan";
    report.metrics.qualityGrade = "A";
    report.metrics.rootMeanSquaredError = 0.005;
    report.metrics.overlapPercentage = 85.0;
    report.metrics.confidenceScore = 0.95;
    report.summary = "Test quality assessment summary";
    report.recommendations << "Test recommendation 1" << "Test recommendation 2";
    
    // Test report generation
    ReportOptions options;
    options.outputPath = m_tempDir->path() + "/test_report.pdf";
    options.projectName = "PDF Test";
    options.companyName = "Test Company";
    options.operatorName = "Test Operator";
    
    bool success = generator.generateReport(report, options);
    QVERIFY(success);
    verifyFileExists(options.outputPath);
    
    qDebug() << "PDFReportGenerator test passed";
}

void QualityReportingTest::testQualityReportWorkflow()
{
    qDebug() << "Testing quality report workflow...";
    
    // Create test point clouds
    std::vector<Point> sourceCloud = createTestPointCloud(300);
    std::vector<Point> targetCloud = createTestPointCloud(300);
    std::vector<Point> transformedCloud = sourceCloud;
    
    // Perform quality assessment
    QualityAssessment assessment;
    QualityReport report = assessment.assessRegistrationQuality(sourceCloud, targetCloud, transformedCloud);
    
    QVERIFY(!report.metrics.qualityGrade.isEmpty());
    
    // Generate PDF report
    PDFReportGenerator generator;
    ReportOptions options;
    options.outputPath = m_tempDir->path() + "/workflow_report.pdf";
    options.projectName = "Workflow Test";
    options.includeCharts = true;
    options.includeRecommendations = true;
    
    bool success = generator.generateReport(report, options);
    QVERIFY(success);
    verifyFileExists(options.outputPath);
    
    qDebug() << "Quality report workflow test passed";
}

std::vector<Point> QualityReportingTest::createTestPointCloud(size_t numPoints)
{
    std::vector<Point> points;
    points.reserve(numPoints);
    
    for (size_t i = 0; i < numPoints; ++i) {
        Point point;
        point.x = static_cast<float>(i % 100);
        point.y = static_cast<float>((i / 100) % 100);
        point.z = static_cast<float>(i % 10);
        point.r = static_cast<uint8_t>(i % 256);
        point.g = static_cast<uint8_t>((i * 2) % 256);
        point.b = static_cast<uint8_t>((i * 3) % 256);
        point.intensity = static_cast<float>((i % 100) / 100.0);
        
        points.push_back(point);
    }
    
    return points;
}

void QualityReportingTest::verifyFileExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.exists(), qPrintable(QString("File does not exist: %1").arg(filePath)));
}

void QualityReportingTest::verifyFileSize(const QString& filePath, qint64 minSize)
{
    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.size() >= minSize, 
             qPrintable(QString("File size too small: %1 bytes (expected >= %2)")
                       .arg(fileInfo.size()).arg(minSize)));
}

#include "test_quality_reporting.moc"
