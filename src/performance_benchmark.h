#ifndef PERFORMANCE_BENCHMARK_H
#define PERFORMANCE_BENCHMARK_H

#include <QObject>
#include <QElapsedTimer>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDebug>
#include <vector>

/**
 * @brief Structure to hold benchmark results for a single test
 * 
 * This structure captures comprehensive performance metrics for
 * point cloud file loading operations.
 */
struct BenchmarkResult {
    QString testName;           ///< Descriptive name for the test
    QString filePath;           ///< Path to the test file
    QString fileType;           ///< File type (E57, LAS, etc.)
    qint64 loadTimeMs;          ///< Total loading time in milliseconds
    qint64 fileSize;            ///< File size in bytes
    int pointCount;             ///< Number of points loaded
    double pointsPerSecond;     ///< Loading rate (points per second)
    qint64 memoryUsage;         ///< Peak memory usage during loading (bytes)
    bool success;               ///< Whether the loading was successful
    QString errorMessage;       ///< Error message if loading failed
    
    // Detailed timing breakdown
    qint64 fileOpenTime = 0;    ///< Time to open file
    qint64 headerParseTime = 0; ///< Time to parse header
    qint64 dataParseTime = 0;   ///< Time to parse point data
    qint64 gpuUploadTime = 0;   ///< Time to upload to GPU
    
    /**
     * @brief Calculate loading efficiency in MB/s
     * @return Loading speed in megabytes per second
     */
    double getMBPerSecond() const {
        return loadTimeMs > 0 ? (double)fileSize / (1024.0 * 1024.0) / (loadTimeMs / 1000.0) : 0.0;
    }
    
    /**
     * @brief Get human-readable file size
     * @return Formatted file size string
     */
    QString getFormattedFileSize() const;
    
    /**
     * @brief Get human-readable point count
     * @return Formatted point count string
     */
    QString getFormattedPointCount() const;
};

/**
 * @brief Comprehensive performance benchmarking system
 * 
 * This class provides automated benchmarking capabilities for point cloud
 * file loading operations. It can run individual benchmarks or comprehensive
 * test suites, generating detailed performance reports.
 * 
 * Features:
 * - Automated timing of file loading operations
 * - Memory usage monitoring
 * - Detailed performance breakdowns
 * - Comparison reports between different files/formats
 * - JSON and text report generation
 * 
 * Usage:
 * @code
 * PerformanceBenchmark benchmark;
 * benchmark.runE57Benchmark("large_file.e57");
 * benchmark.runLASBenchmark("large_file.las");
 * benchmark.generateBenchmarkReport("benchmark_results");
 * @endcode
 */
class PerformanceBenchmark : public QObject
{
    Q_OBJECT
    
public:
    /**
     * @brief Construct a performance benchmark instance
     * @param parent Parent QObject
     */
    explicit PerformanceBenchmark(QObject *parent = nullptr);
    
    /**
     * @brief Run benchmark on an E57 file
     * @param filePath Path to the E57 file to benchmark
     * @return BenchmarkResult containing performance metrics
     */
    BenchmarkResult runE57Benchmark(const QString &filePath);
    
    /**
     * @brief Run benchmark on a LAS file
     * @param filePath Path to the LAS file to benchmark
     * @return BenchmarkResult containing performance metrics
     */
    BenchmarkResult runLASBenchmark(const QString &filePath);
    
    /**
     * @brief Run comprehensive benchmark suite on multiple files
     * @param files List of file paths to benchmark
     */
    void runComparisonSuite(const QStringList &files);
    
    /**
     * @brief Generate comprehensive benchmark report
     * @param outputPath Base path for output files (without extension)
     */
    void generateBenchmarkReport(const QString &outputPath);
    
    /**
     * @brief Clear all benchmark results
     */
    void clearResults();
    
    /**
     * @brief Get all benchmark results
     * @return Vector of all benchmark results
     */
    const std::vector<BenchmarkResult>& getResults() const { return m_results; }
    
    /**
     * @brief Set memory monitoring enabled/disabled
     * @param enabled True to enable memory monitoring
     */
    void setMemoryMonitoringEnabled(bool enabled) { m_memoryMonitoringEnabled = enabled; }
    
    /**
     * @brief Check if memory monitoring is enabled
     * @return True if memory monitoring is enabled
     */
    bool isMemoryMonitoringEnabled() const { return m_memoryMonitoringEnabled; }

public slots:
    /**
     * @brief Slot called when parsing is finished
     * @param success Whether parsing was successful
     * @param message Status/error message
     * @param points Vector of loaded points
     */
    void onParsingFinished(bool success, const QString &message, 
                          const std::vector<float> &points);

private:
    /**
     * @brief Run a single benchmark test
     * @param filePath Path to the file to benchmark
     * @param parserType Type of parser to use ("E57" or "LAS")
     * @return BenchmarkResult containing performance metrics
     */
    BenchmarkResult runSingleBenchmark(const QString &filePath, 
                                     const QString &parserType);
    
    /**
     * @brief Get current memory usage of the process
     * @return Memory usage in bytes, or 0 if unable to determine
     */
    qint64 getMemoryUsage() const;
    
    /**
     * @brief Get baseline memory usage before loading
     * @return Baseline memory usage in bytes
     */
    qint64 getBaselineMemoryUsage() const;
    
    /**
     * @brief Generate JSON report from benchmark results
     * @return QJsonObject containing all benchmark data
     */
    QJsonObject generateJsonReport() const;
    
    /**
     * @brief Generate human-readable text report
     * @return QString containing formatted report
     */
    QString generateTextReport() const;
    
    /**
     * @brief Generate comparison table for multiple results
     * @return QString containing comparison table
     */
    QString generateComparisonTable() const;
    
    /**
     * @brief Wait for parsing to complete with timeout
     * @param timeoutMs Timeout in milliseconds
     * @return True if parsing completed within timeout
     */
    bool waitForParsingComplete(int timeoutMs = 30000);
    
    std::vector<BenchmarkResult> m_results;     ///< All benchmark results
    QElapsedTimer m_benchmarkTimer;             ///< Timer for current benchmark
    bool m_benchmarkInProgress = false;         ///< Whether a benchmark is currently running
    BenchmarkResult m_currentResult;            ///< Current benchmark being executed
    bool m_memoryMonitoringEnabled = true;      ///< Whether to monitor memory usage
    qint64 m_baselineMemory = 0;               ///< Baseline memory usage
    
    // Detailed timing
    QElapsedTimer m_detailedTimer;              ///< Timer for detailed timing measurements
    qint64 m_lastTimestamp = 0;                 ///< Last timestamp for timing calculations
};

#endif // PERFORMANCE_BENCHMARK_H
