#ifndef SCANIMPORTMANAGER_H
#define SCANIMPORTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief ScanImportManager - Manages scan import operations
 *
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class ScanImportManager : public QObject
{
    Q_OBJECT

public:
    explicit ScanImportManager(QObject* parent = nullptr);
    virtual ~ScanImportManager() = default;

    // Basic import operations
    void importScan(const QString& filePath);
    void importScans(const QStringList& filePaths);
    void cancelImport();
    bool isImporting() const
    {
        return m_isImporting;
    }

signals:
    void importStarted(const QString& filePath);
    void importProgress(int percentage, const QString& stage);
    void importFinished(bool success, const QString& message);
    void importCancelled();

private:
    bool m_isImporting = false;
};

#endif  // SCANIMPORTMANAGER_H
