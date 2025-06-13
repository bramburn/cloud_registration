#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

// Include Sprint 6 components
#include "../src/crs/CoordinateSystemManager.h"
#include "../src/export/FormatWriters/E57Writer.h"
#include "../src/export/FormatWriters/LASWriter.h"
#include "../src/export/FormatWriters/PLYWriter.h"
#include "../src/export/FormatWriters/XYZWriter.h"
#include "../src/export/PointCloudExporter.h"
#include "../src/quality/PDFReportGenerator.h"
#include "../src/quality/QualityAssessment.h"
#include "../src/ui/ExportDialog.h"

class Sprint6Test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Export functionality tests
    void testPointCloudExporter();
    void testE57Writer();
    void testLASWriter();
    void testPLYWriter();
    void testXYZWriter();
    void testExportDialog();

    // Quality assessment tests
    void testQualityAssessment();
    void testPDFReportGenerator();

    // Coordinate system tests
    void testCoordinateSystemManager();
    void testCoordinateTransformation();

    // Integration tests
    void testCompleteExportWorkflow();
    void testQualityReportWorkflow();

private:
    std::vector<Point> createTestPointCloud(size_t numPoints = 1000);
    void verifyFileExists(const QString& filePath);
    void verifyFileSize(const QString& filePath, qint64 minSize);

    QTemporaryDir* m_tempDir;
    std::vector<Point> m_testPoints;
};

void Sprint6Test::initTestCase()
{
    qDebug() << "Starting Sprint 6 comprehensive tests...";

    // Create temporary directory for test files
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    // Create test point cloud data
    m_testPoints = createTestPointCloud(1000);
    QVERIFY(!m_testPoints.empty());

    qDebug() << "Test setup complete. Temp dir:" << m_tempDir->path();
}

void Sprint6Test::cleanupTestCase()
{
    delete m_tempDir;
    qDebug() << "Sprint 6 tests completed.";
}

void Sprint6Test::testPointCloudExporter()
{
    qDebug() << "Testing PointCloudExporter...";

    PointCloudExporter exporter;

    // Test supported formats
    QStringList formats = PointCloudExporter::getSupportedFormats();
    QVERIFY(formats.contains("E57"));
    QVERIFY(formats.contains("LAS"));
    QVERIFY(formats.contains("PLY"));
    QVERIFY(formats.contains("XYZ"));

    // Test file extensions
    QCOMPARE(PointCloudExporter::getFileExtension(ExportFormat::E57), QString(".e57"));
    QCOMPARE(PointCloudExporter::getFileExtension(ExportFormat::LAS), QString(".las"));
    QCOMPARE(PointCloudExporter::getFileExtension(ExportFormat::PLY), QString(".ply"));
    QCOMPARE(PointCloudExporter::getFileExtension(ExportFormat::XYZ), QString(".xyz"));

    // Test export options validation
    ExportOptions invalidOptions;
    QString error = PointCloudExporter::validateOptions(invalidOptions);
    QVERIFY(!error.isEmpty());

    ExportOptions validOptions;
    validOptions.outputPath = m_tempDir->path() + "/test.e57";
    validOptions.projectName = "Test Project";
    validOptions.description = "Test export";
    error = PointCloudExporter::validateOptions(validOptions);
    QVERIFY(error.isEmpty());

    // Test synchronous export
    ExportResult result = exporter.exportPointCloud(m_testPoints, validOptions);
    QVERIFY(result.success);
    QCOMPARE(result.pointsExported, m_testPoints.size());
    QVERIFY(result.exportTimeSeconds > 0);

    verifyFileExists(validOptions.outputPath);
    verifyFileSize(validOptions.outputPath, 100);  // At least 100 bytes

    qDebug() << "PointCloudExporter test passed";
}

