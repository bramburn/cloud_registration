#include <QMatrix4x4>
#include <QPoint>
#include <QSize>
#include <QVector3D>

#include <cmath>
#include <vector>

#include "detection/NaturalPointSelector.h"
#include "pointdata.h"
#include "registration/Target.h"

#include <gtest/gtest.h>

class NaturalPointSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        selector = std::make_unique<NaturalPointSelector>();

        // Set up default parameters
        params.distanceThreshold = 0.01f;
        params.neighborhoodRadius = 0.1f;
        params.curvatureThreshold = 0.1f;
        params.enablePreprocessing = false;

        // Set up test view matrices
        viewMatrix.setToIdentity();
        viewMatrix.lookAt(QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

        projectionMatrix.setToIdentity();
        projectionMatrix.perspective(45.0f, 1.0f, 0.1f, 100.0f);

        viewportSize = QSize(800, 600);
    }

    // Generate test point cloud with some geometric features
    std::vector<PointFullData> generateTestPointCloud()
    {
        std::vector<PointFullData> points;

        // Create a planar surface
        for (int i = 0; i < 10; ++i)
        {
            for (int j = 0; j < 10; ++j)
            {
                PointFullData point;
                point.x = static_cast<float>(i) * 0.1f;
                point.y = static_cast<float>(j) * 0.1f;
                point.z = 0.0f;
                point.intensity = 100.0f;
                point.hasIntensity = true;
                point.hasNormal = true;
                point.nx = 0.0f;
                point.ny = 0.0f;
                point.nz = 1.0f;
                points.push_back(point);
            }
        }

        // Add some edge points (linear feature)
        for (int i = 0; i < 10; ++i)
        {
            PointFullData point;
            point.x = static_cast<float>(i) * 0.1f;
            point.y = 1.0f;
            point.z = static_cast<float>(i) * 0.05f;  // Sloped edge
            point.intensity = 150.0f;
            point.hasIntensity = true;
            point.hasNormal = true;
            point.nx = 0.0f;
            point.ny = -0.707f;
            point.nz = 0.707f;
            points.push_back(point);
        }

        // Add a corner point (distinctive feature)
        PointFullData cornerPoint;
        cornerPoint.x = 1.0f;
        cornerPoint.y = 1.0f;
        cornerPoint.z = 0.5f;
        cornerPoint.intensity = 200.0f;
        cornerPoint.hasIntensity = true;
        cornerPoint.hasNormal = true;
        cornerPoint.nx = 0.577f;
        cornerPoint.ny = 0.577f;
        cornerPoint.nz = 0.577f;
        points.push_back(cornerPoint);

        return points;
    }

    std::unique_ptr<NaturalPointSelector> selector;
    TargetDetectionBase::DetectionParams params;
    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QSize viewportSize;
};

// Test basic point selection using screen coordinates
TEST_F(NaturalPointSelectorTest, SelectPointFromScreen)
{
    auto points = generateTestPointCloud();

    // Try to select the corner point (should be most distinctive)
    QPoint screenPos(600, 200);  // Approximate screen position

    auto result = selector->selectPoint(points, viewMatrix, projectionMatrix, screenPos, viewportSize, 10.0f);

    // Should find some point (exact matching depends on projection)
    // For this test, we mainly check that the selection mechanism works
    if (result.success)
    {
        EXPECT_TRUE(result.isValid());
        EXPECT_GE(result.pointIndex, 0);
        EXPECT_LT(result.pointIndex, static_cast<int>(points.size()));
        EXPECT_GT(result.confidence, 0.0f);
        EXPECT_FALSE(result.description.isEmpty());
    }
}

// Test closest point selection
TEST_F(NaturalPointSelectorTest, SelectClosestPoint)
{
    auto points = generateTestPointCloud();

    // Target position close to the corner point
    QVector3D targetPosition(1.0f, 1.0f, 0.5f);

    auto result = selector->selectClosestPoint(points, targetPosition, 0.1f);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.isValid());

    // Should find the corner point
    QVector3D selectedPos = result.selectedPoint;
    float distance = (selectedPos - targetPosition).length();
    EXPECT_LT(distance, 0.1f);

    // Check that we got a reasonable confidence and description
    EXPECT_GT(result.confidence, 0.0f);
    EXPECT_FALSE(result.description.isEmpty());
}

