#include <QDebug>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

// Include quality components
#include "quality/PDFReportGenerator.h"
#include "quality/QualityAssessment.h"

#include <gtest/gtest.h>

class QualityFunctionalityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for test files
        m_tempDir = new QTemporaryDir();
        ASSERT_TRUE(m_tempDir->isValid());

        // Create test point cloud data
        m_testPoints = createTestPointCloud(1000);
        ASSERT_FALSE(m_testPoints.empty());
    }

    void TearDown() override
    {
        delete m_tempDir;
    }

    std::vector<Point> createTestPointCloud(size_t numPoints)
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

    void verifyFileExists(const QString& filePath)
    {
        QFileInfo fileInfo(filePath);
        EXPECT_TRUE(fileInfo.exists()) << "File does not exist: " << filePath.toStdString();
    }

    QTemporaryDir* m_tempDir;
    std::vector<Point> m_testPoints;
};

TEST_F(QualityFunctionalityTest, QualityAssessment)
{
    QualityAssessment assessment;

    // Create test point clouds
    std::vector<Point> sourceCloud = createTestPointCloud(500);
    std::vector<Point> targetCloud = createTestPointCloud(500);
    std::vector<Point> transformedCloud = sourceCloud;  // Perfect alignment for test

    // Test quality assessment
    QualityReport report = assessment.assessRegistrationQuality(sourceCloud, targetCloud, transformedCloud);

    EXPECT_FALSE(report.metrics.qualityGrade.isEmpty());
    EXPECT_GE(report.metrics.confidenceScore, 0.0);
    EXPECT_LE(report.metrics.confidenceScore, 1.0);
    EXPECT_GE(report.metrics.rootMeanSquaredError, 0.0);
    EXPECT_FALSE(report.recommendations.isEmpty());

    // Test individual quality metrics
    QualityMetrics metrics = assessment.assessPointCloudQuality(sourceCloud);
    EXPECT_EQ(metrics.totalPoints, sourceCloud.size());

    // Test overlap calculation
    double overlap = assessment.calculateOverlapPercentage(sourceCloud, targetCloud, 0.1);
    EXPECT_GE(overlap, 0.0);
    EXPECT_LE(overlap, 100.0);
}

TEST_F(QualityFunctionalityTest, PDFReportGenerator)
{
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
    report.recommendations << "Test recommendation 1"
                           << "Test recommendation 2";

    // Test report generation
    ReportOptions options;
    options.outputPath = m_tempDir->path() + "/test_report.pdf";
    options.projectName = "PDF Test";
    options.companyName = "Test Company";
    options.operatorName = "Test Operator";

    bool success = generator.generateReport(report, options);
    EXPECT_TRUE(success);
    verifyFileExists(options.outputPath);
}

TEST_F(QualityFunctionalityTest, QualityReportWorkflow)
{
    // Create test point clouds
    std::vector<Point> sourceCloud = createTestPointCloud(300);
    std::vector<Point> targetCloud = createTestPointCloud(300);
    std::vector<Point> transformedCloud = sourceCloud;

    // Perform quality assessment
    QualityAssessment assessment;
    QualityReport report = assessment.assessRegistrationQuality(sourceCloud, targetCloud, transformedCloud);

    EXPECT_FALSE(report.metrics.qualityGrade.isEmpty());

    // Generate PDF report
    PDFReportGenerator generator;
    ReportOptions options;
    options.outputPath = m_tempDir->path() + "/workflow_report.pdf";
    options.projectName = "Workflow Test";
    options.includeCharts = true;
    options.includeRecommendations = true;

    bool success = generator.generateReport(report, options);
    EXPECT_TRUE(success);
    verifyFileExists(options.outputPath);
}
