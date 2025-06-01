#include "performance_profiler.h"
#include <QStandardPaths>
#include <QDir>
#include <algorithm>

PerformanceProfiler& PerformanceProfiler::instance()
{
    static PerformanceProfiler instance;
    return instance;
}

PerformanceProfiler::PerformanceProfiler(QObject *parent) 
    : QObject(parent), m_enabled(true)
{
    m_globalTimer.start();
    qDebug() << "PerformanceProfiler initialized";
}

void PerformanceProfiler::startSection(const QString &sectionName)
{
    if (!m_enabled) {
        return;
    }
    
    // Initialize section if it doesn't exist
    if (!m_sections.contains(sectionName)) {
        m_sections[sectionName] = ProfileSection();
        m_sections[sectionName].name = sectionName;
    }
    
    // Start timing
    m_activeTimers[sectionName].start();
    m_sections[sectionName].startTime = m_globalTimer.elapsed();
}

void PerformanceProfiler::endSection(const QString &sectionName)
{
    if (!m_enabled) {
        return;
    }
    
    if (!m_activeTimers.contains(sectionName)) {
        qWarning() << "PerformanceProfiler: Attempted to end section that was not started:" << sectionName;
        return;
    }
    
    // Calculate elapsed time
    qint64 elapsed = m_activeTimers[sectionName].elapsed();
    m_activeTimers.remove(sectionName);
    
    // Update section statistics
    ProfileSection &section = m_sections[sectionName];
    section.elapsedMs = elapsed;
    section.callCount++;
    section.totalTime += elapsed;
    section.minTime = qMin(section.minTime, elapsed);
    section.maxTime = qMax(section.maxTime, elapsed);
}

void PerformanceProfiler::generateReport(const QString &filePath)
{
    if (m_sections.isEmpty()) {
        qDebug() << "PerformanceProfiler: No profiling data to report";
        return;
    }
    
    QString textReport = generateTextReport();
    QJsonObject jsonReport = generateJsonReport();
    
    if (filePath.isEmpty()) {
        // Output to debug console
        qDebug().noquote() << "=== PERFORMANCE PROFILING REPORT ===";
        qDebug().noquote() << textReport;
    } else {
        // Save to file
        QFileInfo fileInfo(filePath);
        QString basePath = fileInfo.absolutePath();
        QString baseName = fileInfo.completeBaseName();
        QString extension = fileInfo.suffix();
        
        // Ensure directory exists
        QDir().mkpath(basePath);
        
        // Save text report
        QString textFilePath = QString("%1/%2.txt").arg(basePath, baseName);
        QFile textFile(textFilePath);
        if (textFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&textFile);
            stream << "=== PERFORMANCE PROFILING REPORT ===\n";
            stream << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
            stream << textReport;
            qDebug() << "Performance report saved to:" << textFilePath;
        } else {
            qWarning() << "Failed to save text report to:" << textFilePath;
        }
        
        // Save JSON report
        QString jsonFilePath = QString("%1/%2.json").arg(basePath, baseName);
        QFile jsonFile(jsonFilePath);
        if (jsonFile.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(jsonReport);
            jsonFile.write(doc.toJson());
            qDebug() << "Performance JSON data saved to:" << jsonFilePath;
        } else {
            qWarning() << "Failed to save JSON report to:" << jsonFilePath;
        }
    }
}

void PerformanceProfiler::reset()
{
    m_sections.clear();
    m_activeTimers.clear();
    m_globalTimer.restart();
    qDebug() << "PerformanceProfiler: All profiling data reset";
}

ProfileSection PerformanceProfiler::getSection(const QString &sectionName) const
{
    return m_sections.value(sectionName, ProfileSection());
}

QString PerformanceProfiler::generateTextReport() const
{
    QString report;
    QTextStream stream(&report);
    
    // Sort sections by total time (descending)
    QList<ProfileSection> sortedSections = m_sections.values();
    std::sort(sortedSections.begin(), sortedSections.end(), 
              [](const ProfileSection &a, const ProfileSection &b) {
                  return a.totalTime > b.totalTime;
              });
    
    // Calculate total time for percentage calculations
    qint64 totalTime = 0;
    for (const auto &section : sortedSections) {
        totalTime += section.totalTime;
    }
    
    // Header
    stream << QString("Total Profiling Time: %1 ms\n").arg(totalTime);
    stream << QString("Number of Sections: %1\n\n").arg(sortedSections.size());
    
    // Table header
    stream << QString("%1 %2 %3 %4 %5 %6 %7\n")
              .arg("Section Name", -40)
              .arg("Calls", 8)
              .arg("Total(ms)", 8)
              .arg("Avg(ms)", 8)
              .arg("Min(ms)", 8)
              .arg("Max(ms)", 8)
              .arg("% Total", 8);
    stream << QString("-").repeated(90) << "\n";
    
    // Section data
    for (const auto &section : sortedSections) {
        double percentage = totalTime > 0 ? (double)section.totalTime * 100.0 / totalTime : 0.0;
        double avgTime = section.averageTime();
        
        stream << QString("%1 %2 %3 %4 %5 %6 %7\n")
                  .arg(section.name.left(40), -40)
                  .arg(section.callCount, 8)
                  .arg(section.totalTime, 8)
                  .arg(avgTime, 8, 'f', 2)
                  .arg(section.minTime == LLONG_MAX ? 0 : section.minTime, 8)
                  .arg(section.maxTime, 8)
                  .arg(QString::number(percentage, 'f', 1) + "%", 8);
    }
    
    return report;
}

QJsonObject PerformanceProfiler::generateJsonReport() const
{
    QJsonObject report;
    QJsonArray sections;
    
    // Add metadata
    report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["totalSections"] = m_sections.size();
    
    // Calculate total time
    qint64 totalTime = 0;
    for (const auto &section : m_sections) {
        totalTime += section.totalTime;
    }
    report["totalTime"] = totalTime;
    
    // Add section data
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const ProfileSection &section = it.value();
        QJsonObject sectionObj;
        
        sectionObj["name"] = section.name;
        sectionObj["callCount"] = section.callCount;
        sectionObj["totalTime"] = section.totalTime;
        sectionObj["averageTime"] = section.averageTime();
        sectionObj["minTime"] = section.minTime == LLONG_MAX ? 0 : section.minTime;
        sectionObj["maxTime"] = section.maxTime;
        sectionObj["percentage"] = totalTime > 0 ? (double)section.totalTime * 100.0 / totalTime : 0.0;
        
        sections.append(sectionObj);
    }
    
    report["sections"] = sections;
    return report;
}