void Sprint6Test::testE57Writer()
{
    qDebug() << "Testing E57Writer...";

    E57Writer writer;
    QString outputPath = m_tempDir->path() + "/test_e57.e57";

    // Test writer features
    QVERIFY(writer.supportsFeature("color"));
    QVERIFY(writer.supportsFeature("intensity"));
    QVERIFY(writer.supportsFeature("compression"));

    // Test file operations
    QVERIFY(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "E57 Test";
    header.hasColor = true;
    header.hasIntensity = true;
    QVERIFY(writer.writeHeader(header));

    // Write some test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        QVERIFY(writer.writePoint(m_testPoints[i]));
    }

    QVERIFY(writer.close());
    verifyFileExists(outputPath);

    qDebug() << "E57Writer test passed";
}

void Sprint6Test::testLASWriter()
{
    qDebug() << "Testing LASWriter...";

    LASWriter writer;
    QString outputPath = m_tempDir->path() + "/test_las.las";

    // Test writer features
    QVERIFY(writer.supportsFeature("color"));
    QVERIFY(writer.supportsFeature("intensity"));

    // Test file operations
    QVERIFY(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "LAS Test";
    header.hasColor = true;
    header.hasIntensity = true;
    QVERIFY(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        QVERIFY(writer.writePoint(m_testPoints[i]));
    }

    QVERIFY(writer.close());
    verifyFileExists(outputPath);

    qDebug() << "LASWriter test passed";
}

void Sprint6Test::testPLYWriter()
{
    qDebug() << "Testing PLYWriter...";

    PLYWriter writer;
    QString outputPath = m_tempDir->path() + "/test_ply.ply";

    // Test writer features
    QVERIFY(writer.supportsFeature("color"));
    QVERIFY(writer.supportsFeature("intensity"));
    QVERIFY(writer.supportsFeature("ascii"));
    QVERIFY(writer.supportsFeature("binary"));

    // Test ASCII format
    writer.setAsciiFormat(true);
    writer.setPrecision(6);

    QVERIFY(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "PLY Test";
    header.hasColor = true;
    header.hasIntensity = true;
    QVERIFY(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        QVERIFY(writer.writePoint(m_testPoints[i]));
    }

    QVERIFY(writer.close());
    verifyFileExists(outputPath);

    qDebug() << "PLYWriter test passed";
}

void Sprint6Test::testXYZWriter()
{
    qDebug() << "Testing XYZWriter...";

    XYZWriter writer;
    QString outputPath = m_tempDir->path() + "/test_xyz.xyz";

    // Test writer features
    QVERIFY(writer.supportsFeature("color"));
    QVERIFY(writer.supportsFeature("intensity"));
    QVERIFY(writer.supportsFeature("comments"));
    QVERIFY(writer.supportsFeature("separator"));

    // Test format options
    writer.setFormat(XYZWriter::Format::XYZRGB);
    writer.setPrecision(6);
    writer.setFieldSeparator(" ");
    writer.setHeaderCommentsEnabled(true);

    QVERIFY(writer.open(outputPath));

    HeaderInfo header;
    header.pointCount = m_testPoints.size();
    header.projectName = "XYZ Test";
    header.hasColor = true;
    header.hasIntensity = false;
    QVERIFY(writer.writeHeader(header));

    // Write test points
    for (size_t i = 0; i < std::min(m_testPoints.size(), static_cast<size_t>(100)); ++i)
    {
        QVERIFY(writer.writePoint(m_testPoints[i]));
    }

    QVERIFY(writer.close());
    verifyFileExists(outputPath);

    qDebug() << "XYZWriter test passed";
}

void Sprint6Test::testExportDialog()
{
    qDebug() << "Testing ExportDialog...";

    ExportDialog dialog;

    // Test dialog setup
    dialog.setPointCloudData(m_testPoints);

    ExportOptions defaultOptions;
    defaultOptions.outputPath = m_tempDir->path() + "/dialog_test.e57";
    defaultOptions.projectName = "Dialog Test";
    dialog.setDefaultOptions(defaultOptions);

    // Test getting options
    ExportOptions retrievedOptions = dialog.getExportOptions();
    QCOMPARE(retrievedOptions.projectName, QString("Dialog Test"));

    qDebug() << "ExportDialog test passed";
}

void Sprint6Test::testQualityAssessment()
{
    qDebug() << "Testing QualityAssessment...";

    QualityAssessment assessment;

    // Create test point clouds
    std::vector<Point> sourceCloud = createTestPointCloud(500);
    std::vector<Point> targetCloud = createTestPointCloud(500);
    std::vector<Point> transformedCloud = sourceCloud;  // Perfect alignment for test

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

void Sprint6Test::testPDFReportGenerator()
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
    report.recommendations << "Test recommendation 1"
                           << "Test recommendation 2";

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

void Sprint6Test::testCoordinateSystemManager()
{
    qDebug() << "Testing CoordinateSystemManager...";

    CoordinateSystemManager manager;

    // Test available CRS
    QStringList crsList = manager.getAvailableCRS();
    QVERIFY(crsList.contains("WGS84"));
    QVERIFY(crsList.contains("UTM Zone 10N"));
    QVERIFY(crsList.contains("Local"));

    // Test CRS definitions
    CRSDefinition wgs84 = manager.getCRSDefinition("WGS84");
    QCOMPARE(wgs84.name, QString("WGS84"));
    QCOMPARE(wgs84.type, QString("geographic"));

    // Test transformation availability
    QVERIFY(manager.isTransformationAvailable("WGS84", "UTM Zone 10N"));
    QVERIFY(manager.isTransformationAvailable("Local", "Local"));  // Identity

    // Test custom CRS
    CRSDefinition customCRS;
    customCRS.name = "Test CRS";
    customCRS.type = "local";
    customCRS.units = "meters";
    customCRS.description = "Test coordinate system";

    QVERIFY(manager.addCustomCRS(customCRS));
    QVERIFY(manager.getAvailableCRS().contains("Test CRS"));
    QVERIFY(manager.removeCustomCRS("Test CRS"));

    qDebug() << "CoordinateSystemManager test passed";
}

void Sprint6Test::testCoordinateTransformation()
{
    qDebug() << "Testing coordinate transformation...";

    CoordinateSystemManager manager;

    // Test point transformation
    QVector3D testPoint(100.0, 200.0, 50.0);
    QVector3D transformedPoint = manager.transformPoint(testPoint, "Local", "Local");

    // Identity transformation should return same point
    QCOMPARE(transformedPoint.x(), testPoint.x());
    QCOMPARE(transformedPoint.y(), testPoint.y());
    QCOMPARE(transformedPoint.z(), testPoint.z());

    // Test point cloud transformation
    std::vector<Point> originalPoints = createTestPointCloud(100);
    std::vector<Point> transformedPoints = manager.transformPoints(originalPoints, "Local", "Local");

    QCOMPARE(transformedPoints.size(), originalPoints.size());

    qDebug() << "Coordinate transformation test passed";
}

void Sprint6Test::testCompleteExportWorkflow()
{
    qDebug() << "Testing complete export workflow...";

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
        QVERIFY2(result.success, qPrintable(result.errorMessage));
        QCOMPARE(result.pointsExported, pointCloud.size());

        verifyFileExists(options.outputPath);
    }

    qDebug() << "Complete export workflow test passed";
}

void Sprint6Test::testQualityReportWorkflow()
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

std::vector<Point> Sprint6Test::createTestPointCloud(size_t numPoints)
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

void Sprint6Test::verifyFileExists(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.exists(), qPrintable(QString("File does not exist: %1").arg(filePath)));
}

void Sprint6Test::verifyFileSize(const QString& filePath, qint64 minSize)
{
    QFileInfo fileInfo(filePath);
    QVERIFY2(fileInfo.size() >= minSize,
             qPrintable(QString("File size %1 is less than minimum %2 for file: %3")
                            .arg(fileInfo.size())
                            .arg(minSize)
                            .arg(filePath)));
}

QTEST_MAIN(Sprint6Test)
#include "Sprint6Test.moc"
