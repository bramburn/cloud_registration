#ifndef INTELLIGENT_BUG_MANAGER_H
#define INTELLIGENT_BUG_MANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>
#include <QMap>
#include <QNetworkAccessManager>

/**
 * @brief Enhanced bug report structure
 */
struct EnhancedBugReport {
    QString id;
    QString title;
    QString description;
    QString severity; // "Critical", "High", "Medium", "Low"
    QString priority; // Calculated based on multiple factors
    QString status;
    QString component;
    QString foundInVersion;
    QString assignedDeveloper;
    QStringList dependencies; // Other bug IDs this depends on
    QStringList blockedBy;    // Bug IDs that block this one
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime estimatedFixDate;
    QJsonObject aiAnalysis;   // AI-generated insights
    double similarityScores;  // For finding duplicate bugs
};

/**
 * @brief Intelligent Bug Management System for Sprint 2.4
 * 
 * Implements AI-enhanced bug tracking and management with:
 * - Automated severity prediction
 * - Developer assignment based on expertise
 * - Duplicate bug detection
 * - Dependency analysis and scheduling
 */
class IntelligentBugManager : public QObject
{
    Q_OBJECT

public:
    explicit IntelligentBugManager(QObject *parent = nullptr);
    
    // Enhanced bug lifecycle with AI assistance
    QString createBugReport(const EnhancedBugReport &bug);
    void triageBugWithAI(const QString &bugId);
    void analyzeBugDependencies();
    QList<QString> findSimilarBugs(const QString &bugId);
    
    // Automated severity prediction based on content analysis
    QString predictSeverity(const QString &title, const QString &description);
    
    // Developer assignment based on expertise and availability
    QString suggestDeveloper(const EnhancedBugReport &bug);
    
    // Schedule-aware bug fixing planning
    void generateFixingSchedule();

signals:
    void bugTriaged(const QString &bugId, const QString &newPriority);
    void duplicateBugFound(const QString &bugId, const QString &duplicateOf);
    void scheduleUpdated();

private:
    void analyzeTextContent(const QString &text, QJsonObject &analysis);
    double calculateBugSimilarity(const EnhancedBugReport &bug1, const EnhancedBugReport &bug2);
    void buildDependencyGraph();
    
    QMap<QString, EnhancedBugReport> m_bugs;
    QMap<QString, QStringList> m_developerExpertise;
    QMap<QString, QDateTime> m_developerAvailability;
    QNetworkAccessManager *m_networkManager;
};

#endif // INTELLIGENT_BUG_MANAGER_H
