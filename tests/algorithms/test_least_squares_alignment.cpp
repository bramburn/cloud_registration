#include <QtTest/QtTest>
#include <QVector3D>
#include <QMatrix4x4>
#include <QList>
#include <QPair>
#include <cmath>

// Include the class under test
#include "../../src/algorithms/LeastSquaresAlignment.h"

/**
 * @brief Unit tests for LeastSquaresAlignment class
 * 
 * Tests the core least-squares transformation computation algorithm
 * with various scenarios including perfect alignment, noisy data,
 * and edge cases.
 */
class TestLeastSquaresAlignment : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Core functionality tests
    void testPerfectAlignment();
    void testTranslationOnly();
    void testRotationOnly();
    void testCombinedTransformation();
    void testNoisyData();

    // Edge case tests
    void testMinimumCorrespondences();
    void testCollinearPoints();
    void testDuplicatePoints();
    void testEmptyCorrespondences();

    // Numerical stability tests
    void testLargeTranslations();
    void testSmallRotations();
    void testReflectionCorrection();

private:
    // Helper methods
    QList<QPair<QVector3D, QVector3D>> createTestCorrespondences(
        const QList<QVector3D>& sourcePoints,
        const QMatrix4x4& knownTransform);
    
    bool isTransformationClose(const QMatrix4x4& computed, const QMatrix4x4& expected, float tolerance = 1e-3f);
    QMatrix4x4 createRotationMatrix(float angleX, float angleY, float angleZ);
    QMatrix4x4 createTranslationMatrix(float x, float y, float z);
    
    // Test data
    QList<QVector3D> m_testPoints;
    static constexpr float TOLERANCE = 1e-3f;
};

void TestLeastSquaresAlignment::initTestCase()
{
    qDebug() << "Starting LeastSquaresAlignment tests";
    
    // Create a set of test points that are not collinear
    m_testPoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f),
        QVector3D(1.0f, 1.0f, 1.0f)
    };
}

void TestLeastSquaresAlignment::cleanupTestCase()
{
    qDebug() << "LeastSquaresAlignment tests completed";
}

void TestLeastSquaresAlignment::testPerfectAlignment()
{
    qDebug() << "Testing perfect alignment (identity transformation)";
    
    // Create correspondences with identical points
    QList<QPair<QVector3D, QVector3D>> correspondences;
    for (const QVector3D& point : m_testPoints) {
        correspondences.append(qMakePair(point, point));
    }
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    // Should be identity matrix
    QMatrix4x4 identity;
    identity.setToIdentity();
    
    QVERIFY2(isTransformationClose(result, identity), 
             "Perfect alignment should produce identity transformation");
}

void TestLeastSquaresAlignment::testTranslationOnly()
{
    qDebug() << "Testing translation-only transformation";
    
    QVector3D translation(5.0f, -3.0f, 2.0f);
    QMatrix4x4 expectedTransform = createTranslationMatrix(translation.x(), translation.y(), translation.z());
    
    // Create correspondences with known translation
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(m_testPoints, expectedTransform);
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    QVERIFY2(isTransformationClose(result, expectedTransform), 
             "Translation-only transformation should be computed accurately");
}

void TestLeastSquaresAlignment::testRotationOnly()
{
    qDebug() << "Testing rotation-only transformation";
    
    // 45-degree rotation around Z-axis
    QMatrix4x4 expectedTransform = createRotationMatrix(0.0f, 0.0f, 45.0f);
    
    // Create correspondences with known rotation
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(m_testPoints, expectedTransform);
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    QVERIFY2(isTransformationClose(result, expectedTransform, 1e-2f), 
             "Rotation-only transformation should be computed accurately");
}

void TestLeastSquaresAlignment::testCombinedTransformation()
{
    qDebug() << "Testing combined rotation and translation";
    
    // Create combined transformation
    QMatrix4x4 rotation = createRotationMatrix(30.0f, 45.0f, 60.0f);
    QMatrix4x4 translation = createTranslationMatrix(2.0f, -1.5f, 3.0f);
    QMatrix4x4 expectedTransform = translation * rotation;
    
    // Create correspondences
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(m_testPoints, expectedTransform);
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    QVERIFY2(isTransformationClose(result, expectedTransform, 1e-2f), 
             "Combined transformation should be computed accurately");
}

