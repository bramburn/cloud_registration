#include "progressmanager.h"
#include <QApplication>
#include <QDebug>

const int ProgressManager::ESTIMATION_UPDATE_INTERVAL;
const int ProgressManager::CLEANUP_INTERVAL;
const int ProgressManager::MIN_SAMPLES_FOR_ESTIMATION;

ProgressManager& ProgressManager::instance()
{
    static ProgressManager instance;
    return instance;
}

ProgressManager::ProgressManager(QObject* parent)
    : QObject(parent)
{
    // Setup timers
    m_estimationTimer = new QTimer(this);
    m_estimationTimer->setInterval(ESTIMATION_UPDATE_INTERVAL);
    connect(m_estimationTimer, &QTimer::timeout, this, &ProgressManager::updateTimeEstimates);
    
    m_cleanupTimer = new QTimer(this);
    m_cleanupTimer->setInterval(CLEANUP_INTERVAL);
    connect(m_cleanupTimer, &QTimer::timeout, this, &ProgressManager::cleanupFinishedOperations);
    m_cleanupTimer->start();
}

QString ProgressManager::startOperation(OperationType type, const QString& name, int maxSteps, bool cancellable)
{
    QMutexLocker locker(&m_mutex);
    
    QString operationId = generateOperationId();
    
    ProgressInfo info;
    info.operationName = name;
    info.type = type;
    info.currentValue = 0;
    info.maxValue = maxSteps;
    info.isActive = true;
    info.isCancellable = cancellable;
    info.isCancelled = false;
    info.startTime = QDateTime::currentDateTime();
    info.operationId = operationId;
    
    m_operations[operationId] = info;
    
    // Start estimation timer if this is the first operation
    if (m_operations.size() == 1) {
        m_estimationTimer->start();
    }
    
    emit operationStarted(operationId, name, type);
    return operationId;
}

void ProgressManager::updateProgress(const QString& operationId, int value, const QString& step, const QString& details)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_operations.contains(operationId)) {
        qWarning() << "ProgressManager: Unknown operation ID:" << operationId;
        return;
    }
    
    auto& info = m_operations[operationId];
    
    // Check if operation was cancelled
    if (info.isCancelled) {
        return;
    }
    
    info.currentValue = qBound(0, value, info.maxValue);
    info.currentStep = step;
    info.detailedStatus = details;
    
    // Update time estimation
    calculateTimeEstimate(info);
    
    emit progressUpdated(operationId, info.currentValue, info.maxValue, step, details);
}

void ProgressManager::finishOperation(const QString& operationId, const QString& result)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_operations.contains(operationId)) {
        return;
    }
    
    auto& info = m_operations[operationId];
    info.isActive = false;
    info.currentValue = info.maxValue; // Ensure 100% completion
    
    emit operationFinished(operationId, result);
    
    // Remove the operation after a short delay to allow UI updates
    QTimer::singleShot(1000, [this, operationId]() {
        QMutexLocker locker(&m_mutex);
        m_operations.remove(operationId);
        
        // Stop estimation timer if no active operations
        if (activeOperations().isEmpty()) {
            m_estimationTimer->stop();
            emit allOperationsFinished();
        }
    });
}

void ProgressManager::cancelOperation(const QString& operationId)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_operations.contains(operationId)) {
        return;
    }
    
    auto& info = m_operations[operationId];
    if (info.isCancellable && info.isActive) {
        info.isCancelled = true;
        info.isActive = false;
        emit operationCancelled(operationId);
        
        // Remove cancelled operation after delay
        QTimer::singleShot(1000, [this, operationId]() {
            QMutexLocker locker(&m_mutex);
            m_operations.remove(operationId);
        });
    }
}

ProgressInfo ProgressManager::getProgressInfo(const QString& operationId) const
{
    QMutexLocker locker(&m_mutex);
    return m_operations.value(operationId);
}

QStringList ProgressManager::activeOperations() const
{
    QMutexLocker locker(&m_mutex);
    QStringList active;
    
    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if (it->isActive) {
            active.append(it.key());
        }
    }
    
    return active;
}

