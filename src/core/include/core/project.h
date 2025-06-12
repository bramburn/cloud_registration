#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDateTime>
#include <QObject>

struct ProjectInfo {
    QString projectId;
    QString projectName;
    QString creationDate;
    QString fileFormatVersion;
    QString projectPath;
    
    bool isValid() const {
        return !projectId.isEmpty() && !projectName.isEmpty() && 
               !creationDate.isEmpty() && !fileFormatVersion.isEmpty();
    }
};

class Project : public QObject
{
    Q_OBJECT

public:
    explicit Project(const ProjectInfo &info, QObject *parent = nullptr);
    
    // Getters
    QString projectId() const { return m_info.projectId; }
    QString projectName() const { return m_info.projectName; }
    QString creationDate() const { return m_info.creationDate; }
    QString fileFormatVersion() const { return m_info.fileFormatVersion; }
    QString projectPath() const { return m_info.projectPath; }
    
    // Validation
    bool isValid() const { return m_info.isValid(); }
    
    // Get full project info
    const ProjectInfo& getProjectInfo() const { return m_info; }

private:
    ProjectInfo m_info;
};

#endif // PROJECT_H
