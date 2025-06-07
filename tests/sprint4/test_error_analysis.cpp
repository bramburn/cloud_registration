#include <QtTest/QtTest>
#include <QVector3D>
#include <QMatrix4x4>
#include <QList>
#include <QPair>
#include <cmath>

// Include the class under test
#include "../../src/registration/ErrorAnalysis.h"

/**
 * @brief Unit tests for ErrorAnalysis class
 * 
 * Tests error calculation, statistical analysis, and quality assessment
 * functionality for point cloud alignment validation.
 */
class TestErrorAnalysis : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Core functionality tests
    void testRMSErrorCalculation();
    void testErrorStatistics();
    void testIndividualErrors();
    void testOutlierDetection();

    // Quality assessment tests
    void testQualityThresholds();
    void testQualityLevels();
    void testErrorReporting();

    // Transformation validation tests
    void testTransformationValidation();
    void testConditionNumber();

    // Edge case tests
    void testEmptyCorrespondences();
    void testPerfectAlignment();
    void testLargeErrors();

private:
    // Helper methods
    QList<QPair<QVector3D, QVector3D>> createCorrespondencesWithKnownError(
        const QList<QVector3D>& sourcePoints,
        const QMatrix4x4& transform,
        float errorMagnitude);
    
    QMatrix4x4 createTestTransformation();
    bool isFloatClose(float a, float b, float tolerance = 1e-3f);
    
    // Test data
    QList<QVector3D> m_testPoints;
    QMatrix4x4 m_testTransform;
    static constexpr float TOLERANCE = 1e-3f;
};

void TestErrorAnalysis::initTestCase()
{
    qDebug() << "Starting ErrorAnalysis tests";
    
    // Create test points
    m_testPoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f),
        QVector3D(1.0f, 1.0f, 1.0f)
    };
    
    // Create test transformation
    m_testTransform = createTestTransformation();
}

void TestErrorAnalysis::cleanupTestCase()
{
    qDebug() << "ErrorAnalysis tests completed";
}

void TestErrorAnalysis::testRMSErrorCalculation()
{
    qDebug() << "Testing RMS error calculation";
    
    // Create correspondences with known error
    float knownError = 2.0f; // 2mm error
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createCorrespondencesWithKnownError(m_testPoints, m_testTransform, knownError);
    
    // Calculate RMS error
    float rmsError = ErrorAnalysis::calculateRMSError(correspondences, m_testTransform);
    
    // Should be close to known error
    QVERIFY2(isFloatClose(rmsError, knownError, 0.1f), 
             QString("RMS error should be close to known error: %1 vs %2")
             .arg(rmsError).arg(knownError).toLocal8Bit());
}

void TestErrorAnalysis::testErrorStatistics()
{
    qDebug() << "Testing comprehensive error statistics";
    
    // Create correspondences with varying errors
    QList<QPair<QVector3D, QVector3D>> correspondences;
    QList<float> knownErrors = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f}; // mm
    
    for (int i = 0; i < m_testPoints.size() && i < knownErrors.size(); ++i) {
        QVector3D source = m_testPoints[i];
        QVector3D target = m_testTransform.map(source);
        
        // Add known error in X direction
        target.setX(target.x() + knownErrors[i]);
        
        correspondences.append(qMakePair(source, target));
    }
    
    // Calculate statistics
    ErrorAnalysis::ErrorStatistics stats = 
        ErrorAnalysis::calculateErrorStatistics(correspondences, m_testTransform);
    
    // Verify statistics
    QVERIFY2(stats.numCorrespondences == correspondences.size(), 
             "Correspondence count should match");
    
    QVERIFY2(stats.minError >= 0.0f, "Minimum error should be non-negative");
    QVERIFY2(stats.maxError >= stats.minError, "Maximum error should be >= minimum");
    QVERIFY2(stats.meanError >= stats.minError && stats.meanError <= stats.maxError, 
             "Mean error should be between min and max");
    QVERIFY2(stats.rmsError >= stats.meanError, "RMS error should be >= mean error");
    QVERIFY2(stats.standardDeviation >= 0.0f, "Standard deviation should be non-negative");
    
    qDebug() << "Statistics:" << stats.generateReport();
}

void TestErrorAnalysis::testIndividualErrors()
{
    qDebug() << "Testing individual error calculation";
    
    // Create correspondences with known individual errors
    QList<QPair<QVector3D, QVector3D>> correspondences;
    QList<float> expectedErrors = {1.0f, 2.0f, 3.0f};
    
    for (int i = 0; i < 3; ++i) {
        QVector3D source = m_testPoints[i];
        QVector3D target = m_testTransform.map(source);
        target.setX(target.x() + expectedErrors[i]); // Add error in X
        correspondences.append(qMakePair(source, target));
    }
    
    // Calculate individual errors
    QList<float> calculatedErrors = 
        ErrorAnalysis::calculateIndividualErrors(correspondences, m_testTransform);
    
    QVERIFY2(calculatedErrors.size() == expectedErrors.size(), 
             "Should return same number of errors as correspondences");
    
    for (int i = 0; i < expectedErrors.size(); ++i) {
        QVERIFY2(isFloatClose(calculatedErrors[i], expectedErrors[i], 0.1f),
                 QString("Individual error %1 should match expected: %2 vs %3")
                 .arg(i).arg(calculatedErrors[i]).arg(expectedErrors[i]).toLocal8Bit());
    }
}

