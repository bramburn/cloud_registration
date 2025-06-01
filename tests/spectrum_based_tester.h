#ifndef SPECTRUM_BASED_TESTER_H
#define SPECTRUM_BASED_TESTER_H

#include <QObject>
#include <QString>
#include <QSet>
#include <QMap>
#include <QList>

/**
 * @brief Execution trace for spectrum-based fault localization
 */
struct ExecutionTrace {
    QString testName;
    QString filePath;
    QSet<QString> executedMethods;
    QSet<QString> failedMethods;
    bool testPassed;
    QString errorMessage;
};

/**
 * @brief Spectrum-Based Fault Localization Tester
 * 
 * Implements spectrum-based testing techniques for automated fault localization
 * in point cloud parsing components. Tracks method execution patterns to identify
 * suspicious code components when tests fail.
 */
class SpectrumBasedTester : public QObject
{
    Q_OBJECT

public:
    explicit SpectrumBasedTester(QObject *parent = nullptr);
    
    void recordExecution(const QString &testName, const QString &method, bool success);
    void analyzeSpectrumForFaultLocalization();
    QList<QString> getSuspiciousComponents();
    
    // Test case purification for better fault isolation
    QList<ExecutionTrace> purifyFailingTests(const QList<ExecutionTrace> &traces);

signals:
    void suspiciousComponentFound(const QString &component, double suspiciousness);
    void faultLocalizationCompleted(const QList<QString> &suspiciousComponents);

private:
    double calculateSuspiciousness(const QString &method);
    void generateFaultLocalizationReport();
    
    QList<ExecutionTrace> m_executionTraces;
    QMap<QString, double> m_suspiciousness;
    QSet<QString> m_allMethods;
};

#endif // SPECTRUM_BASED_TESTER_H
