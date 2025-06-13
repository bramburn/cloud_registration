#include "export/PointCloudExporter.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMutexLocker>

#include <algorithm>
#include <limits>

#include "export/E57Writer.h"
#include "export/LASWriter.h"
#include "export/PLYWriter.h"
#include "export/XYZWriter.h"

/**
 * @brief Worker class for asynchronous export operations
 */
class PointCloudExporter::ExportWorker : public QObject
{
    Q_OBJECT

public:
    ExportWorker(PointCloudExporter* parent) : m_exporter(parent) {}

    void setExportData(const std::vector<Point>& points, const ExportOptions& options)
    {
        m_points = points;
        m_options = options;
    }

public slots:
    void doExport()
    {
        ExportResult result = m_exporter->exportPointCloud(m_points, m_options);
        emit exportCompleted(result);
    }

signals:
    void progressUpdated(int percentage, const QString& stage);
    void exportCompleted(const ExportResult& result);

private:
    PointCloudExporter* m_exporter;
    std::vector<Point> m_points;
    ExportOptions m_options;
};

PointCloudExporter::PointCloudExporter(QObject* parent) : QObject(parent) {}

PointCloudExporter::~PointCloudExporter()
{
    if (m_isExporting)
    {
        cancelExport();
    }
}

ExportResult PointCloudExporter::exportPointCloud(const std::vector<Point>& points, const ExportOptions& options)
{
    QElapsedTimer timer;
    timer.start();

    ExportResult result;
    result.outputPath = options.outputPath;

    try
    {
        // Validate options
        QString validationError = validateOptions(options);
        if (!validationError.isEmpty())
        {
            result.errorMessage = validationError;
            return result;
        }

        emit progressUpdated(0, "Initializing export...");

        // Create appropriate writer
        auto writer = createWriter(options.format);
        if (!writer)
        {
            result.errorMessage = "Failed to create format writer";
            return result;
        }

        emit progressUpdated(10, "Opening output file...");

        // Open file for writing
        if (!writer->open(options.outputPath))
        {
            result.errorMessage = QString("Failed to open output file: %1").arg(writer->getLastError());
            return result;
        }

        emit progressUpdated(20, "Preparing point cloud data...");

        // Transform coordinates if needed
        std::vector<Point> transformedPoints = points;
        if (options.sourceCRS != options.targetCRS)
        {
            emit progressUpdated(25, "Transforming coordinates...");
            transformedPoints = transformCoordinates(points, options.sourceCRS, options.targetCRS);
        }

        emit progressUpdated(30, "Writing header...");

        // Create and write header
        HeaderInfo headerInfo = createHeaderInfo(transformedPoints, options);
        if (!writer->writeHeader(headerInfo))
        {
            result.errorMessage = QString("Failed to write header: %1").arg(writer->getLastError());
            return result;
        }

        emit progressUpdated(40, "Writing point data...");

        // Write points in batches
        size_t totalPoints = transformedPoints.size();
        size_t batchSize = options.batchSize;
        size_t pointsWritten = 0;

        for (size_t i = 0; i < totalPoints; i += batchSize)
        {
            // Check for cancellation
            {
                QMutexLocker locker(&m_mutex);
                if (m_cancelRequested)
                {
                    result.errorMessage = "Export cancelled by user";
                    writer->close();
                    return result;
                }
            }

            size_t endIndex = std::min(i + batchSize, totalPoints);

            // Write batch of points
            for (size_t j = i; j < endIndex; ++j)
            {
                if (!writer->writePoint(transformedPoints[j]))
                {
                    result.errorMessage = QString("Failed to write point %1: %2").arg(j).arg(writer->getLastError());
                    writer->close();
                    return result;
                }
                pointsWritten++;
            }

            // Update progress
            int progress = 40 + static_cast<int>((pointsWritten * 50) / totalPoints);
            emit progressUpdated(progress, QString("Writing points: %1/%2").arg(pointsWritten).arg(totalPoints));
        }

        emit progressUpdated(90, "Finalizing file...");

        // Close writer
        if (!writer->close())
        {
            result.errorMessage = QString("Failed to close file: %1").arg(writer->getLastError());
            return result;
        }

        emit progressUpdated(95, "Validating output...");

        // Validate output if requested
        if (options.validateOutput)
        {
            if (!validateExportedFile(options.outputPath, options))
            {
                result.errorMessage = "Output file validation failed";
                return result;
            }
        }

        emit progressUpdated(100, "Export completed successfully");

        // Fill result
        result.success = true;
        result.pointsExported = pointsWritten;
        result.exportTimeSeconds = timer.elapsed() / 1000.0;

        QFileInfo fileInfo(options.outputPath);
        result.fileSizeBytes = fileInfo.size();

        qDebug() << "PointCloudExporter: Successfully exported" << pointsWritten << "points to" << options.outputPath
                 << "in" << result.exportTimeSeconds << "seconds";

        return result;
    }
    catch (const std::exception& e)
    {
        result.errorMessage = QString("Export failed with exception: %1").arg(e.what());
        qWarning() << result.errorMessage;
        return result;
    }
}