void TestErrorAnalysis::testOutlierDetection()
{
    qDebug() << "Testing outlier detection";
    
    // Create correspondences with one clear outlier
    QList<QPair<QVector3D, QVector3D>> correspondences;
    
    // Normal correspondences with small errors
    for (int i = 0; i < 4; ++i) {
        QVector3D source = m_testPoints[i];
        QVector3D target = m_testTransform.map(source);
        target.setX(target.x() + 1.0f); // 1mm error
        correspondences.append(qMakePair(source, target));
    }
    
    // Add outlier with large error
    QVector3D outlierSource = m_testPoints[4];
    QVector3D outlierTarget = m_testTransform.map(outlierSource);
    outlierTarget.setX(outlierTarget.x() + 10.0f); // 10mm error (outlier)
    correspondences.append(qMakePair(outlierSource, outlierTarget));
    
    // Detect outliers with a lower threshold to ensure detection
    QList<int> outliers = ErrorAnalysis::identifyOutliers(correspondences, m_testTransform, 1.0f);

    QVERIFY2(outliers.size() >= 1, "Should detect at least one outlier");
    if (outliers.size() > 0) {
        QVERIFY2(outliers.contains(4), "Should identify the last correspondence as outlier");
    }
}

void TestErrorAnalysis::testQualityThresholds()
{
    qDebug() << "Testing quality threshold validation";
    
    // Create statistics with known values
    ErrorAnalysis::ErrorStatistics goodStats;
    goodStats.rmsError = 2.0f;
    goodStats.maxError = 4.0f;
    
    ErrorAnalysis::ErrorStatistics poorStats;
    poorStats.rmsError = 8.0f;
    poorStats.maxError = 15.0f;
    
    // Test quality thresholds
    QVERIFY2(goodStats.meetsQualityThresholds(5.0f, 10.0f), 
             "Good statistics should meet reasonable thresholds");
    
    QVERIFY2(!poorStats.meetsQualityThresholds(5.0f, 10.0f), 
             "Poor statistics should not meet strict thresholds");
}

void TestErrorAnalysis::testQualityLevels()
{
    qDebug() << "Testing quality level reporting";

    // Test different quality levels
    ErrorAnalysis::ErrorStatistics excellentStats;
    excellentStats.rmsError = 0.5f;
    QString excellentReport = excellentStats.generateReport();
    QVERIFY2(excellentReport.contains("Excellent"), "Should report excellent quality");

    ErrorAnalysis::ErrorStatistics poorStats;
    poorStats.rmsError = 15.0f;
    QString poorReport = poorStats.generateReport();
    QVERIFY2(poorReport.contains("Poor"), "Should report poor quality");
}

void TestErrorAnalysis::testErrorReporting()
{
    qDebug() << "Testing error report generation";

    // Create statistics with known values
    ErrorAnalysis::ErrorStatistics stats;
    stats.rmsError = 2.5f;
    stats.meanError = 2.0f;
    stats.maxError = 5.0f;
    stats.minError = 0.5f;
    stats.standardDeviation = 1.2f;
    stats.numCorrespondences = 10;

    QString report = stats.generateReport();

    // Verify report contains all expected information
    QVERIFY2(report.contains("RMS Error"), "Report should contain RMS error");
    QVERIFY2(report.contains("Mean Error"), "Report should contain mean error");
    QVERIFY2(report.contains("Max Error"), "Report should contain max error");
    QVERIFY2(report.contains("Min Error"), "Report should contain min error");
    QVERIFY2(report.contains("Std Deviation"), "Report should contain standard deviation");
    QVERIFY2(report.contains("Correspondences"), "Report should contain correspondence count");
    QVERIFY2(report.contains("2.500"), "Report should contain formatted RMS error value");
}

void TestErrorAnalysis::testTransformationValidation()
{
    qDebug() << "Testing transformation matrix validation";
    
    // Valid transformation
    QMatrix4x4 validTransform = createTestTransformation();
    QVERIFY2(ErrorAnalysis::validateTransformation(validTransform), 
             "Valid transformation should pass validation");
    
    // Invalid transformation (non-orthogonal rotation)
    QMatrix4x4 invalidTransform;
    invalidTransform.setToIdentity();
    invalidTransform(0, 0) = 2.0f; // Scale instead of rotation
    QVERIFY2(!ErrorAnalysis::validateTransformation(invalidTransform), 
             "Invalid transformation should fail validation");
    
    // Transformation with excessive translation
    QMatrix4x4 excessiveTranslation;
    excessiveTranslation.setToIdentity();
    excessiveTranslation.translate(10000.0f, 0.0f, 0.0f); // 10km translation
    QVERIFY2(!ErrorAnalysis::validateTransformation(excessiveTranslation), 
             "Excessive translation should fail validation");
}

