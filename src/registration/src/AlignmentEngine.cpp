#include "registration/AlignmentEngine.h"
#include "../algorithms/LeastSquaresAlignment.h"
#include <QDebug>
#include <QElapsedTimer>

AlignmentEngine::AlignmentEngine(QObject* parent)
    : QObject(parent)
    , m_computationTimer(new QTimer(this))
{
    // Initialize current result
    m_currentResult.transformation.setToIdentity();
    m_currentResult.state = AlignmentState::Idle;
    m_currentResult.message = "No correspondences defined";

    // Setup computation timer for async processing
    m_computationTimer->setSingleShot(true);
    m_computationTimer->setInterval(COMPUTATION_DELAY_MS);
    connect(m_computationTimer, &QTimer::timeout, this, &AlignmentEngine::performAlignment);

    qDebug() << "AlignmentEngine initialized";
}

void AlignmentEngine::setCorrespondences(const QList<QPair<QVector3D, QVector3D>>& correspondences)
{
    m_correspondences = correspondences;
    
    qDebug() << "Correspondences set:" << m_correspondences.size() << "pairs";
    
    emit correspondencesChanged(m_correspondences.size());
    triggerRecomputeIfEnabled();
}

void AlignmentEngine::addCorrespondence(const QVector3D& sourcePoint, const QVector3D& targetPoint)
{
    m_correspondences.append(qMakePair(sourcePoint, targetPoint));
    
    qDebug() << "Correspondence added. Total:" << m_correspondences.size();
    
    emit correspondencesChanged(m_correspondences.size());
    triggerRecomputeIfEnabled();
}

void AlignmentEngine::removeCorrespondence(int index)
{
    if (index >= 0 && index < m_correspondences.size()) {
        m_correspondences.removeAt(index);
        
        qDebug() << "Correspondence removed at index" << index << ". Remaining:" << m_correspondences.size();
        
        emit correspondencesChanged(m_correspondences.size());
        triggerRecomputeIfEnabled();
    } else {
        qWarning() << "Invalid correspondence index for removal:" << index;
    }
}

void AlignmentEngine::clearCorrespondences()
{
    m_correspondences.clear();
    
    // Reset to idle state
    m_currentResult.transformation.setToIdentity();
    m_currentResult.state = AlignmentState::Idle;
    m_currentResult.message = "No correspondences defined";
    m_currentResult.errorStats = ErrorAnalysis::ErrorStatistics();
    
    qDebug() << "All correspondences cleared";
    
    emit correspondencesChanged(0);
    emit alignmentResultUpdated(m_currentResult);
    emit alignmentStateChanged(AlignmentState::Idle, "No correspondences defined");
    emit transformationUpdated(m_currentResult.transformation);
    emit qualityMetricsUpdated(0.0f);
}

void AlignmentEngine::recomputeAlignment()
{
    if (m_computationPending) {
        // Restart timer to debounce rapid requests
        m_computationTimer->start();
        return;
    }
    
    m_computationPending = true;
    m_computationTimer->start();
    
    qDebug() << "Alignment recomputation requested";
}

