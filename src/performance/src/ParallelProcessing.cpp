#include "performance/ParallelProcessing.h"

ParallelProcessing::ParallelProcessing(QObject *parent)
    : QObject(parent)
{
}

void ParallelProcessing::processInParallel(const QString& operationId)
{
    m_isProcessing = true;
    emit parallelProcessingStarted(operationId);
    
    // Stub implementation - actual parallel processing logic will be added in future sprints
    emit parallelProcessingFinished(operationId, true);
    m_isProcessing = false;
}

void ParallelProcessing::cancelParallelOperation(const QString& operationId)
{
    if (m_isProcessing) {
        m_isProcessing = false;
        emit parallelProcessingCancelled(operationId);
    }
}
