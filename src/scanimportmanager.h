#ifndef SCANIMPORTMANAGER_H
#define SCANIMPORTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

class SQLiteManager;
class QFileInfo;
struct ScanInfo;

enum class ImportMode {
    Copy,
    Move
};

struct ImportResult {
    bool success;
    QString errorMessage;
    QStringList successfulFiles;
    QStringList failedFiles;
    
    bool hasErrors() const { return !failedFiles.isEmpty(); }
    int totalFiles() const { return successfulFiles.size() + failedFiles.size(); }
};

class ScanImportManager : public QObject
{
    Q_OBJECT

public:
    explicit ScanImportManager(QObject *parent = nullptr);
    
    ImportResult importScans(const QStringList &filePaths, 
                           const QString &projectPath,
                           const QString &projectId,
                           ImportMode mode,
                           QWidget *parent = nullptr);
    
    void setSQLiteManager(SQLiteManager *manager);
    
    static bool isValidScanFile(const QString &filePath);
    static QStringList getSupportedExtensions();
    static QString getFileBaseName(const QString &filePath);

signals:
    void scansImported(const QList<ScanInfo> &scans);
    void importProgress(int current, int total, const QString &currentFile);
    void importFinished();

private:
    bool performFileOperation(const QString &sourcePath, 
                            const QString &targetPath, 
                            ImportMode mode);
    ScanInfo createScanInfo(const QString &filePath, 
                          const QString &projectPath,
                          const QString &projectId, 
                          ImportMode mode);
    QString getRelativePath(const QString &filePath, const QString &projectPath);
    
    SQLiteManager *m_sqliteManager;
    
    static const QStringList SUPPORTED_EXTENSIONS;
};

#endif // SCANIMPORTMANAGER_H
