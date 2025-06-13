#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

// Include export components
#include "export/FormatWriters/E57Writer.h"
#include "export/FormatWriters/LASWriter.h"
#include "export/FormatWriters/PLYWriter.h"
#include "export/FormatWriters/XYZWriter.h"
#include "export/PointCloudExporter.h"

#include <gtest/gtest.h>

class ExportFunctionalityTest : public ::testing::Test
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

    void verifyFileSize(const QString& filePath, qint64 minSize)
    {
        QFileInfo fileInfo(filePath);
        EXPECT_GE(fileInfo.size(), minSize) << "File size too small: " << filePath.toStdString();
    }

    QTemporaryDir* m_tempDir;
    std::vector<Point> m_testPoints;
};

TEST_F(ExportFunctionalityTest, PointCloudExporter)
{
    PointCloudExporter exporter;

    // Test supported formats
    QStringList formats = PointCloudExporter::getSupportedFormats();
    EXPECT_TRUE(formats.contains("E57"));
    EXPECT_TRUE(formats.contains("LAS"));
    EXPECT_TRUE(formats.contains("PLY"));
    EXPECT_TRUE(formats.contains("XYZ"));

    // Test file extensions
    EXPECT_EQ(PointCloudExporter::getFileExtension(ExportFormat::E57), QString(".e57"));
    EXPECT_EQ(PointCloudExporter::getFileExtension(ExportFormat::LAS), QString(".las"));
    EXPECT_EQ(PointCloudExporter::getFileExtension(ExportFormat::PLY), QString(".ply"));
    EXPECT_EQ(PointCloudExporter::getFileExtension(ExportFormat::XYZ), QString(".xyz"));

    // Test export options validation
    ExportOptions invalidOptions;
    QString error = PointCloudExporter::validateOptions(invalidOptions);
    EXPECT_FALSE(error.isEmpty());

    ExportOptions validOptions;
    validOptions.outputPath = m_tempDir->path() + "/test.e57";
    validOptions.projectName = "Test Project";
    validOptions.description = "Test export";
    error = PointCloudExporter::validateOptions(validOptions);
    EXPECT_TRUE(error.isEmpty());

    // Test synchronous export
    ExportResult result = exporter.exportPointCloud(m_testPoints, validOptions);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.pointsExported, m_testPoints.size());
    EXPECT_GT(result.exportTimeSeconds, 0);

    verifyFileExists(validOptions.outputPath);
    verifyFileSize(validOptions.outputPath, 100);  // At least 100 bytes
}

TEST_F(ExportFunctionalityTest, E57Writer)
{
    E57Writer writer;
    QString outputPath = m_tempDir->path() + "/test_e57.e57";

    // Test writer features
    EXPECT_TRUE(writer.supportsFeature("color"));
    EXPECT_TRUE(writer.supportsFeature("intensity"));
    EXPECT_TRUE(writer.supportsFeature("compression"));

    // Test file operations
    EXPECT_TRUE(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "E57 Test";
    header.hasColor = true;
    header.hasIntensity = true;
    EXPECT_TRUE(writer.writeHeader(header));

    // Write some test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        EXPECT_TRUE(writer.writePoint(m_testPoints[i]));
    }

    EXPECT_TRUE(writer.close());
    verifyFileExists(outputPath);
}

TEST_F(ExportFunctionalityTest, LASWriter)
{
    LASWriter writer;
    QString outputPath = m_tempDir->path() + "/test_las.las";

    // Test writer features
    EXPECT_TRUE(writer.supportsFeature("color"));
    EXPECT_TRUE(writer.supportsFeature("intensity"));

    // Test file operations
    EXPECT_TRUE(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "LAS Test";
    header.hasColor = true;
    header.hasIntensity = true;
    EXPECT_TRUE(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        EXPECT_TRUE(writer.writePoint(m_testPoints[i]));
    }

    EXPECT_TRUE(writer.close());
    verifyFileExists(outputPath);
}

TEST_F(ExportFunctionalityTest, PLYWriter)
{
    PLYWriter writer;
    QString outputPath = m_tempDir->path() + "/test_ply.ply";

    // Test writer features
    EXPECT_TRUE(writer.supportsFeature("color"));
    EXPECT_TRUE(writer.supportsFeature("intensity"));
    EXPECT_TRUE(writer.supportsFeature("ascii"));
    EXPECT_TRUE(writer.supportsFeature("binary"));

    // Test ASCII format
    writer.setAsciiFormat(true);
    writer.setPrecision(6);

    EXPECT_TRUE(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "PLY Test";
    header.hasColor = true;
    header.hasIntensity = true;
    EXPECT_TRUE(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        EXPECT_TRUE(writer.writePoint(m_testPoints[i]));
    }

    EXPECT_TRUE(writer.close());
    verifyFileExists(outputPath);
}

TEST_F(ExportFunctionalityTest, XYZWriter)
{
    XYZWriter writer;
    QString outputPath = m_tempDir->path() + "/test_xyz.xyz";

    // Test writer features
    EXPECT_TRUE(writer.supportsFeature("color"));
    EXPECT_TRUE(writer.supportsFeature("intensity"));
    EXPECT_TRUE(writer.supportsFeature("comments"));
    EXPECT_TRUE(writer.supportsFeature("separator"));

    // Test format options
    writer.setFormat(XYZWriter::Format::XYZRGB);
    writer.setPrecision(6);
    writer.setFieldSeparator(" ");
    writer.setHeaderCommentsEnabled(true);

    EXPECT_TRUE(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "XYZ Test";
    header.hasColor = true;
    header.hasIntensity = false;
    EXPECT_TRUE(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        EXPECT_TRUE(writer.writePoint(m_testPoints[i]));
    }

    EXPECT_TRUE(writer.close());
    verifyFileExists(outputPath);
}

TEST_F(ExportFunctionalityTest, CompleteExportWorkflow)
{
    // Create test data
    std::vector<Point> pointCloud = createTestPointCloud(500);

    // Test export to all formats
    QStringList formats = {"E57", "LAS", "PLY", "XYZ"};
    QList<ExportFormat> exportFormats = {ExportFormat::E57, ExportFormat::LAS, ExportFormat::PLY, ExportFormat::XYZ};

    PointCloudExporter exporter;

    for (int i = 0; i < formats.size(); ++i)
    {
        ExportOptions options;
        options.format = exportFormats[i];
        options.outputPath =
            m_tempDir->path() + "/workflow_test" + PointCloudExporter::getFileExtension(exportFormats[i]);
        options.projectName = "Workflow Test";
        options.description = "Complete workflow test";
        options.includeColor = true;
        options.includeIntensity = true;

        ExportResult result = exporter.exportPointCloud(pointCloud, options);
        EXPECT_TRUE(result.success) << "Export failed: " << result.errorMessage.toStdString();
        EXPECT_EQ(result.pointsExported, pointCloud.size());

        verifyFileExists(options.outputPath);
    }
}