void AlignmentEngine::performAlignment()
{
    m_computationPending = false;
    
    QElapsedTimer timer;
    timer.start();
    
    qDebug() << "Starting alignment computation with" << m_correspondences.size() << "correspondences";
    
    // Validate correspondences
    if (!validateCorrespondences()) {
        return; // Error state already set by validateCorrespondences()
    }
    
    updateAlignmentState(AlignmentState::Computing, "Computing transformation...");
    
    try {
        // Compute transformation using least-squares alignment
        QMatrix4x4 transformation = LeastSquaresAlignment::computeTransformation(m_correspondences);
        
        // Validate computed transformation
        if (!ErrorAnalysis::validateTransformation(transformation)) {
            updateAlignmentState(AlignmentState::Error, "Invalid transformation computed");
            return;
        }
        
        // Calculate comprehensive error statistics
        ErrorAnalysis::ErrorStatistics errorStats = 
            ErrorAnalysis::calculateErrorStatistics(m_correspondences, transformation);
        
        // Update result
        m_currentResult.transformation = transformation;
        m_currentResult.errorStats = errorStats;
        m_currentResult.computationTimeMs = timer.elapsed();
        
        // Check quality thresholds
        if (errorStats.meetsQualityThresholds(m_rmsThreshold, m_maxErrorThreshold)) {
            m_currentResult.state = AlignmentState::Valid;
            m_currentResult.message = QString("Alignment computed successfully (RMS: %1 mm)")
                                     .arg(errorStats.rmsError, 0, 'f', 3);
        } else {
            m_currentResult.state = AlignmentState::Valid; // Still valid, just poor quality
            m_currentResult.message = QString("Alignment computed with poor quality (RMS: %1 mm)")
                                     .arg(errorStats.rmsError, 0, 'f', 3);
        }
        
        qDebug() << "Alignment computation completed in" << m_currentResult.computationTimeMs << "ms";
        qDebug() << "RMS error:" << errorStats.rmsError << "mm";
        
        // Emit signals for real-time update
        emit transformationUpdated(transformation);
        emit qualityMetricsUpdated(errorStats.rmsError);
        emit alignmentResultUpdated(m_currentResult);
        emit alignmentStateChanged(m_currentResult.state, m_currentResult.message);
        
    } catch (const std::exception& e) {
        QString errorMsg = QString("Alignment computation failed: %1").arg(e.what());
        updateAlignmentState(AlignmentState::Error, errorMsg);
        qCritical() << errorMsg;
    } catch (...) {
        QString errorMsg = "Unknown error during alignment computation";
        updateAlignmentState(AlignmentState::Error, errorMsg);
        qCritical() << errorMsg;
    }
}

bool AlignmentEngine::validateCorrespondences() const
{
    if (m_correspondences.size() < MIN_CORRESPONDENCES) {
        QString message = QString("Insufficient correspondences: %1 < %2")
                         .arg(m_correspondences.size()).arg(MIN_CORRESPONDENCES);
        
        const_cast<AlignmentEngine*>(this)->updateAlignmentState(AlignmentState::Insufficient, message);
        return false;
    }
    
    // Check for duplicate or very close points
    for (int i = 0; i < m_correspondences.size(); ++i) {
        for (int j = i + 1; j < m_correspondences.size(); ++j) {
            float srcDist = m_correspondences[i].first.distanceToPoint(m_correspondences[j].first);
            float tgtDist = m_correspondences[i].second.distanceToPoint(m_correspondences[j].second);
            
            if (srcDist < 0.001f || tgtDist < 0.001f) { // 1mm minimum separation
                QString message = QString("Duplicate correspondences detected at indices %1 and %2")
                                 .arg(i).arg(j);
                const_cast<AlignmentEngine*>(this)->updateAlignmentState(AlignmentState::Error, message);
                return false;
            }
        }
    }
    
    return true;
}

void AlignmentEngine::updateAlignmentState(AlignmentState state, const QString& message)
{
    m_currentResult.state = state;
    m_currentResult.message = message;
    
    // Reset transformation and metrics for error states
    if (state == AlignmentState::Error || state == AlignmentState::Insufficient) {
        m_currentResult.transformation.setToIdentity();
        m_currentResult.errorStats = ErrorAnalysis::ErrorStatistics();
        
        emit transformationUpdated(m_currentResult.transformation);
        emit qualityMetricsUpdated(0.0f);
    }
    
    emit alignmentResultUpdated(m_currentResult);
    emit alignmentStateChanged(state, message);
    
    qDebug() << "Alignment state updated:" << static_cast<int>(state) << "-" << message;
}

void AlignmentEngine::triggerRecomputeIfEnabled()
{
    if (m_autoRecompute) {
        recomputeAlignment();
    }
}

void AlignmentEngine::setQualityThresholds(float rmsThreshold, float maxErrorThreshold)
{
    m_rmsThreshold = rmsThreshold;
    m_maxErrorThreshold = maxErrorThreshold;
    
    qDebug() << "Quality thresholds updated - RMS:" << rmsThreshold << "mm, Max:" << maxErrorThreshold << "mm";
    
    // Revalidate current result if available
    if (m_currentResult.state == AlignmentState::Valid) {
        triggerRecomputeIfEnabled();
    }
}