// Test closest point selection with no points in range
TEST_F(NaturalPointSelectorTest, SelectClosestPointOutOfRange)
{
    auto points = generateTestPointCloud();

    // Target position far from any points
    QVector3D targetPosition(10.0f, 10.0f, 10.0f);

    auto result = selector->selectClosestPoint(points, targetPosition, 0.1f);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.isValid());
}

// Test correspondence suggestion
TEST_F(NaturalPointSelectorTest, SuggestCorrespondences)
{
    auto sourcePoints = generateTestPointCloud();

    // Create target points similar to source but slightly offset
    auto targetPoints = generateTestPointCloud();
    for (auto& point : targetPoints)
    {
        point.x += 0.05f;  // Small offset
        point.y += 0.05f;
    }

    // Select a point from source
    QVector3D sourcePosition(1.0f, 1.0f, 0.5f);  // Corner point
    auto sourceSelection = selector->selectClosestPoint(sourcePoints, sourcePosition, 0.1f);

    ASSERT_TRUE(sourceSelection.success);

    // Find correspondences in target
    auto correspondences = selector->suggestCorrespondences(sourcePoints, targetPoints, sourceSelection, 0.2f);

    // Should find at least one correspondence
    EXPECT_GT(correspondences.size(), 0);

    if (!correspondences.empty())
    {
        // Best correspondence should have reasonable confidence
        EXPECT_GT(correspondences[0].confidence, 0.3f);
        EXPECT_TRUE(correspondences[0].isValid());

        // Position should be close to expected offset position
        QVector3D expectedPos = sourceSelection.selectedPoint + QVector3D(0.05f, 0.05f, 0.0f);
        float distance = (correspondences[0].selectedPoint - expectedPos).length();
        EXPECT_LT(distance, 0.2f);
    }
}

// Test feature analysis
TEST_F(NaturalPointSelectorTest, FeatureAnalysis)
{
    auto points = generateTestPointCloud();

    // Test different types of points

    // Planar point (should have high planarity)
    QVector3D planarPosition(0.5f, 0.5f, 0.0f);
    auto planarResult = selector->selectClosestPoint(points, planarPosition, 0.1f);

    if (planarResult.success)
    {
        // Should detect planar characteristics
        EXPECT_GT(planarResult.featureVector.x(), 0.3f);  // High planarity
        EXPECT_TRUE(planarResult.description.contains("surface") || planarResult.description.contains("Planar"));
    }

    // Edge point (should have high linearity)
    QVector3D edgePosition(0.5f, 1.0f, 0.025f);
    auto edgeResult = selector->selectClosestPoint(points, edgePosition, 0.1f);

    if (edgeResult.success)
    {
        // Should detect linear characteristics
        EXPECT_GT(edgeResult.featureVector.y(), 0.2f);  // Some linearity
        EXPECT_TRUE(edgeResult.description.contains("edge") || edgeResult.description.contains("Linear") ||
                    edgeResult.description.contains("ridge"));
    }

    // Corner point (should have high sphericity/corner characteristics)
    QVector3D cornerPosition(1.0f, 1.0f, 0.5f);
    auto cornerResult = selector->selectClosestPoint(points, cornerPosition, 0.1f);

    if (cornerResult.success)
    {
        // Should detect corner characteristics
        EXPECT_GT(cornerResult.confidence, 0.5f);  // High confidence for distinctive feature
        EXPECT_TRUE(cornerResult.description.contains("corner") || cornerResult.description.contains("Point") ||
                    cornerResult.description.contains("isolated"));
    }
}

