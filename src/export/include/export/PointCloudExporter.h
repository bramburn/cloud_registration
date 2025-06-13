#ifndef POINTCLOUDEXPORTER_H
#define POINTCLOUDEXPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMutex>
#include <QThread>
#include <memory>
#include <vector>
#include <functional>

#include "core/pointdata.h"
#include "export/ExportTypes.h"
#include "export/IFormatWriter.h"

/**
 * @brief PointCloudExporter - Manages point cloud export operations
 *
 * This class provides comprehensive point cloud export functionality with support
 * for multiple formats, asynchronous operations, and progress reporting.
 */
class PointCloudExporter : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudExporter(QObject* parent = nullptr);
    virtual ~PointCloudExporter();

    /**
     * @brief Export point cloud synchronously
     * @param points Vector of points to export
     * @param options Export options
     * @return Export result
     */
    ExportResult exportPointCloud(const std::vector<Point>& points, const ExportOptions& options);

    /**
     * @brief Export point cloud asynchronously
     * @param points Vector of points to export
     * @param options Export options
     */
    void exportPointCloudAsync(const std::vector<Point>& points, const ExportOptions& options);

    /**
     * @brief Cancel current export operation
     */
    void cancelExport();

    /**
     * @brief Check if export is in progress
     * @return true if exporting, false otherwise
     */
    bool isExporting() const { return m_isExporting; }

    /**
     * @brief Get list of supported export formats
     * @return List of supported formats
     */
    QStringList getSupportedFormats() const;

signals:
    void exportStarted(const QString& filePath);
    void progressUpdated(int percentage, const QString& stage);
    void exportFinished(const ExportResult& result);
    void exportCancelled();
    void errorOccurred(const QString& error);

private slots:
    void onWorkerFinished();

private:
    // Forward declaration of worker class
    class ExportWorker;

    // Helper methods
    QString validateOptions(const ExportOptions& options);
    std::unique_ptr<IFormatWriter> createWriter(ExportFormat format);
    std::vector<Point> transformCoordinates(const std::vector<Point>& points,
                                           const QMatrix4x4& transformation,
                                           CoordinateSystem targetSystem);
    HeaderInfo createHeaderInfo(const std::vector<Point>& points, const ExportOptions& options);
    bool validateExportedFile(const QString& filePath, const ExportOptions& options);
    void calculateBounds(const std::vector<Point>& points, QVector3D& minBounds, QVector3D& maxBounds);

    // Member variables
    bool m_isExporting;
    bool m_cancelRequested;
    QMutex m_mutex;
    std::unique_ptr<QThread> m_workerThread;
    std::unique_ptr<ExportWorker> m_worker;
};

#endif  // POINTCLOUDEXPORTER_H
