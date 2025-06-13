#include "core/project.h"
#include <QUuid>

Project::Project(const ProjectInfo& info, QObject* parent) : QObject(parent), m_info(info), m_isModified(false) {}

Project::Project(QObject* parent) : QObject(parent), m_isModified(false)
{
    // Initialize with default project info
    m_info.projectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_info.projectName = "Untitled Project";
    m_info.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_info.fileFormatVersion = "1.0";
    m_info.projectPath = "";
}

Project::Project(const QString& name, const QString& path, QObject* parent) : QObject(parent), m_isModified(false)
{
    // Initialize with provided name and path
    m_info.projectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_info.projectName = name;
    m_info.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_info.fileFormatVersion = "1.0";
    m_info.projectPath = path;
}

void Project::markAsModified()
{
    if (!m_isModified)
    {
        m_isModified = true;
        emit projectModified();
    }
}

QVariantMap Project::serialize() const
{
    QVariantMap data;
    data["projectId"] = m_info.projectId;
    data["projectName"] = m_info.projectName;
    data["creationDate"] = m_info.creationDate;
    data["fileFormatVersion"] = m_info.fileFormatVersion;
    data["projectPath"] = m_info.projectPath;
    return data;
}

bool Project::deserialize(const QVariantMap& data)
{
    m_info.projectId = data.value("projectId").toString();
    m_info.projectName = data.value("projectName").toString();
    m_info.creationDate = data.value("creationDate").toString();
    m_info.fileFormatVersion = data.value("fileFormatVersion").toString();
    m_info.projectPath = data.value("projectPath").toString();

    m_isModified = false;
    return m_info.isValid();
}

bool Project::validate() const
{
    return m_info.isValid();
}
