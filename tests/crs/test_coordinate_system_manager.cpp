#include <QDebug>
#include <QVector3D>

// Include CRS components
#include "crs/CoordinateSystemManager.h"

#include <gtest/gtest.h>

class CoordinateSystemManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        manager = new CoordinateSystemManager();
    }

    void TearDown() override
    {
        delete manager;
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

    CoordinateSystemManager* manager;
};

TEST_F(CoordinateSystemManagerTest, AvailableCRS)
{
    // Test available CRS
    QStringList crsList = manager->getAvailableCRS();
    EXPECT_TRUE(crsList.contains("WGS84"));
    EXPECT_TRUE(crsList.contains("UTM Zone 10N"));
    EXPECT_TRUE(crsList.contains("Local"));
}

TEST_F(CoordinateSystemManagerTest, CRSDefinitions)
{
    // Test CRS definitions
    CRSDefinition wgs84 = manager->getCRSDefinition("WGS84");
    EXPECT_EQ(wgs84.name, QString("WGS84"));
    EXPECT_EQ(wgs84.type, QString("geographic"));
}

TEST_F(CoordinateSystemManagerTest, TransformationAvailability)
{
    // Test transformation availability
    EXPECT_TRUE(manager->isTransformationAvailable("WGS84", "UTM Zone 10N"));
    EXPECT_TRUE(manager->isTransformationAvailable("Local", "Local"));  // Identity
}

TEST_F(CoordinateSystemManagerTest, CustomCRS)
{
    // Test custom CRS
    CRSDefinition customCRS;
    customCRS.name = "Test CRS";
    customCRS.type = "local";
    customCRS.units = "meters";
    customCRS.description = "Test coordinate system";

    EXPECT_TRUE(manager->addCustomCRS(customCRS));
    EXPECT_TRUE(manager->getAvailableCRS().contains("Test CRS"));
    EXPECT_TRUE(manager->removeCustomCRS("Test CRS"));
}

TEST_F(CoordinateSystemManagerTest, CoordinateTransformation)
{
    // Test point transformation
    QVector3D testPoint(100.0, 200.0, 50.0);
    QVector3D transformedPoint = manager->transformPoint(testPoint, "Local", "Local");

    // Identity transformation should return same point
    EXPECT_EQ(transformedPoint.x(), testPoint.x());
    EXPECT_EQ(transformedPoint.y(), testPoint.y());
    EXPECT_EQ(transformedPoint.z(), testPoint.z());

    // Test point cloud transformation
    std::vector<Point> originalPoints = createTestPointCloud(100);
    std::vector<Point> transformedPoints = manager->transformPoints(originalPoints, "Local", "Local");

    EXPECT_EQ(transformedPoints.size(), originalPoints.size());
}
