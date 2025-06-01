#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <stdexcept>
#include "project.h"

class ProjectCreationException : public std::runtime_error {
public:
    explicit ProjectCreationException(const QString &message) 
        : std::runtime_error(message.toStdString()) {}
};

class ProjectLoadException : public std::runtime_error {
public:
    explicit ProjectLoadException(const QString &message) 
        : std::runtime_error(message.toStdString()) {}
};

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);
    
    QString createProject(const QString &name, const QString &basePath);
    bool isValidProject(const QString &projectPath);
    ProjectInfo loadProject(const QString &projectPath);
    bool validateProjectMetadata(const QJsonObject &metadata);
    
    static QString getMetadataFilePath(const QString &projectPath);
    static bool isProjectDirectory(const QString &path);

private:
    bool createProjectMetadata(const QString &projectPath, const QString &projectName);
    QJsonObject readProjectMetadata(const QString &projectPath);
    bool validateDirectoryPermissions(const QString &path, bool requireWrite = false);
    
    static const QString METADATA_FILENAME;
    static const QString CURRENT_FORMAT_VERSION;
};

#endif // PROJECTMANAGER_H
