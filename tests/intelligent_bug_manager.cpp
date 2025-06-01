#include "intelligent_bug_manager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QJsonArray>
#include <algorithm>

IntelligentBugManager::IntelligentBugManager(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    
    // Initialize developer expertise database
    m_developerExpertise["john.doe"] = {"E57Parser", "compression", "bitPackCodec"};
    m_developerExpertise["jane.smith"] = {"LasParser", "coordinates", "PDRF"};
    m_developerExpertise["mike.jones"] = {"UI", "OpenGL", "visualization"};
}

QString IntelligentBugManager::createBugReport(const EnhancedBugReport &bug)
{
    EnhancedBugReport enhancedBug = bug;
    enhancedBug.id = QString("BUG_%1").arg(QDateTime::currentMSecsSinceEpoch());
    enhancedBug.createdAt = QDateTime::currentDateTime();
    
    // AI-enhanced analysis
    QJsonObject aiAnalysis;
    analyzeTextContent(bug.title + " " + bug.description, aiAnalysis);
    enhancedBug.aiAnalysis = aiAnalysis;
    
    // Predict severity if not provided
    if (enhancedBug.severity.isEmpty()) {
        enhancedBug.severity = predictSeverity(bug.title, bug.description);
    }
    
    // Find similar bugs
    QList<QString> similarBugs = findSimilarBugs(enhancedBug.id);
    if (!similarBugs.isEmpty()) {
        enhancedBug.aiAnalysis["possibleDuplicates"] = QJsonArray::fromStringList(similarBugs);
    }
    
    // Suggest developer assignment
    enhancedBug.assignedDeveloper = suggestDeveloper(enhancedBug);
    
    m_bugs[enhancedBug.id] = enhancedBug;
    
    qDebug() << "Created bug report:" << enhancedBug.id << "with severity:" << enhancedBug.severity;
    return enhancedBug.id;
}

QString IntelligentBugManager::predictSeverity(const QString &title, const QString &description)
{
    QString text = (title + " " + description).toLower();
    
    // Critical indicators
    if (text.contains("crash") || text.contains("segfault") || 
        text.contains("memory leak") || text.contains("data loss")) {
        return "Critical";
    }
    
    // High severity indicators
    if (text.contains("performance") || text.contains("slow") ||
        text.contains("hang") || text.contains("freeze") ||
        text.contains("incorrect") || text.contains("wrong")) {
        return "High";
    }
    
    // Medium severity indicators
    if (text.contains("ui") || text.contains("display") ||
        text.contains("warning") || text.contains("minor")) {
        return "Medium";
    }
    
    // Default to Low
    return "Low";
}

QString IntelligentBugManager::suggestDeveloper(const EnhancedBugReport &bug)
{
    QMap<QString, double> developerScores;
    
    // Analyze bug content to match with developer expertise
    QString bugContent = (bug.title + " " + bug.description + " " + bug.component).toLower();
    
    for (auto it = m_developerExpertise.begin(); it != m_developerExpertise.end(); ++it) {
        QString developer = it.key();
        QStringList expertise = it.value();
        
        double score = 0.0;
        
        // Calculate expertise match score
        for (const QString &skill : expertise) {
            if (bugContent.contains(skill.toLower())) {
                score += 1.0;
            }
        }
        
        // Factor in component match
        if (bug.component.toLower().contains("e57") && expertise.contains("E57Parser")) {
            score += 2.0;
        } else if (bug.component.toLower().contains("las") && expertise.contains("LasParser")) {
            score += 2.0;
        } else if (bug.component.toLower().contains("ui") && expertise.contains("UI")) {
            score += 2.0;
        }
        
        // Consider developer availability (simple check)
        if (m_developerAvailability.contains(developer)) {
            QDateTime availableDate = m_developerAvailability[developer];
            if (availableDate <= QDateTime::currentDateTime()) {
                score += 0.5; // Bonus for immediate availability
            }
        }
        
        developerScores[developer] = score;
    }
    
    // Find developer with highest score
    QString bestDeveloper;
    double bestScore = -1.0;
    
    for (auto it = developerScores.begin(); it != developerScores.end(); ++it) {
        if (it.value() > bestScore) {
            bestScore = it.value();
            bestDeveloper = it.key();
        }
    }
    
    return bestDeveloper;
}

QList<QString> IntelligentBugManager::findSimilarBugs(const QString &bugId)
{
    QList<QString> similarBugs;
    
    if (!m_bugs.contains(bugId)) {
        return similarBugs;
    }
    
    const EnhancedBugReport &targetBug = m_bugs[bugId];
    const double SIMILARITY_THRESHOLD = 0.7;
    
    for (auto it = m_bugs.begin(); it != m_bugs.end(); ++it) {
        if (it.key() == bugId) continue;
        
        double similarity = calculateBugSimilarity(targetBug, it.value());
        if (similarity >= SIMILARITY_THRESHOLD) {
            similarBugs.append(it.key());
        }
    }
    
    return similarBugs;
}

