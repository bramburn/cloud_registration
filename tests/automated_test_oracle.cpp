#include "automated_test_oracle.h"
#include "advanced_test_executor.h"
#include <QDebug>
#include <QJsonArray>
#include <cmath>
#include <algorithm>
#include <numeric>

AutomatedTestOracle::AutomatedTestOracle(QObject *parent) 
    : QObject(parent)
    , m_trainingSetSize(0)
{
    addCoordinateRangeInvariants();
    addPointCountInvariants();
    addDistributionInvariants();
    addPerformanceInvariants();
}

void AutomatedTestOracle::learnInvariants(const QList<TestResult> &knownGoodResults)
{
    qDebug() << "Learning invariants from" << knownGoodResults.size() << "known good results";
    
    m_trainingSetSize = knownGoodResults.size();
    QJsonObject patterns;
    
    // Analyze patterns in successful test results
    QList<double> loadTimes, memoryUsages;
    QList<int> pointCounts;
    QList<QPair<double, double>> coordinateRanges; // min, max pairs
    
    for (const TestResult &result : knownGoodResults) {
        if (!result.success) continue;
        
        loadTimes.append(result.loadTimeMs);
        memoryUsages.append(result.memoryUsageMB);
        pointCounts.append(result.pointsLoaded);
        
        // Analyze coordinate patterns from metadata if available
        if (result.metadata.contains("coordinateStats")) {
            QJsonObject stats = result.metadata["coordinateStats"].toObject();
            coordinateRanges.append({stats["minCoord"].toDouble(), 
                                   stats["maxCoord"].toDouble()});
        }
    }
    
    // Learn performance patterns
    if (!loadTimes.isEmpty()) {
        double avgLoadTime = std::accumulate(loadTimes.begin(), loadTimes.end(), 0.0) / loadTimes.size();
        double maxLoadTime = *std::max_element(loadTimes.begin(), loadTimes.end());
        
        patterns["performance"] = QJsonObject{
            {"averageLoadTimeMs", avgLoadTime},
            {"maxAcceptableLoadTimeMs", maxLoadTime * 1.5}, // 50% tolerance
            {"averageMemoryMB", std::accumulate(memoryUsages.begin(), memoryUsages.end(), 0.0) / memoryUsages.size()}
        };
        
        // Add performance invariant
        InvariantRule perfRule;
        perfRule.name = "PerformanceRegression";
        perfRule.description = "Loading time should not exceed learned maximum by more than 50%";
        perfRule.confidence = 0.85;
        perfRule.category = "performance";
        perfRule.validator = [maxLoadTime](const std::vector<float>&, const QJsonObject& metadata) {
            double loadTime = metadata["loadTimeMs"].toDouble();
            return loadTime <= maxLoadTime * 1.5;
        };
        
        m_invariants.append(perfRule);
        emit newInvariantLearned(perfRule.name);
    }
    
    // Learn point count patterns
    if (!pointCounts.isEmpty()) {
        int minPoints = *std::min_element(pointCounts.begin(), pointCounts.end());
        int maxPoints = *std::max_element(pointCounts.begin(), pointCounts.end());
        
        patterns["pointCounts"] = QJsonObject{
            {"minPoints", minPoints},
            {"maxPoints", maxPoints},
            {"averagePoints", std::accumulate(pointCounts.begin(), pointCounts.end(), 0) / pointCounts.size()}
        };
        
        // Add point count invariant
        InvariantRule countRule;
        countRule.name = "ReasonablePointCount";
        countRule.description = "Point count should be within learned reasonable range";
        countRule.confidence = 0.9;
        countRule.category = "count";
        countRule.validator = [minPoints, maxPoints](const std::vector<float>& points, const QJsonObject&) {
            int count = points.size() / 3;
            return count >= 0 && (count == 0 || (count >= minPoints * 0.1 && count <= maxPoints * 10));
        };
        
        m_invariants.append(countRule);
        emit newInvariantLearned(countRule.name);
    }
    
    m_learnedPatterns = patterns;
    qDebug() << "Learned" << m_invariants.size() << "invariants from training data";
}

QList<QString> AutomatedTestOracle::validateResult(const TestResult &result)
{
    QList<QString> violations;
    
    // Convert result to point data (this would need actual point data)
    std::vector<float> points; // In real implementation, extract from result
    
    for (const InvariantRule &rule : m_invariants) {
        if (!rule.validator(points, result.metadata)) {
            QString violation = QString("Invariant violation: %1 - %2")
                              .arg(rule.name, rule.description);
            violations.append(violation);
            emit invariantViolated(rule.name, violation);
        }
    }
    
    return violations;
}

void AutomatedTestOracle::addCoordinateRangeInvariants()
{
    // Invariant: Coordinates should be finite numbers
    InvariantRule finiteRule;
    finiteRule.name = "FiniteCoordinates";
    finiteRule.description = "All coordinate values must be finite (not NaN or infinite)";
    finiteRule.confidence = 1.0;
    finiteRule.category = "coordinate";
    finiteRule.validator = [](const std::vector<float>& points, const QJsonObject&) {
        for (float coord : points) {
            if (!std::isfinite(coord)) return false;
        }
        return true;
    };
    
    m_invariants.append(finiteRule);
    
    // Invariant: Point coordinates should come in XYZ triplets
    InvariantRule tripletRule;
    tripletRule.name = "XYZTriplets";
    tripletRule.description = "Point data should contain coordinates in XYZ triplets";
    tripletRule.confidence = 1.0;
    tripletRule.category = "coordinate";
    tripletRule.validator = [](const std::vector<float>& points, const QJsonObject&) {
        return points.size() % 3 == 0;
    };
    
    m_invariants.append(tripletRule);
}

