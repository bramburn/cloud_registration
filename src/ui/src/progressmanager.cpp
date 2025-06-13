#include "ui/progressmanager.h"

ProgressManager::ProgressManager(QObject* parent) : QObject(parent) {}

void ProgressManager::startOperation(const QString& operationId, const QString& name, OperationType type)
{
    emit operationStarted(operationId, name, type);
}

void ProgressManager::updateProgress(
    const QString& operationId, int value, int max, const QString& step, const QString& details)
{
    emit progressUpdated(operationId, value, max, step, details);
}

void ProgressManager::finishOperation(const QString& operationId, const QString& result)
{
    emit operationFinished(operationId, result);
}

void ProgressManager::cancelOperation(const QString& operationId)
{
    emit operationCancelled(operationId);
}
