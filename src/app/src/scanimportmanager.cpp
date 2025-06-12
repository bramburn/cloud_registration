#include "app/scanimportmanager.h"

ScanImportManager::ScanImportManager(QObject *parent)
    : QObject(parent)
{
}

void ScanImportManager::importScan(const QString& filePath)
{
    m_isImporting = true;
    emit importStarted(filePath);
    
    // Stub implementation - actual import logic will be added in future sprints
    emit importProgress(100, "Complete");
    emit importFinished(true, "Import completed successfully");
    m_isImporting = false;
}

void ScanImportManager::importScans(const QStringList& filePaths)
{
    for (const QString& filePath : filePaths) {
        importScan(filePath);
    }
}

void ScanImportManager::cancelImport()
{
    if (m_isImporting) {
        m_isImporting = false;
        emit importCancelled();
    }
}
