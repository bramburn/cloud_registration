#ifndef POINTCLOUDEXPORTER_H
#define POINTCLOUDEXPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief PointCloudExporter - Manages point cloud export operations
 * 
 * This is a stub implementation for Sprint 7 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class PointCloudExporter : public QObject {
    Q_OBJECT

public:
    explicit PointCloudExporter(QObject *parent = nullptr);
    virtual ~PointCloudExporter() = default;

    // Basic export methods
    bool exportPointCloud(const QString& filePath, const QString& format);
    QStringList getSupportedFormats() const;
    bool isExporting() const { return m_isExporting; }

signals:
    void exportStarted(const QString& filePath);
    void exportProgress(int percentage, const QString& stage);
    void exportFinished(bool success, const QString& message);
    void exportCancelled();

private:
    bool m_isExporting = false;
};

#endif // POINTCLOUDEXPORTER_H