bool ProgressManager::hasActiveOperations() const
{
    return !activeOperations().isEmpty();
}

bool ProgressManager::isOperationCancelled(const QString& operationId) const
{
    QMutexLocker locker(&m_mutex);
    if (m_operations.contains(operationId)) {
        return m_operations[operationId].isCancelled;
    }
    return false;
}

QString ProgressManager::generateOperationId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ProgressManager::calculateTimeEstimate(ProgressInfo& info)
{
    if (info.currentValue <= 0 || info.maxValue <= 0) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 elapsed = info.startTime.msecsTo(now);

    if (elapsed < 1000) { // Need at least 1 second of data
        return;
    }

    double progress = double(info.currentValue) / info.maxValue;
    if (progress > 0.01) { // Need at least 1% progress
        qint64 estimatedTotal = elapsed / progress;
        info.estimatedEndTime = info.startTime.addMSecs(estimatedTotal);
    }
}

QString ProgressManager::formatTimeRemaining(const QString& operationId) const
{
    QMutexLocker locker(&m_mutex);

    if (!m_operations.contains(operationId)) {
        return QString();
    }

    const auto& info = m_operations[operationId];
    if (!info.estimatedEndTime.isValid()) {
        return "Calculating...";
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 remaining = now.msecsTo(info.estimatedEndTime);

    if (remaining <= 0) {
        return "Almost done...";
    }

    return formatDuration(remaining);
}

QString ProgressManager::formatDuration(qint64 milliseconds) const
{
    qint64 seconds = milliseconds / 1000;

    if (seconds < 60) {
        return QString("%1s").arg(seconds);
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        int remainingSeconds = seconds % 60;
        return QString("%1m %2s").arg(minutes).arg(remainingSeconds);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return QString("%1h %2m").arg(hours).arg(minutes);
    }
}

int ProgressManager::getProgressPercentage(const QString& operationId) const
{
    QMutexLocker locker(&m_mutex);

    if (!m_operations.contains(operationId)) {
        return 0;
    }

    const auto& info = m_operations[operationId];
    if (info.maxValue <= 0) {
        return 0;
    }

    return (info.currentValue * 100) / info.maxValue;
}

void ProgressManager::updateTimeEstimates()
{
    QMutexLocker locker(&m_mutex);

    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if (it->isActive && it->currentValue > 0) {
            emit estimatedTimeChanged(it.key(), it->estimatedEndTime);
        }
    }
}

void ProgressManager::cleanupFinishedOperations()
{
    QMutexLocker locker(&m_mutex);

    QStringList toRemove;
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-60); // Remove operations finished more than 1 minute ago

    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if (!it->isActive && it->startTime < cutoff) {
            toRemove.append(it.key());
        }
    }

    for (const QString& id : toRemove) {
        m_operations.remove(id);
    }
}

void ProgressManager::setOperationCancellable(const QString& operationId, bool cancellable)
{
    QMutexLocker locker(&m_mutex);

    if (m_operations.contains(operationId)) {
        m_operations[operationId].isCancellable = cancellable;
    }
}

void ProgressManager::clearFinishedOperations()
{
    QMutexLocker locker(&m_mutex);

    QStringList toRemove;
    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if (!it->isActive) {
            toRemove.append(it.key());
        }
    }

    for (const QString& id : toRemove) {
        m_operations.remove(id);
    }
}

void ProgressManager::cancelAllOperations()
{
    QMutexLocker locker(&m_mutex);

    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if (it->isActive && it->isCancellable) {
            it->isCancelled = true;
            it->isActive = false;
            emit operationCancelled(it.key());
        }
    }
}

QDateTime ProgressManager::estimateCompletion(const QString& operationId) const
{
    QMutexLocker locker(&m_mutex);

    if (m_operations.contains(operationId)) {
        return m_operations[operationId].estimatedEndTime;
    }

    return QDateTime();
}
