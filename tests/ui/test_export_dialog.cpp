#include <QDebug>
#include <QTemporaryDir>

// Include UI components
#include "ui/ExportDialog.h"

#include <gtest/gtest.h>

class ExportDialogTest : public ::testing::Test
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

    QTemporaryDir* m_tempDir;
    std::vector<Point> m_testPoints;
};

TEST_F(ExportDialogTest, ExportDialog)
{
    ExportDialog dialog;

    // Test dialog setup
    dialog.setPointCloudData(m_testPoints);

    ExportOptions defaultOptions;
    defaultOptions.outputPath = m_tempDir->path() + "/dialog_test.e57";
    defaultOptions.projectName = "Dialog Test";
    dialog.setDefaultOptions(defaultOptions);

    // Test getting options
    ExportOptions retrievedOptions = dialog.getExportOptions();
    EXPECT_EQ(retrievedOptions.projectName, QString("Dialog Test"));
}