void TestLeastSquaresAlignment::testNoisyData()
{
    qDebug() << "Testing alignment with noisy data";
    
    QMatrix4x4 expectedTransform = createTranslationMatrix(1.0f, 2.0f, 3.0f);
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(m_testPoints, expectedTransform);
    
    // Add small amount of noise to target points
    for (auto& pair : correspondences) {
        pair.second += QVector3D(0.01f, -0.01f, 0.005f); // 1cm noise
    }
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    // Should be close to expected (within noise tolerance)
    QVERIFY2(isTransformationClose(result, expectedTransform, 0.1f), 
             "Noisy data should still produce reasonable transformation");
}

void TestLeastSquaresAlignment::testMinimumCorrespondences()
{
    qDebug() << "Testing with minimum number of correspondences (3 points)";
    
    QList<QVector3D> minPoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f)
    };
    
    QMatrix4x4 expectedTransform = createTranslationMatrix(2.0f, 3.0f, 4.0f);
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(minPoints, expectedTransform);
    
    // Compute transformation
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    QVERIFY2(isTransformationClose(result, expectedTransform), 
             "Minimum correspondences should produce exact transformation");
}

void TestLeastSquaresAlignment::testCollinearPoints()
{
    qDebug() << "Testing with collinear points (degenerate case)";
    
    QList<QVector3D> collinearPoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(2.0f, 0.0f, 0.0f),
        QVector3D(3.0f, 0.0f, 0.0f)
    };
    
    QMatrix4x4 transform = createTranslationMatrix(1.0f, 1.0f, 1.0f);
    QList<QPair<QVector3D, QVector3D>> correspondences = 
        createTestCorrespondences(collinearPoints, transform);
    
    // Should handle gracefully (may not be perfectly accurate due to degeneracy)
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);
    
    // Just verify it doesn't crash and produces a valid matrix
    QVERIFY2(!result.isIdentity() || transform.isIdentity(), 
             "Collinear points should be handled gracefully");
}

void TestLeastSquaresAlignment::testEmptyCorrespondences()
{
    qDebug() << "Testing with empty correspondences";

    QList<QPair<QVector3D, QVector3D>> emptyCorrespondences;

    // Should return identity matrix for empty input
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(emptyCorrespondences);

    QMatrix4x4 identity;
    identity.setToIdentity();

    QVERIFY2(isTransformationClose(result, identity),
             "Empty correspondences should return identity matrix");
}

void TestLeastSquaresAlignment::testDuplicatePoints()
{
    qDebug() << "Testing with duplicate points";

    QList<QPair<QVector3D, QVector3D>> duplicateCorrespondences;
    QVector3D duplicate(0, 0, 0);

    // Add duplicate source points
    duplicateCorrespondences.append(qMakePair(duplicate, QVector3D(1, 0, 0)));
    duplicateCorrespondences.append(qMakePair(duplicate, QVector3D(2, 0, 0))); // Duplicate source
    duplicateCorrespondences.append(qMakePair(QVector3D(1, 0, 0), QVector3D(3, 0, 0)));

    // Should handle gracefully (return identity or valid transformation)
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(duplicateCorrespondences);

    // Just verify it doesn't crash and produces a valid matrix
    QVERIFY2(!std::isnan(result(0, 0)), "Result should not contain NaN values");
}

void TestLeastSquaresAlignment::testLargeTranslations()
{
    qDebug() << "Testing with large translations";

    QVector3D largeTranslation(1000.0f, -500.0f, 2000.0f); // Large translation
    QMatrix4x4 expectedTransform = createTranslationMatrix(largeTranslation.x(), largeTranslation.y(), largeTranslation.z());

    QList<QPair<QVector3D, QVector3D>> correspondences =
        createTestCorrespondences(m_testPoints, expectedTransform);

    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);

    QVERIFY2(isTransformationClose(result, expectedTransform, 1.0f),
             "Large translations should be computed accurately");
}

