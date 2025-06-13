#ifndef PERFORMANCE_PROFILER_H
#define PERFORMANCE_PROFILER_H

#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTextStream>

#include <climits>

/**
 * @brief Structure to hold profiling data for a specific section
 *
 * This structure tracks timing statistics for individual profiling sections,
 * allowing for detailed performance analysis.
 */
struct ProfileSection
{
    QString name;                ///< Name of the profiled section
    qint64 elapsedMs = 0;        ///< Last elapsed time in milliseconds
    qint64 startTime = 0;        ///< Start time for current measurement
    int callCount = 0;           ///< Number of times this section was called
    qint64 totalTime = 0;        ///< Total accumulated time across all calls
    qint64 minTime = LLONG_MAX;  ///< Minimum time recorded for this section
    qint64 maxTime = 0;          ///< Maximum time recorded for this section

    /**
     * @brief Calculate average time per call
     * @return Average time in milliseconds, or 0 if no calls recorded
     */
    double averageTime() const
    {
        return callCount > 0 ? (double)totalTime / callCount : 0.0;
    }
};

/**
 * @brief High-performance profiler for measuring execution times
 *
 * This class provides a singleton profiler that can measure execution times
 * for different sections of code. It's designed to be lightweight and suitable
 * for production use with minimal overhead.
 *
 * Usage:
 * @code
 * // Method 1: Manual start/end
 * PerformanceProfiler::instance().startSection("MyOperation");
 * // ... code to profile ...
 * PerformanceProfiler::instance().endSection("MyOperation");
 *
 * // Method 2: RAII with SectionTimer
 * {
 *     PerformanceProfiler::SectionTimer timer("MyOperation");
 *     // ... code to profile ...
 * } // Timer automatically ends when going out of scope
 *
 * // Method 3: Convenience macros
 * PROFILE_SECTION("MyOperation");
 * PROFILE_FUNCTION(); // Uses current function name
 * @endcode
 */
class PerformanceProfiler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance of the profiler
     * @return Reference to the global profiler instance
     */
    static PerformanceProfiler& instance();

    /**
     * @brief Start timing a named section
     * @param sectionName Unique name for the section being profiled
     */
    void startSection(const QString& sectionName);

    /**
     * @brief End timing a named section
     * @param sectionName Name of the section to stop timing
     */
    void endSection(const QString& sectionName);

    /**
     * @brief Generate a comprehensive profiling report
     * @param filePath Optional file path to save the report. If empty, outputs to debug console
     */
    void generateReport(const QString& filePath = "");

    /**
     * @brief Reset all profiling data
     */
    void reset();

    /**
     * @brief Get profiling data for a specific section
     * @param sectionName Name of the section
     * @return ProfileSection data, or default-constructed section if not found
     */
    ProfileSection getSection(const QString& sectionName) const;

    /**
     * @brief Check if profiling is currently enabled
     * @return True if profiling is enabled
     */
    bool isEnabled() const
    {
        return m_enabled;
    }

    /**
     * @brief Enable or disable profiling
     * @param enabled True to enable profiling, false to disable
     */
    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    /**
     * @brief RAII timer class for automatic section timing
     *
     * This class automatically starts timing when constructed and stops
     * timing when destroyed, making it perfect for scope-based profiling.
     */
    class SectionTimer
    {
    public:
        /**
         * @brief Construct a section timer and start timing
         * @param name Name of the section to profile
         */
        explicit SectionTimer(const QString& name) : m_name(name)
        {
            PerformanceProfiler::instance().startSection(m_name);
        }

        /**
         * @brief Destructor automatically ends timing
         */
        ~SectionTimer()
        {
            PerformanceProfiler::instance().endSection(m_name);
        }

        // Disable copy and move to prevent timing issues
        SectionTimer(const SectionTimer&) = delete;
        SectionTimer& operator=(const SectionTimer&) = delete;
        SectionTimer(SectionTimer&&) = delete;
        SectionTimer& operator=(SectionTimer&&) = delete;

    private:
        QString m_name;
    };

private:
    /**
     * @brief Private constructor for singleton pattern
     * @param parent Parent QObject (typically nullptr for singleton)
     */
    explicit PerformanceProfiler(QObject* parent = nullptr);

    /**
     * @brief Generate JSON report data
     * @return QJsonObject containing all profiling data
     */
    QJsonObject generateJsonReport() const;

    /**
     * @brief Generate human-readable text report
     * @return QString containing formatted report
     */
    QString generateTextReport() const;

    QMap<QString, ProfileSection> m_sections;     ///< Map of section names to profiling data
    QMap<QString, QElapsedTimer> m_activeTimers;  ///< Active timers for ongoing measurements
    QElapsedTimer m_globalTimer;                  ///< Global timer for relative timestamps
    bool m_enabled = true;                        ///< Whether profiling is currently enabled
};

// Note: Profiling macros are now defined in profiling_macros.h for better organization

#endif  // PERFORMANCE_PROFILER_H