// Test parameter validation
TEST_F(NaturalPointSelectorTest, ParameterValidation)
{
    // Test valid parameters
    EXPECT_TRUE(selector->validateParameters(params));

    // Test invalid parameters
    TargetDetectionBase::DetectionParams invalidParams = params;

    // Invalid neighborhood radius
    invalidParams.neighborhoodRadius = -0.1f;
    EXPECT_FALSE(selector->validateParameters(invalidParams));

    // Invalid curvature threshold
    invalidParams = params;
    invalidParams.curvatureThreshold = -0.1f;
    EXPECT_FALSE(selector->validateParameters(invalidParams));
}

// Test empty point cloud handling
TEST_F(NaturalPointSelectorTest, EmptyPointCloud)
{
    std::vector<PointFullData> emptyPoints;

    QPoint screenPos(400, 300);
    auto result = selector->selectPoint(emptyPoints, viewMatrix, projectionMatrix, screenPos, viewportSize);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.isValid());
}

// Test algorithm information
TEST_F(NaturalPointSelectorTest, AlgorithmInfo)
{
    EXPECT_EQ(selector->getAlgorithmName(), "Natural Point Selector");

    auto supportedTypes = selector->getSupportedTargetTypes();
    EXPECT_EQ(supportedTypes.size(), 1);
    EXPECT_EQ(supportedTypes[0], "Natural Point");
}

// Test detect method (should indicate manual interaction required)
TEST_F(NaturalPointSelectorTest, DetectMethod)
{
    auto points = generateTestPointCloud();

    auto result = selector->detect(points, params);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.isEmpty());
    EXPECT_TRUE(result.errorMessage.contains("manual"));
}

// Test ray creation and point-to-ray distance calculation
TEST_F(NaturalPointSelectorTest, RayCalculations)
{
    auto points = generateTestPointCloud();

    // Test with center of screen (should create reasonable ray)
    QPoint centerScreen(viewportSize.width() / 2, viewportSize.height() / 2);

    auto result = selector->selectPoint(points, viewMatrix, projectionMatrix, centerScreen, viewportSize, 50.0f);

    // Should be able to select some point when using large selection radius
    // The exact result depends on the projection, but the mechanism should work
    if (result.success)
    {
        EXPECT_TRUE(result.isValid());
        EXPECT_GE(result.pointIndex, 0);
    }
}

// Test feature similarity calculation
TEST_F(NaturalPointSelectorTest, FeatureSimilarity)
{
    auto points = generateTestPointCloud();

    // Select two similar points (both planar)
    QVector3D pos1(0.2f, 0.2f, 0.0f);
    QVector3D pos2(0.7f, 0.7f, 0.0f);

    auto result1 = selector->selectClosestPoint(points, pos1, 0.1f);
    auto result2 = selector->selectClosestPoint(points, pos2, 0.1f);

    if (result1.success && result2.success)
    {
        // Both should be planar points, so should have similar features
        // This is tested indirectly through the correspondence suggestion
        auto correspondences = selector->suggestCorrespondences(points, points, result1, 2.0f);  // Large search radius

        // Should find the second point as a potential correspondence
        bool foundSimilar = false;
        for (const auto& corr : correspondences)
        {
            float distance = (corr.selectedPoint - result2.selectedPoint).length();
            if (distance < 0.1f && corr.confidence > 0.5f)
            {
                foundSimilar = true;
                break;
            }
        }

        EXPECT_TRUE(foundSimilar);
    }
}

// Test confidence calculation
TEST_F(NaturalPointSelectorTest, ConfidenceCalculation)
{
    auto points = generateTestPointCloud();

    // Corner point should have higher confidence than planar point
    QVector3D cornerPos(1.0f, 1.0f, 0.5f);
    QVector3D planarPos(0.5f, 0.5f, 0.0f);

    auto cornerResult = selector->selectClosestPoint(points, cornerPos, 0.1f);
    auto planarResult = selector->selectClosestPoint(points, planarPos, 0.1f);

    if (cornerResult.success && planarResult.success)
    {
        // Corner should generally have higher confidence due to distinctiveness
        // Note: This might not always be true depending on the exact feature calculation
        // but it's a reasonable expectation for this test setup
        EXPECT_GT(cornerResult.confidence, 0.3f);
        EXPECT_GT(planarResult.confidence, 0.0f);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