void PointCloudExporter::exportPointCloudAsync(const std::vector<Point>& points, const ExportOptions& options)
{
    if (m_isExporting)
    {
        emit errorOccurred("Export already in progress");
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_isExporting = true;
        m_cancelRequested = false;
    }

    // Create worker thread
    m_workerThread = std::make_unique<QThread>();
    m_worker = std::make_unique<ExportWorker>(this);

    m_worker->setExportData(points, options);
    m_worker->moveToThread(m_workerThread.get());

    // Connect signals
    connect(m_workerThread.get(), &QThread::started, m_worker.get(), &ExportWorker::doExport);
    connect(m_worker.get(), &ExportWorker::progressUpdated, this, &PointCloudExporter::progressUpdated);
    connect(m_worker.get(), &ExportWorker::exportCompleted, this, &PointCloudExporter::onAsyncExportFinished);
    connect(m_workerThread.get(), &QThread::finished, m_workerThread.get(), &QThread::deleteLater);

    // Start export
    m_workerThread->start();
}

void PointCloudExporter::cancelExport()
{
    QMutexLocker locker(&m_mutex);
    m_cancelRequested = true;

    if (m_workerThread && m_workerThread->isRunning())
    {
        m_workerThread->quit();
        m_workerThread->wait(5000);  // Wait up to 5 seconds
    }
}

bool PointCloudExporter::isExporting() const
{
    QMutexLocker locker(&m_mutex);
    return m_isExporting;
}

QStringList PointCloudExporter::getSupportedFormats()
{
    return {"E57", "LAS", "PLY", "XYZ"};
}

QString PointCloudExporter::getFileExtension(ExportFormat format)
{
    switch (format)
    {
        case ExportFormat::E57:
            return ".e57";
        case ExportFormat::LAS:
            return ".las";
        case ExportFormat::PLY:
            return ".ply";
        case ExportFormat::XYZ:
            return ".xyz";
        default:
            return ".dat";
    }
}

QString PointCloudExporter::validateOptions(const ExportOptions& options)
{
    if (options.outputPath.isEmpty())
    {
        return "Output path is required";
    }

    QFileInfo fileInfo(options.outputPath);
    if (!fileInfo.dir().exists())
    {
        return "Output directory does not exist";
    }

    if (options.batchSize == 0)
    {
        return "Batch size must be greater than 0";
    }

    if (options.precision < 0 || options.precision > 15)
    {
        return "Precision must be between 0 and 15";
    }

    return QString();  // Valid
}

void PointCloudExporter::onAsyncExportFinished()
{
    {
        QMutexLocker locker(&m_mutex);
        m_isExporting = false;
    }

    // Clean up worker thread
    if (m_workerThread)
    {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread.reset();
        m_worker.reset();
    }
}

std::unique_ptr<IFormatWriter> PointCloudExporter::createWriter(ExportFormat format)
{
    switch (format)
    {
        case ExportFormat::E57:
            return std::make_unique<E57Writer>();
        case ExportFormat::LAS:
            return std::make_unique<LASWriter>();
        case ExportFormat::PLY:
            return std::make_unique<PLYWriter>();
        case ExportFormat::XYZ:
            return std::make_unique<XYZWriter>();
        default:
            return nullptr;
    }
}

std::vector<Point>
PointCloudExporter::transformCoordinates(const std::vector<Point>& points, const QString& fromCRS, const QString& toCRS)
{
    // For now, return points unchanged
    // In a full implementation, this would use a coordinate transformation library
    qDebug() << "PointCloudExporter: Coordinate transformation from" << fromCRS << "to" << toCRS << "not implemented";
    return points;
}

HeaderInfo PointCloudExporter::createHeaderInfo(const std::vector<Point>& points, const ExportOptions& options)
{
    HeaderInfo header;
    header.pointCount = points.size();
    header.projectName = options.projectName;
    header.description = options.description;
    header.hasColor = options.includeColor;
    header.hasIntensity = options.includeIntensity;

    if (!points.empty())
    {
        calculateBounds(points, header.minBounds, header.maxBounds);
    }

    return header;
}

void PointCloudExporter::calculateBounds(const std::vector<Point>& points, QVector3D& minBounds, QVector3D& maxBounds)
{
    if (points.empty())
    {
        minBounds = maxBounds = QVector3D(0, 0, 0);
        return;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& point : points)
    {
        minX = std::min(minX, point.x);
        minY = std::min(minY, point.y);
        minZ = std::min(minZ, point.z);
        maxX = std::max(maxX, point.x);
        maxY = std::max(maxY, point.y);
        maxZ = std::max(maxZ, point.z);
    }

    minBounds = QVector3D(minX, minY, minZ);
    maxBounds = QVector3D(maxX, maxY, maxZ);
}

bool PointCloudExporter::validateExportedFile(const QString& filePath, const ExportOptions& options)
{
    QFileInfo fileInfo(filePath);

    // Basic file existence and size check
    if (!fileInfo.exists())
    {
        qWarning() << "PointCloudExporter: Exported file does not exist:" << filePath;
        return false;
    }

    if (fileInfo.size() == 0)
    {
        qWarning() << "PointCloudExporter: Exported file is empty:" << filePath;
        return false;
    }

    // Format-specific validation could be added here
    qDebug() << "PointCloudExporter: File validation passed for" << filePath << "size:" << fileInfo.size() << "bytes";
    return true;
}

#include "PointCloudExporter.moc"
