#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>

/**
 * @brief OperationType - Types of operations that can be tracked
 */
enum class OperationType {
    FileLoading,
    FileParsing,
    DataProcessing,
    ProjectOperation
};

/**
 * @brief ProgressManager - Manages progress tracking for long-running operations
 * 
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class ProgressManager : public QObject {
    Q_OBJECT

public:
    explicit ProgressManager(QObject *parent = nullptr);
    virtual ~ProgressManager() = default;

    // Basic progress tracking methods
    void startOperation(const QString& operationId, const QString& name, OperationType type);
    void updateProgress(const QString& operationId, int value, int max, const QString& step = QString(), const QString& details = QString());
    void finishOperation(const QString& operationId, const QString& result);
    void cancelOperation(const QString& operationId);

signals:
    void operationStarted(const QString& operationId, const QString& name, OperationType type);
    void progressUpdated(const QString& operationId, int value, int max, const QString& step, const QString& details);
    void operationFinished(const QString& operationId, const QString& result);
    void operationCancelled(const QString& operationId);
    void estimatedTimeChanged(const QString& operationId, const QDateTime& estimatedEnd);

private:
    // Stub implementation - will be expanded in future sprints
};

#endif // PROGRESSMANAGER_H
