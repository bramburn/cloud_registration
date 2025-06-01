#ifndef AUTOMATED_TEST_ORACLE_H
#define AUTOMATED_TEST_ORACLE_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>
#include <functional>
#include <vector>

/**
 * @brief Invariant rule for automated test validation
 */
struct InvariantRule {
    QString name;
    QString description;
    std::function<bool(const std::vector<float>&, const QJsonObject&)> validator;
    double confidence;
    QString category; // "coordinate", "count", "range", "distribution"
};

/**
 * @brief Test result forward declaration
 */
struct TestResult;

/**
 * @brief Automated Test Oracle for Sprint 2.4
 * 
 * Implements automated test oracle generation with invariant detection
 * to automatically identify expected behaviors and catch subtle bugs.
 * 
 * Based on research in automated test oracle generation for improved
 * test reliability and bug detection capabilities.
 */
class AutomatedTestOracle : public QObject
{
    Q_OBJECT

public:
    explicit AutomatedTestOracle(QObject *parent = nullptr);
    
    // Learn invariants from known good test results
    void learnInvariants(const QList<TestResult> &knownGoodResults);
    
    // Validate new results against learned invariants
    QList<QString> validateResult(const TestResult &result);
    
    // Specific invariant categories for point cloud data
    void addCoordinateRangeInvariants();
    void addPointCountInvariants();
    void addDistributionInvariants();
    void addPerformanceInvariants();

signals:
    void invariantViolated(const QString &invariantName, const QString &details);
    void newInvariantLearned(const QString &invariantName);

private:
    void detectCoordinateInvariants(const std::vector<float> &points);
    void detectPerformanceInvariants(const TestResult &result);
    bool analyzePointDistribution(const std::vector<float> &points);
    
    QList<InvariantRule> m_invariants;
    QJsonObject m_learnedPatterns;
    int m_trainingSetSize;
};

#endif // AUTOMATED_TEST_ORACLE_H
