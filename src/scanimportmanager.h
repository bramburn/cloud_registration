#ifndef SCANIMPORTMANAGER_H
#define SCANIMPORTMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

class SQLiteManager;
class QFileInfo;
class ProjectTreeModel;
class E57DataManager;
struct ScanInfo;

enum class ImportMode {
    Copy,
    Move,
    Link  // New for Sprint 2.2 - Link to Source
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
    void setProjectTreeModel(ProjectTreeModel *model);

    // Sprint 1.3: E57-specific import methods
    void handleE57Import(const QString& filePath);

    static bool isValidScanFile(const QString &filePath);
    static QStringList getSupportedExtensions();
    static QString getFileBaseName(const QString &filePath);

signals:
    void scansImported(const QList<ScanInfo> &scans);
    void importProgress(int current, int total, const QString &currentFile);
    void importFinished();

    // Sprint 1.3: E57-specific signals
    void importCompleted(const QString& filePath, int scanCount);
    void importFailed(const QString& filePath, const QString& error);

private:
    bool performFileOperation(const QString &sourcePath,
                            const QString &targetPath,
                            ImportMode mode);
    ScanInfo createScanInfo(const QString &sourcePath,
                          const QString &targetPath,
                          const QString &projectPath,
                          const QString &projectId,
                          ImportMode mode);
    QString getRelativePath(const QString &filePath, const QString &projectPath);

    // Sprint 1.3: E57-specific helper methods
    void handleE57ImportError(const QString& filePath, const QString& error);

    SQLiteManager *m_sqliteManager;
    ProjectTreeModel *m_projectTreeModel;
    QWidget *m_parentWidget;

    static const QStringList SUPPORTED_EXTENSIONS;
};

#endif // SCANIMPORTMANAGER_H