void TestErrorAnalysis::testConditionNumber()
{
    qDebug() << "Testing condition number calculation";
    
    // Well-conditioned correspondences (spread out points)
    QList<QPair<QVector3D, QVector3D>> wellConditioned;
    for (const QVector3D& point : m_testPoints) {
        wellConditioned.append(qMakePair(point, m_testTransform.map(point)));
    }
    
    float conditionNumber = ErrorAnalysis::calculateConditionNumber(wellConditioned);
    QVERIFY2(conditionNumber < 100.0f, "Well-conditioned points should have low condition number");
    
    // Ill-conditioned correspondences (collinear points)
    QList<QPair<QVector3D, QVector3D>> illConditioned;
    for (int i = 0; i < 5; ++i) {
        QVector3D point(static_cast<float>(i), 0.0f, 0.0f); // Collinear
        illConditioned.append(qMakePair(point, m_testTransform.map(point)));
    }
    
    float badConditionNumber = ErrorAnalysis::calculateConditionNumber(illConditioned);
    QVERIFY2(badConditionNumber > conditionNumber, 
             "Ill-conditioned points should have higher condition number");
}

void TestErrorAnalysis::testEmptyCorrespondences()
{
    qDebug() << "Testing empty correspondences handling";
    
    QList<QPair<QVector3D, QVector3D>> empty;
    
    float rmsError = ErrorAnalysis::calculateRMSError(empty, m_testTransform);
    QVERIFY2(rmsError == 0.0f, "Empty correspondences should return zero RMS error");
    
    ErrorAnalysis::ErrorStatistics stats = 
        ErrorAnalysis::calculateErrorStatistics(empty, m_testTransform);
    QVERIFY2(stats.numCorrespondences == 0, "Empty correspondences should report zero count");
}

void TestErrorAnalysis::testPerfectAlignment()
{
    qDebug() << "Testing perfect alignment (zero error)";
    
    // Create perfect correspondences
    QList<QPair<QVector3D, QVector3D>> perfect;
    for (const QVector3D& point : m_testPoints) {
        perfect.append(qMakePair(point, m_testTransform.map(point)));
    }
    
    float rmsError = ErrorAnalysis::calculateRMSError(perfect, m_testTransform);
    QVERIFY2(rmsError < TOLERANCE, "Perfect alignment should have near-zero RMS error");
    
    ErrorAnalysis::ErrorStatistics stats = 
        ErrorAnalysis::calculateErrorStatistics(perfect, m_testTransform);
    QVERIFY2(stats.rmsError < TOLERANCE, "Perfect alignment should have near-zero statistics");
    QVERIFY2(stats.maxError < TOLERANCE, "Perfect alignment should have near-zero max error");
}

void TestErrorAnalysis::testLargeErrors()
{
    qDebug() << "Testing handling of large errors";
    
    // Create correspondences with very large errors
    QList<QPair<QVector3D, QVector3D>> largeErrors;
    for (const QVector3D& point : m_testPoints) {
        QVector3D target = m_testTransform.map(point);
        target += QVector3D(100.0f, 100.0f, 100.0f); // 100mm error in each direction
        largeErrors.append(qMakePair(point, target));
    }
    
    ErrorAnalysis::ErrorStatistics stats = 
        ErrorAnalysis::calculateErrorStatistics(largeErrors, m_testTransform);
    
    QVERIFY2(stats.rmsError > 100.0f, "Large errors should be reflected in statistics");
    QVERIFY2(!stats.meetsQualityThresholds(), "Large errors should not meet quality thresholds");
    
    QString report = stats.generateReport();
    QVERIFY2(report.contains("Poor"), "Large errors should result in poor quality rating");
}

// Helper method implementations
QList<QPair<QVector3D, QVector3D>> TestErrorAnalysis::createCorrespondencesWithKnownError(
    const QList<QVector3D>& sourcePoints,
    const QMatrix4x4& transform,
    float errorMagnitude)
{
    QList<QPair<QVector3D, QVector3D>> correspondences;
    
    for (const QVector3D& source : sourcePoints) {
        QVector3D target = transform.map(source);
        // Add error in X direction
        target.setX(target.x() + errorMagnitude);
        correspondences.append(qMakePair(source, target));
    }
    
    return correspondences;
}

QMatrix4x4 TestErrorAnalysis::createTestTransformation()
{
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(2.0f, 3.0f, 4.0f);
    transform.rotate(30.0f, 0.0f, 0.0f, 1.0f);
    return transform;
}

bool TestErrorAnalysis::isFloatClose(float a, float b, float tolerance)
{
    return std::abs(a - b) <= tolerance;
}

QTEST_MAIN(TestErrorAnalysis)
#include "test_error_analysis.moc"
