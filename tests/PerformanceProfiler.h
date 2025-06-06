#ifndef PERFORMANCEPROFILER_H
#define PERFORMANCEPROFILER_H

#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QJsonObject>
#include <vector>
#include <memory>

// Forward declaration
class E57ParserLib;

/**
 * @brief Performance profiling system for E57 library integration
 * 
 * Implements Sprint 4 User Story 2: Profile and Optimize E57 Loading Performance
 * Provides comprehensive performance monitoring, bottleneck identification,
 * and optimization comparison capabilities.
 */
class PerformanceProfiler : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Comprehensive performance metrics
     */
    struct PerformanceMetrics {
        QString fileName;
        int64_t fileSize = 0;
        int64_t pointCount = 0;
        double totalLoadTime = 0.0;
        double xmlParseTime = 0.0;
        double binaryReadTime = 0.0;
        double dataConversionTime = 0.0;
        size_t peakMemoryUsage = 0;
        size_t finalMemoryUsage = 0;
        double pointsPerSecond = 0.0;
        double memoryEfficiency = 0.0; // MB per million points
        QString optimizationSettings;
        bool success = false;
        QString errorMessage;
    };

    /**
     * @brief Optimization settings for performance testing
     */
    struct OptimizationSettings {
        int bufferSize = 65536;          // POINTS_PER_READ_BLOCK
        bool useMemoryMapping = false;
        bool enableParallelProcessing = false;
        double subsamplingRatio = 1.0;   // 1.0 = no subsampling
        bool enableProgressReporting = true;
        QString description = "Default";
        
        QString toString() const {
            return QString("Buffer:%1, MemMap:%2, Parallel:%3, Sampling:%4")
                   .arg(bufferSize)
                   .arg(useMemoryMapping ? "Y" : "N")
                   .arg(enableParallelProcessing ? "Y" : "N")
                   .arg(subsamplingRatio, 0, 'f', 2);
        }
    };

    /**
     * @brief Benchmark suite configuration
     */
    struct BenchmarkConfig {
        QStringList testFiles;
        std::vector<OptimizationSettings> optimizationVariants;
        int maxPointsPerTest = 1000000;  // Limit for performance tests
        int timeoutSeconds = 300;        // 5 minute timeout
        bool generateDetailedReport = true;
        QString outputDirectory = "benchmark_results";
    };

public:
    explicit PerformanceProfiler(QObject* parent = nullptr);
    ~PerformanceProfiler();

    // Core profiling methods
    PerformanceMetrics profileE57Loading(const QString& filePath, 
                                       const OptimizationSettings& settings = OptimizationSettings());
    
    std::vector<PerformanceMetrics> runBenchmarkSuite(const BenchmarkConfig& config);
    std::vector<PerformanceMetrics> compareOptimizations(const QString& filePath);
    
    // Analysis and reporting
    void generatePerformanceReport(const std::vector<PerformanceMetrics>& metrics, 
                                 const QString& outputPath = "");
    QJsonObject exportMetricsToJson(const std::vector<PerformanceMetrics>& metrics);
    
    // Configuration
    void setBaselineMemory(size_t baseline) { m_baselineMemory = baseline; }
    void setVerboseLogging(bool verbose) { m_verboseLogging = verbose; }

    // Memory monitoring (public for MemoryMonitor access)
    size_t getCurrentMemoryUsage();
    void startMemoryMonitoring();
    void stopMemoryMonitoring();
    size_t getPeakMemoryUsage();

signals:
    void profilingProgress(const QString& stage, int percentage);
    void benchmarkCompleted(const PerformanceMetrics& metrics);
    void profilingStarted(const QString& fileName);
    void profilingFinished(const PerformanceMetrics& metrics);

private:
    std::unique_ptr<E57ParserLib> m_parser;
    QElapsedTimer m_timer;
    size_t m_baselineMemory;
    bool m_verboseLogging = false;
    
    // Performance measurement helpers
    double measureXMLParsingTime(const QString& filePath);
    double measureBinaryReadTime(const QString& filePath, const OptimizationSettings& settings);
    double measureDataConversionTime(const std::vector<float>& points);
    
    // Analysis helpers
    double calculatePointsPerSecond(int64_t pointCount, double timeSeconds);
    double calculateMemoryEfficiency(size_t memoryBytes, int64_t pointCount);
    QString categorizePerformance(const PerformanceMetrics& metrics);
    
    // Optimization testing
    std::vector<OptimizationSettings> generateOptimizationVariants();
    PerformanceMetrics testOptimizationSettings(const QString& filePath, 
                                              const OptimizationSettings& settings);
    
    // Reporting helpers
    QString generateTextReport(const std::vector<PerformanceMetrics>& metrics);
    QString generateHtmlReport(const std::vector<PerformanceMetrics>& metrics);
    void exportToCsv(const std::vector<PerformanceMetrics>& metrics, const QString& filePath);
    
    // Memory tracking
    std::vector<size_t> m_memorySnapshots;
    size_t m_peakMemory = 0;
    bool m_monitoringActive = false;
};

/**
 * @brief RAII helper for automatic memory monitoring
 */
class MemoryMonitor {
public:
    explicit MemoryMonitor(PerformanceProfiler* profiler);
    ~MemoryMonitor();
    
    size_t getCurrentUsage() const;
    size_t getPeakUsage() const;

private:
    PerformanceProfiler* m_profiler;
    size_t m_startMemory;
    size_t m_peakMemory;
};

#endif // PERFORMANCEPROFILER_H
