#include "projectmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>

const QString ProjectManager::METADATA_FILENAME = "project_meta.json";
const QString ProjectManager::CURRENT_FORMAT_VERSION = "1.0.0";

ProjectManager::ProjectManager(QObject *parent) : QObject(parent)
{
}

QString ProjectManager::createProject(const QString &name, const QString &basePath)
{
    // Input validation
    if (name.trimmed().isEmpty()) {
        throw ProjectCreationException("Project name cannot be empty");
    }
    
    if (basePath.isEmpty()) {
        throw ProjectCreationException("Base path cannot be empty");
    }
    
    QDir baseDir(basePath);
    if (!baseDir.exists()) {
        throw ProjectCreationException(QString("Base directory does not exist: %1").arg(basePath));
    }
    
    if (!validateDirectoryPermissions(basePath, true)) {
        throw ProjectCreationException(QString("No write permission for directory: %1").arg(basePath));
    }
    
    QString projectPath = baseDir.absoluteFilePath(name.trimmed());
    QDir projectDir;
    
    // Create project directory
    if (!projectDir.mkpath(projectPath)) {
        throw ProjectCreationException(QString("Failed to create project directory: %1").arg(projectPath));
    }
    
    // Verify directory was created and is writable
    if (!validateDirectoryPermissions(projectPath, true)) {
        // Cleanup
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException(QString("Created directory is not writable: %1").arg(projectPath));
    }
    
    // Create metadata file
    try {
        if (!createProjectMetadata(projectPath, name.trimmed())) {
            // Cleanup on failure
            QDir(projectPath).removeRecursively();
            throw ProjectCreationException("Failed to create project metadata file");
        }
    } catch (...) {
        // Cleanup on any exception
        QDir(projectPath).removeRecursively();
        throw;
    }
    
    // Final validation
    if (!isValidProject(projectPath)) {
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException("Project validation failed after creation");
    }
    
    qInfo() << "Project created successfully:" << projectPath;
    return projectPath;
}

bool ProjectManager::isValidProject(const QString &projectPath)
{
    if (projectPath.isEmpty()) {
        return false;
    }
    
    QDir projectDir(projectPath);
    if (!projectDir.exists()) {
        return false;
    }
    
    QString metadataPath = getMetadataFilePath(projectPath);
    if (!QFile::exists(metadataPath)) {
        return false;
    }
    
    try {
        QJsonObject metadata = readProjectMetadata(projectPath);
        return validateProjectMetadata(metadata);
    } catch (...) {
        return false;
    }
}

ProjectInfo ProjectManager::loadProject(const QString &projectPath)
{
    if (!isValidProject(projectPath)) {
        throw ProjectLoadException(QString("Invalid project directory: %1").arg(projectPath));
    }
    
    try {
        QJsonObject metadata = readProjectMetadata(projectPath);
        
        ProjectInfo info;
        info.projectId = metadata["projectID"].toString();
        info.projectName = metadata["projectName"].toString();
        info.creationDate = metadata["creationDate"].toString();
        info.fileFormatVersion = metadata["fileFormatVersion"].toString();
        info.projectPath = projectPath;
        
        if (!info.isValid()) {
            throw ProjectLoadException("Project metadata is incomplete or invalid");
        }
        
        qInfo() << "Project loaded successfully:" << info.projectName;
        return info;
        
    } catch (const ProjectLoadException&) {
        throw;
    } catch (const std::exception& e) {
        throw ProjectLoadException(QString("Failed to load project: %1").arg(e.what()));
    }
}

bool ProjectManager::validateProjectMetadata(const QJsonObject &metadata)
{
    QStringList requiredFields = {"projectID", "projectName", "creationDate", "fileFormatVersion"};
    
    for (const QString &field : requiredFields) {
        if (!metadata.contains(field) || metadata[field].toString().isEmpty()) {
            qWarning() << "Missing or empty required field:" << field;
            return false;
        }
    }
    
    // Validate UUID format
    QString projectId = metadata["projectID"].toString();
    QUuid uuid(projectId);
    if (uuid.isNull()) {
        qWarning() << "Invalid UUID format for projectID:" << projectId;
        return false;
    }
    
    return true;
}

QString ProjectManager::getMetadataFilePath(const QString &projectPath)
{
    return QDir(projectPath).absoluteFilePath(METADATA_FILENAME);
}

bool ProjectManager::isProjectDirectory(const QString &path)
{
    return QFile::exists(getMetadataFilePath(path));
}

bool ProjectManager::createProjectMetadata(const QString &projectPath, const QString &projectName)
{
    QString metadataPath = getMetadataFilePath(projectPath);
    
    QJsonObject metadata;
    metadata["projectID"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    metadata["projectName"] = projectName;
    metadata["creationDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["fileFormatVersion"] = CURRENT_FORMAT_VERSION;
    
    QJsonDocument doc(metadata);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open metadata file for writing:" << metadataPath;
        return false;
    }
    
    qint64 bytesWritten = file.write(doc.toJson());
    if (bytesWritten == -1) {
        qCritical() << "Failed to write metadata to file:" << metadataPath;
        return false;
    }
    
    file.close();
    
    // Verify file was written correctly
    if (!QFile::exists(metadataPath)) {
        qCritical() << "Metadata file does not exist after writing:" << metadataPath;
        return false;
    }
    
    qDebug() << "Project metadata created successfully:" << metadataPath;
    return true;
}

QJsonObject ProjectManager::readProjectMetadata(const QString &projectPath)
{
    QString metadataPath = getMetadataFilePath(projectPath);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw ProjectLoadException(QString("Cannot open metadata file: %1").arg(metadataPath));
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        throw ProjectLoadException(QString("Invalid JSON in metadata file: %1").arg(error.errorString()));
    }
    
    if (!doc.isObject()) {
        throw ProjectLoadException("Metadata file does not contain a JSON object");
    }
    
    return doc.object();
}

bool ProjectManager::validateDirectoryPermissions(const QString &path, bool requireWrite)
{
    QFileInfo info(path);
    
    if (!info.exists()) {
        return false;
    }
    
    if (!info.isDir()) {
        return false;
    }
    
    if (!info.isReadable()) {
        return false;
    }
    
    if (requireWrite && !info.isWritable()) {
        return false;
    }
    
    return true;
}
