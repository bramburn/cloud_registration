#include <QDebug>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// Include Quality components
#include "quality/PDFReportGenerator.h"
#include "quality/QualityAssessment.h"

// Include Point struct
#include "export/IFormatWriter.h"

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

    // Create test point clouds (convert Point to QualityPoint)
    std::vector<Point> sourcePoints = createTestPointCloud(500);
    std::vector<Point> targetPoints = createTestPointCloud(500);

    std::vector<QualityPoint> sourceCloud;
    std::vector<QualityPoint> targetCloud;

    for (const auto& p : sourcePoints) {
        sourceCloud.emplace_back(p.x, p.y, p.z, p.intensity);
    }
    for (const auto& p : targetPoints) {
        targetCloud.emplace_back(p.x, p.y, p.z, p.intensity);
    }

    // Create correspondences for testing
    std::vector<QualityCorrespondence> correspondences;
    for (size_t i = 0; i < std::min(sourceCloud.size(), targetCloud.size()); ++i) {
        correspondences.emplace_back(sourceCloud[i].toVector3D(), targetCloud[i].toVector3D(), 1.0f);
    }

    // Test quality assessment
    QMatrix4x4 identity;
    identity.setToIdentity();
    QualityReport report = assessment.assessRegistration(sourceCloud, targetCloud, identity, correspondences);

    QVERIFY(report.metrics.qualityGrade != 'F');
    QVERIFY(report.metrics.confidenceScore >= 0.0 && report.metrics.confidenceScore <= 1.0);
    QVERIFY(report.metrics.rmsError >= 0.0);
    QVERIFY(!report.recommendations.isEmpty());

    // Test overlap calculation
    float overlap = assessment.calculateOverlapPercentage(sourceCloud, targetCloud, 0.1f);
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
    report.description = "Test quality assessment summary";
    report.metrics.qualityGrade = 'A';
    report.metrics.rmsError = 0.005f;
    report.metrics.overlapPercentage = 85.0f;
    report.metrics.confidenceScore = 0.95f;
    report.recommendations << "Test recommendation 1"
                           << "Test recommendation 2";

    // Test report generation with new API
    PDFReportGenerator::ReportOptions options;
    options.companyName = "Test Company";
    options.reportTitle = "PDF Test Report";
    options.includeCharts = true;
    options.includeRecommendations = true;

    QString outputPath = m_tempDir->path() + "/test_report.pdf";
    bool success = generator.generatePdfReport(report, outputPath, options);
    QVERIFY(success);
    verifyFileExists(outputPath);

    qDebug() << "PDFReportGenerator test passed";
}

void QualityReportingTest::testQualityReportWorkflow()
{
    qDebug() << "Testing quality report workflow...";

    // Create test point clouds (convert Point to QualityPoint)
    std::vector<Point> sourcePoints = createTestPointCloud(300);
    std::vector<Point> targetPoints = createTestPointCloud(300);

    std::vector<QualityPoint> sourceCloud;
    std::vector<QualityPoint> targetCloud;

    for (const auto& p : sourcePoints) {
        sourceCloud.emplace_back(p.x, p.y, p.z, p.intensity);
    }
    for (const auto& p : targetPoints) {
        targetCloud.emplace_back(p.x, p.y, p.z, p.intensity);
    }

    // Create correspondences for testing
    std::vector<QualityCorrespondence> correspondences;
    for (size_t i = 0; i < std::min(sourceCloud.size(), targetCloud.size()); ++i) {
        correspondences.emplace_back(sourceCloud[i].toVector3D(), targetCloud[i].toVector3D(), 1.0f);
    }

    // Perform quality assessment
    QualityAssessment assessment;
    QMatrix4x4 identity;
    identity.setToIdentity();
    QualityReport report = assessment.assessRegistration(sourceCloud, targetCloud, identity, correspondences);

    QVERIFY(report.metrics.qualityGrade != 'F');

    // Generate PDF report
    PDFReportGenerator generator;
    PDFReportGenerator::ReportOptions options;
    options.companyName = "Workflow Test Company";
    options.includeCharts = true;
    options.includeRecommendations = true;

    QString outputPath = m_tempDir->path() + "/workflow_report.pdf";
    bool success = generator.generatePdfReport(report, outputPath, options);
    QVERIFY(success);
    verifyFileExists(outputPath);

    qDebug() << "Quality report workflow test passed";
}

std::vector<Point> QualityReportingTest::createTestPointCloud(size_t numPoints)
{
    std::vector<Point> points;
    points.reserve(numPoints);

    for (size_t i = 0; i < numPoints; ++i)
    {
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
             qPrintable(QString("File size too small: %1 bytes (expected >= %2)").arg(fileInfo.size()).arg(minSize)));
}

#include "test_quality_reporting.moc"