void AutomatedTestOracle::addPointCountInvariants()
{
    // Invariant: Point count should be non-negative
    InvariantRule nonNegativeRule;
    nonNegativeRule.name = "NonNegativePointCount";
    nonNegativeRule.description = "Point count should be non-negative";
    nonNegativeRule.confidence = 1.0;
    nonNegativeRule.category = "count";
    nonNegativeRule.validator = [](const std::vector<float>& points, const QJsonObject&) {
        return points.size() >= 0;
    };
    
    m_invariants.append(nonNegativeRule);
}

void AutomatedTestOracle::addDistributionInvariants()
{
    // Invariant: Points should not all be identical (unless it's a degenerate case)
    InvariantRule diversityRule;
    diversityRule.name = "CoordinateDiversity";
    diversityRule.description = "Point cloud should contain some coordinate variation";
    diversityRule.confidence = 0.8;
    diversityRule.category = "distribution";
    diversityRule.validator = [](const std::vector<float>& points, const QJsonObject&) {
        if (points.size() < 9) return true; // Too few points to check diversity
        
        // Check if all points are identical
        bool allIdentical = true;
        for (size_t i = 3; i < points.size(); i += 3) {
            if (points[i] != points[0] || points[i+1] != points[1] || points[i+2] != points[2]) {
                allIdentical = false;
                break;
            }
        }
        
        return !allIdentical;
    };
    
    m_invariants.append(diversityRule);
}

void AutomatedTestOracle::addPerformanceInvariants()
{
    // Invariant: Loading should complete within reasonable time
    InvariantRule timeoutRule;
    timeoutRule.name = "ReasonableLoadTime";
    timeoutRule.description = "Loading should complete within reasonable time limits";
    timeoutRule.confidence = 0.9;
    timeoutRule.category = "performance";
    timeoutRule.validator = [](const std::vector<float>&, const QJsonObject& metadata) {
        double loadTime = metadata["loadTimeMs"].toDouble();
        return loadTime >= 0 && loadTime < 600000; // Less than 10 minutes
    };
    
    m_invariants.append(timeoutRule);
    
    // Invariant: Memory usage should be reasonable
    InvariantRule memoryRule;
    memoryRule.name = "ReasonableMemoryUsage";
    memoryRule.description = "Memory usage should be within reasonable limits";
    memoryRule.confidence = 0.8;
    memoryRule.category = "performance";
    memoryRule.validator = [](const std::vector<float>&, const QJsonObject& metadata) {
        double memoryMB = metadata["memoryUsageMB"].toDouble();
        return memoryMB >= 0 && memoryMB < 32000; // Less than 32GB
    };
    
    m_invariants.append(memoryRule);
}

void AutomatedTestOracle::detectCoordinateInvariants(const std::vector<float> &points)
{
    if (points.size() < 3) return;
    
    // Analyze coordinate ranges
    float minX = points[0], maxX = points[0];
    float minY = points[1], maxY = points[1];
    float minZ = points[2], maxZ = points[2];
    
    for (size_t i = 0; i < points.size(); i += 3) {
        minX = std::min(minX, points[i]);
        maxX = std::max(maxX, points[i]);
        minY = std::min(minY, points[i+1]);
        maxY = std::max(maxY, points[i+1]);
        minZ = std::min(minZ, points[i+2]);
        maxZ = std::max(maxZ, points[i+2]);
    }
    
    qDebug() << "Coordinate ranges - X:" << minX << "to" << maxX
             << ", Y:" << minY << "to" << maxY
             << ", Z:" << minZ << "to" << maxZ;
}

void AutomatedTestOracle::detectPerformanceInvariants(const TestResult &result)
{
    // Analyze performance characteristics
    if (result.success && result.pointsLoaded > 0 && result.loadTimeMs > 0) {
        double pointsPerSecond = (double)result.pointsLoaded / (result.loadTimeMs / 1000.0);
        qDebug() << "Performance: " << pointsPerSecond << "points/second";
        
        if (pointsPerSecond < 1000) {
            qWarning() << "Slow loading performance detected";
        }
    }
}

bool AutomatedTestOracle::analyzePointDistribution(const std::vector<float> &points)
{
    if (points.size() < 9) return true; // Too few points to analyze
    
    // Simple distribution analysis
    std::vector<float> distances;
    
    for (size_t i = 3; i < points.size(); i += 3) {
        float dx = points[i] - points[0];
        float dy = points[i+1] - points[1];
        float dz = points[i+2] - points[2];
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
        distances.push_back(distance);
    }
    
    if (!distances.empty()) {
        double avgDistance = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size();
        qDebug() << "Average point distance from first point:" << avgDistance;
        return avgDistance > 0.001; // Points should not be too clustered
    }
    
    return true;
}
