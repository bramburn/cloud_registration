#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QDateTime>
#include <QMutex>
#include <QUuid>

enum class OperationType {
    ScanImport,
    ClusterLoad,
    ProjectSave,
    DataExport,
    FileProcessing,
    Unknown
};

struct ProgressInfo {
    QString operationName;
    OperationType type;
    int currentValue;
    int maxValue;
    QString currentStep;
    QString detailedStatus;
    bool isActive;
    bool isCancellable;
    bool isCancelled;
    QDateTime startTime;
    QDateTime estimatedEndTime;
    QString operationId;
    
    ProgressInfo() 
        : type(OperationType::Unknown)
        , currentValue(0)
        , maxValue(100)
        , isActive(false)
        , isCancellable(true)
        , isCancelled(false)
    {}
};

/**
 * @brief Centralized progress management system for Sprint 3.3
 * 
 * Manages non-modal progress indicators for long-running operations:
 * - Scan import operations
 * - Cluster loading for viewing
 * - Project save/load operations
 * - Data export operations
 * 
 * Features:
 * - Time estimation based on progress
 * - Cancellation support
 * - Multiple concurrent operations
 * - Thread-safe operation
 */
class ProgressManager : public QObject
{
    Q_OBJECT
    
public:
    static ProgressManager& instance();
    
    // Operation management
    QString startOperation(OperationType type, const QString& name, int maxSteps = 100, bool cancellable = true);
    void updateProgress(const QString& operationId, int value, const QString& step = "", const QString& details = "");
    void finishOperation(const QString& operationId, const QString& result = "");
    void cancelOperation(const QString& operationId);
    void setOperationCancellable(const QString& operationId, bool cancellable);
    
    // Query methods
    ProgressInfo getProgressInfo(const QString& operationId) const;
    QStringList activeOperations() const;
    bool hasActiveOperations() const;
    bool isOperationCancelled(const QString& operationId) const;
    
    // Time estimation
    QDateTime estimateCompletion(const QString& operationId) const;
    QString formatTimeRemaining(const QString& operationId) const;
    int getProgressPercentage(const QString& operationId) const;
    
    // Utility methods
    void clearFinishedOperations();
    void cancelAllOperations();
    
signals:
    void operationStarted(const QString& operationId, const QString& name, OperationType type);
    void progressUpdated(const QString& operationId, int value, int max, const QString& step, const QString& details);
    void operationFinished(const QString& operationId, const QString& result);
    void operationCancelled(const QString& operationId);
    void estimatedTimeChanged(const QString& operationId, const QDateTime& estimatedEnd);
    void allOperationsFinished();
    
private slots:
    void updateTimeEstimates();
    void cleanupFinishedOperations();
    
private:
    ProgressManager(QObject* parent = nullptr);
    ~ProgressManager() = default;
    ProgressManager(const ProgressManager&) = delete;
    ProgressManager& operator=(const ProgressManager&) = delete;
    
    QString generateOperationId() const;
    void calculateTimeEstimate(ProgressInfo& info);
    QString formatDuration(qint64 milliseconds) const;
    
    // Thread-safe data access
    mutable QMutex m_mutex;
    QHash<QString, ProgressInfo> m_operations;
    
    // Timers
    QTimer* m_estimationTimer;
    QTimer* m_cleanupTimer;
    
    // Constants
    static const int ESTIMATION_UPDATE_INTERVAL = 1000; // 1 second
    static const int CLEANUP_INTERVAL = 30000; // 30 seconds
    static const int MIN_SAMPLES_FOR_ESTIMATION = 3;
};

#endif // PROGRESSMANAGER_H
