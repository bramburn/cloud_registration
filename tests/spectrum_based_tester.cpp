#include "spectrum_based_tester.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

SpectrumBasedTester::SpectrumBasedTester(QObject *parent) : QObject(parent)
{
}

void SpectrumBasedTester::recordExecution(const QString &testName, const QString &method, bool success)
{
    // Find or create execution trace for this test
    ExecutionTrace *trace = nullptr;
    for (ExecutionTrace &t : m_executionTraces) {
        if (t.testName == testName) {
            trace = &t;
            break;
        }
    }
    
    if (!trace) {
        ExecutionTrace newTrace;
        newTrace.testName = testName;
        newTrace.testPassed = true; // Will be updated if any method fails
        m_executionTraces.append(newTrace);
        trace = &m_executionTraces.last();
    }
    
    // Record method execution
    trace->executedMethods.insert(method);
    m_allMethods.insert(method);
    
    if (!success) {
        trace->failedMethods.insert(method);
        trace->testPassed = false;
    }
}

void SpectrumBasedTester::analyzeSpectrumForFaultLocalization()
{
    qDebug() << "Analyzing spectrum for fault localization with" << m_executionTraces.size() << "traces";
    
    m_suspiciousness.clear();
    
    // Calculate suspiciousness for each method using Tarantula formula
    for (const QString &method : m_allMethods) {
        double suspiciousness = calculateSuspiciousness(method);
        m_suspiciousness[method] = suspiciousness;
    }
    
    // Sort by suspiciousness
    QList<QString> suspiciousComponents = getSuspiciousComponents();
    
    qDebug() << "Fault localization completed. Top suspicious components:";
    for (int i = 0; i < qMin(5, suspiciousComponents.size()); ++i) {
        QString component = suspiciousComponents[i];
        qDebug() << "  " << (i+1) << "." << component << "- suspiciousness:" << m_suspiciousness[component];
        emit suspiciousComponentFound(component, m_suspiciousness[component]);
    }
    
    emit faultLocalizationCompleted(suspiciousComponents);
}

double SpectrumBasedTester::calculateSuspiciousness(const QString &method)
{
    int failedExecuted = 0;    // Failed tests that executed this method
    int failedNotExecuted = 0; // Failed tests that did not execute this method
    int passedExecuted = 0;    // Passed tests that executed this method
    int passedNotExecuted = 0; // Passed tests that did not execute this method
    
    for (const ExecutionTrace &trace : m_executionTraces) {
        bool methodExecuted = trace.executedMethods.contains(method);
        
        if (trace.testPassed) {
            if (methodExecuted) {
                passedExecuted++;
            } else {
                passedNotExecuted++;
            }
        } else {
            if (methodExecuted) {
                failedExecuted++;
            } else {
                failedNotExecuted++;
            }
        }
    }
    
    // Tarantula suspiciousness formula
    double failedRatio = (failedExecuted + failedNotExecuted) > 0 ? 
                        (double)failedExecuted / (failedExecuted + failedNotExecuted) : 0.0;
    double passedRatio = (passedExecuted + passedNotExecuted) > 0 ? 
                        (double)passedExecuted / (passedExecuted + passedNotExecuted) : 0.0;
    
    if (failedRatio + passedRatio == 0.0) {
        return 0.0;
    }
    
    return failedRatio / (failedRatio + passedRatio);
}

QList<QString> SpectrumBasedTester::getSuspiciousComponents()
{
    QList<QString> components = m_suspiciousness.keys();
    
    // Sort by suspiciousness (descending)
    std::sort(components.begin(), components.end(), 
              [this](const QString &a, const QString &b) {
                  return m_suspiciousness[a] > m_suspiciousness[b];
              });
    
    return components;
}

QList<ExecutionTrace> SpectrumBasedTester::purifyFailingTests(const QList<ExecutionTrace> &traces)
{
    QList<ExecutionTrace> purifiedTraces;
    
    // Simple purification: remove traces that have too many executed methods
    // This helps focus on the core failing functionality
    
    for (const ExecutionTrace &trace : traces) {
        if (!trace.testPassed) {
            // For failing tests, create a purified version with only essential methods
            ExecutionTrace purified = trace;
            
            // Keep only methods that are commonly associated with failures
            QSet<QString> essentialMethods;
            for (const QString &method : trace.executedMethods) {
                if (method.contains("parse") || method.contains("load") || 
                    method.contains("read") || method.contains("extract")) {
                    essentialMethods.insert(method);
                }
            }
            
            purified.executedMethods = essentialMethods;
            purifiedTraces.append(purified);
        } else {
            // Keep passing tests as-is
            purifiedTraces.append(trace);
        }
    }
    
    return purifiedTraces;
}

void SpectrumBasedTester::generateFaultLocalizationReport()
{
    qDebug() << "=== Fault Localization Report ===";
    qDebug() << "Total execution traces:" << m_executionTraces.size();
    qDebug() << "Total methods analyzed:" << m_allMethods.size();
    
    int failedTests = 0;
    int passedTests = 0;
    
    for (const ExecutionTrace &trace : m_executionTraces) {
        if (trace.testPassed) {
            passedTests++;
        } else {
            failedTests++;
        }
    }
    
    qDebug() << "Passed tests:" << passedTests;
    qDebug() << "Failed tests:" << failedTests;
    
    if (failedTests > 0) {
        qDebug() << "Top 10 most suspicious methods:";
        QList<QString> suspicious = getSuspiciousComponents();
        for (int i = 0; i < qMin(10, suspicious.size()); ++i) {
            QString method = suspicious[i];
            qDebug() << QString("  %1. %2 (suspiciousness: %3)")
                        .arg(i+1)
                        .arg(method)
                        .arg(m_suspiciousness[method], 0, 'f', 3);
        }
    } else {
        qDebug() << "No failed tests - fault localization not applicable";
    }
    
    qDebug() << "=== End Report ===";
}