void TestLeastSquaresAlignment::testSmallRotations()
{
    qDebug() << "Testing with small rotations";

    // Very small rotation (0.1 degrees around Z-axis)
    QMatrix4x4 expectedTransform = createRotationMatrix(0.0f, 0.0f, 0.1f);

    QList<QPair<QVector3D, QVector3D>> correspondences =
        createTestCorrespondences(m_testPoints, expectedTransform);

    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);

    QVERIFY2(isTransformationClose(result, expectedTransform, 1e-2f),
             "Small rotations should be computed accurately");
}

void TestLeastSquaresAlignment::testReflectionCorrection()
{
    qDebug() << "Testing reflection correction in SVD";
    
    // Create a case that might produce reflection
    QList<QVector3D> sourcePoints = {
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f)
    };
    
    QList<QVector3D> targetPoints = {
        QVector3D(-1.0f, 0.0f, 0.0f),  // Mirrored
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f)
    };
    
    QList<QPair<QVector3D, QVector3D>> correspondences;
    for (int i = 0; i < sourcePoints.size(); ++i) {
        correspondences.append(qMakePair(sourcePoints[i], targetPoints[i]));
    }
    
    QMatrix4x4 result = LeastSquaresAlignment::computeTransformation(correspondences);

    // Check that determinant of rotation part is positive (proper rotation, not reflection)
    // Extract rotation matrix (top-left 3x3) and calculate determinant manually
    float r00 = result(0, 0), r01 = result(0, 1), r02 = result(0, 2);
    float r10 = result(1, 0), r11 = result(1, 1), r12 = result(1, 2);
    float r20 = result(2, 0), r21 = result(2, 1), r22 = result(2, 2);

    // Calculate determinant manually
    float determinant = r00 * (r11 * r22 - r12 * r21) -
                       r01 * (r10 * r22 - r12 * r20) +
                       r02 * (r10 * r21 - r11 * r20);
    QVERIFY2(determinant > 0.5f, "Rotation matrix should have positive determinant (no reflection)");
}

// Helper method implementations
QList<QPair<QVector3D, QVector3D>> TestLeastSquaresAlignment::createTestCorrespondences(
    const QList<QVector3D>& sourcePoints,
    const QMatrix4x4& knownTransform)
{
    QList<QPair<QVector3D, QVector3D>> correspondences;
    
    for (const QVector3D& sourcePoint : sourcePoints) {
        QVector3D targetPoint = knownTransform.map(sourcePoint);
        correspondences.append(qMakePair(sourcePoint, targetPoint));
    }
    
    return correspondences;
}

bool TestLeastSquaresAlignment::isTransformationClose(
    const QMatrix4x4& computed, 
    const QMatrix4x4& expected, 
    float tolerance)
{
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float diff = std::abs(computed(row, col) - expected(row, col));
            if (diff > tolerance) {
                qDebug() << "Matrix difference at (" << row << "," << col << "):" 
                         << diff << ">" << tolerance;
                qDebug() << "Computed:" << computed;
                qDebug() << "Expected:" << expected;
                return false;
            }
        }
    }
    return true;
}

QMatrix4x4 TestLeastSquaresAlignment::createRotationMatrix(float angleX, float angleY, float angleZ)
{
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.rotate(angleX, 1.0f, 0.0f, 0.0f);
    matrix.rotate(angleY, 0.0f, 1.0f, 0.0f);
    matrix.rotate(angleZ, 0.0f, 0.0f, 1.0f);
    return matrix;
}

QMatrix4x4 TestLeastSquaresAlignment::createTranslationMatrix(float x, float y, float z)
{
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.translate(x, y, z);
    return matrix;
}

QTEST_MAIN(TestLeastSquaresAlignment)
#include "test_least_squares_alignment.moc"