double IntelligentBugManager::calculateBugSimilarity(const EnhancedBugReport &bug1, const EnhancedBugReport &bug2)
{
    // Simple similarity calculation based on title and description overlap
    QStringList words1 = (bug1.title + " " + bug1.description).split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    QStringList words2 = (bug2.title + " " + bug2.description).split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    
    QSet<QString> set1 = QSet<QString>(words1.begin(), words1.end());
    QSet<QString> set2 = QSet<QString>(words2.begin(), words2.end());
    
    QSet<QString> intersection = set1.intersect(set2);
    QSet<QString> unionSet = set1.unite(set2);
    
    if (unionSet.isEmpty()) return 0.0;
    
    double jaccardSimilarity = (double)intersection.size() / unionSet.size();
    
    // Boost similarity if components match
    if (bug1.component == bug2.component) {
        jaccardSimilarity += 0.2;
    }
    
    return qMin(jaccardSimilarity, 1.0);
}

void IntelligentBugManager::triageBugWithAI(const QString &bugId)
{
    if (!m_bugs.contains(bugId)) {
        qWarning() << "Bug not found:" << bugId;
        return;
    }
    
    EnhancedBugReport &bug = m_bugs[bugId];
    
    // AI-enhanced triage
    QJsonObject analysis;
    analyzeTextContent(bug.title + " " + bug.description, analysis);
    
    // Update priority based on analysis
    int priorityScore = 0;
    
    if (bug.severity == "Critical") priorityScore += 10;
    else if (bug.severity == "High") priorityScore += 7;
    else if (bug.severity == "Medium") priorityScore += 4;
    else priorityScore += 1;
    
    // Increase priority if this bug blocks others
    for (auto it = m_bugs.begin(); it != m_bugs.end(); ++it) {
        if (it.value().blockedBy.contains(bugId)) {
            priorityScore += 2;
        }
    }
    
    bug.priority = QString::number(priorityScore);
    bug.updatedAt = QDateTime::currentDateTime();
    
    emit bugTriaged(bugId, bug.priority);
    
    qDebug() << "Bug triaged:" << bugId << "Priority:" << bug.priority;
}

void IntelligentBugManager::analyzeBugDependencies()
{
    buildDependencyGraph();
    
    // Identify critical path for bug fixing
    QMap<QString, int> bugPriorities;
    
    for (auto it = m_bugs.begin(); it != m_bugs.end(); ++it) {
        const EnhancedBugReport &bug = it.value();
        
        // Calculate priority based on dependencies
        int priority = 0;
        
        if (bug.severity == "Critical") priority += 10;
        else if (bug.severity == "High") priority += 7;
        else if (bug.severity == "Medium") priority += 4;
        else priority += 1;
        
        // Increase priority if this bug blocks others
        for (auto depIt = m_bugs.begin(); depIt != m_bugs.end(); ++depIt) {
            if (depIt.value().blockedBy.contains(it.key())) {
                priority += 2;
            }
        }
        
        bugPriorities[it.key()] = priority;
    }
    
    // Update bug priorities
    for (auto it = bugPriorities.begin(); it != bugPriorities.end(); ++it) {
        if (m_bugs.contains(it.key())) {
            EnhancedBugReport &bug = m_bugs[it.key()];
            bug.priority = QString::number(it.value());
            emit bugTriaged(it.key(), bug.priority);
        }
    }
    
    qDebug() << "Bug dependency analysis completed";
}

void IntelligentBugManager::buildDependencyGraph()
{
    // Simple dependency graph building
    // In a real implementation, this would analyze code dependencies,
    // test failures, and other relationships
    
    qDebug() << "Building bug dependency graph for" << m_bugs.size() << "bugs";
    
    // For now, just log the dependencies that are already set
    for (auto it = m_bugs.begin(); it != m_bugs.end(); ++it) {
        const EnhancedBugReport &bug = it.value();
        if (!bug.dependencies.isEmpty() || !bug.blockedBy.isEmpty()) {
            qDebug() << "Bug" << it.key() << "depends on:" << bug.dependencies
                     << "blocked by:" << bug.blockedBy;
        }
    }
}

void IntelligentBugManager::analyzeTextContent(const QString &text, QJsonObject &analysis)
{
    // Simple text analysis
    analysis["wordCount"] = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    analysis["containsCrash"] = text.toLower().contains("crash");
    analysis["containsPerformance"] = text.toLower().contains("performance");
    analysis["containsMemory"] = text.toLower().contains("memory");
    analysis["containsE57"] = text.toLower().contains("e57");
    analysis["containsLAS"] = text.toLower().contains("las");
}

void IntelligentBugManager::generateFixingSchedule()
{
    qDebug() << "Generating bug fixing schedule...";
    
    // Sort bugs by priority
    QList<QString> bugIds = m_bugs.keys();
    std::sort(bugIds.begin(), bugIds.end(), [this](const QString &a, const QString &b) {
        return m_bugs[a].priority.toInt() > m_bugs[b].priority.toInt();
    });
    
    qDebug() << "Bug fixing priority order:";
    for (int i = 0; i < bugIds.size(); ++i) {
        const EnhancedBugReport &bug = m_bugs[bugIds[i]];
        qDebug() << QString("  %1. %2 (Priority: %3, Severity: %4, Assigned: %5)")
                    .arg(i+1)
                    .arg(bug.title)
                    .arg(bug.priority)
                    .arg(bug.severity)
                    .arg(bug.assignedDeveloper);
    }
    
    emit scheduleUpdated();
}
