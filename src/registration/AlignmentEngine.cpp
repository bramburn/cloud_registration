#include "AlignmentEngine.h"
#include "../algorithms/LeastSquaresAlignment.h"
#include "../ui/ICPProgressWidget.h"
#include <QDebug>
#include <QApplication>

AlignmentEngine::AlignmentEngine(QObject* parent)
    : QObject(parent)
    , m_currentRMSError(0.0f)
    , m_manualRMSError(0.0f)
    , m_icpAlgorithm(nullptr)
    , m_currentAlgorithmType(ICPAlgorithmType::PointToPoint)
    , m_progressWidget(nullptr)
    , m_hasValidAlignment(false)
    , m_icpInProgress(false) {
    
    m_currentTransform.setToIdentity();
    
    // Create progress widget
    m_progressWidget = new ICPProgressWidget();
    connect(m_progressWidget, &ICPProgressWidget::computationCompleted,
            this, &AlignmentEngine::onProgressWidgetClosed);
}

AlignmentEngine::~AlignmentEngine() {
    if (m_icpAlgorithm) {
        m_icpAlgorithm->cancel();
    }
    
    if (m_progressWidget) {
        m_progressWidget->deleteLater();
    }
}

void AlignmentEngine::setCorrespondences(const QList<QPair<QVector3D, QVector3D>>& correspondences) {
    m_correspondences = correspondences;
    
    if (m_correspondences.size() >= 3) {
        recomputeAlignment();
    } else {
        m_hasValidAlignment = false;
        m_currentTransform.setToIdentity();
        m_currentRMSError = 0.0f;
        
        emit transformationUpdated(m_currentTransform);
        emit qualityMetricsUpdated(m_currentRMSError, m_correspondences.size());
    }
    
    qDebug() << "AlignmentEngine: Set" << correspondences.size() << "correspondences";
}

void AlignmentEngine::recomputeAlignment() {
    if (m_correspondences.size() < 3) {
        qWarning() << "AlignmentEngine: Need at least 3 correspondences for alignment, got" 
                   << m_correspondences.size();
        return;
    }
    
    // Compute transformation using least squares alignment
    m_currentTransform = LeastSquaresAlignment::computeTransformation(m_correspondences);
    
    // Calculate RMS error
    calculateManualAlignmentError();
    
    m_hasValidAlignment = true;
    
    // Emit updates
    emit transformationUpdated(m_currentTransform);
    emit qualityMetricsUpdated(m_currentRMSError, m_correspondences.size());
    
    qDebug() << "AlignmentEngine: Recomputed alignment with RMS error:" << m_currentRMSError;
}

void AlignmentEngine::runICP(const std::vector<float>& sourcePoints,
                            const std::vector<float>& targetPoints,
                            ICPAlgorithmType algorithmType,
                            const ICPParams& params,
                            bool showProgress) {
    
    if (m_icpInProgress) {
        qWarning() << "AlignmentEngine: ICP already in progress";
        return;
    }
    
    if (sourcePoints.empty() || targetPoints.empty()) {
        emit errorOccurred("Cannot run ICP on empty point clouds");
        return;
    }
    
    if (sourcePoints.size() % 3 != 0 || targetPoints.size() % 3 != 0) {
        emit errorOccurred("Point cloud data must be in x,y,z format");
        return;
    }
    
    qDebug() << "AlignmentEngine: Starting ICP with" << (sourcePoints.size() / 3) 
             << "source points and" << (targetPoints.size() / 3) << "target points";
    
    // Create ICP algorithm
    m_icpAlgorithm = createICPAlgorithm(algorithmType);
    if (!m_icpAlgorithm) {
        emit errorOccurred("Failed to create ICP algorithm");
        return;
    }
    
    m_currentAlgorithmType = algorithmType;
    m_currentICPParams = params;
    m_icpInProgress = true;
    
    // Connect ICP signals
    connect(m_icpAlgorithm.get(), &ICPRegistration::progressUpdated,
            this, &AlignmentEngine::onICPProgressUpdated);
    connect(m_icpAlgorithm.get(), &ICPRegistration::computationFinished,
            this, &AlignmentEngine::onICPFinished);
    
    // Show progress widget if requested
    if (showProgress && m_progressWidget) {
        m_progressWidget->startMonitoring(m_icpAlgorithm.get(), params.maxIterations);
    }
    
    // Create point clouds
    PointCloud source(sourcePoints);
    PointCloud target(targetPoints);
    
    // Use current transformation as initial guess, or identity if no manual alignment
    QMatrix4x4 initialGuess = m_hasValidAlignment ? m_currentTransform : QMatrix4x4();
    
    // Store manual RMS for improvement calculation
    m_manualRMSError = m_currentRMSError;
    
    // Emit start signal
    emit icpStarted(algorithmType, params.maxIterations);
    
    // Start ICP computation (this will run and emit signals)
    qDebug() << "AlignmentEngine: Starting" << algorithmTypeToString(algorithmType) 
             << "ICP with initial guess";
    
    // Note: The compute method will run and emit progress signals
    // The actual computation happens asynchronously through the signal/slot mechanism
    m_icpAlgorithm->compute(source, target, initialGuess, params);
}

void AlignmentEngine::cancelICP() {
    if (m_icpAlgorithm && m_icpInProgress) {
        qDebug() << "AlignmentEngine: Cancelling ICP computation";
        m_icpAlgorithm->cancel();
    }
}

bool AlignmentEngine::isICPRunning() const {
    return m_icpInProgress && m_icpAlgorithm && m_icpAlgorithm->isRunning();
}

void AlignmentEngine::onICPProgressUpdated(int iteration, float rmsError, const QMatrix4x4& transformation) {
    // Update current state with ICP progress
    m_currentTransform = transformation;
    m_currentRMSError = rmsError;
    
    // Emit updates
    emit transformationUpdated(transformation);
    emit qualityMetricsUpdated(rmsError, m_correspondences.size());
    
    qDebug() << "AlignmentEngine: ICP iteration" << iteration << "RMS error:" << rmsError;
}

void AlignmentEngine::onICPFinished(bool success, const QMatrix4x4& finalTransformation,
                                   float finalRMSError, int iterations) {
    
    m_icpInProgress = false;
    
    if (success) {
        // Update final transformation
        m_currentTransform = finalTransformation;
        m_currentRMSError = finalRMSError;
        m_hasValidAlignment = true;
        
        // Calculate improvement
        float improvementPercent = 0.0f;
        if (m_manualRMSError > 0.0f) {
            improvementPercent = ((m_manualRMSError - finalRMSError) / m_manualRMSError) * 100.0f;
        }
        
        // Emit final updates
        emit transformationUpdated(finalTransformation);
        emit qualityMetricsUpdated(finalRMSError, m_correspondences.size());
        emit icpFinished(true, finalTransformation, finalRMSError, iterations, improvementPercent);
        
        qDebug() << "AlignmentEngine: ICP completed successfully. Final RMS:" << finalRMSError
                 << "Improvement:" << improvementPercent << "%";
    } else {
        emit icpFinished(false, finalTransformation, finalRMSError, iterations, 0.0f);
        qDebug() << "AlignmentEngine: ICP failed or was cancelled";
    }
    
    // Clean up
    if (m_icpAlgorithm) {
        disconnect(m_icpAlgorithm.get(), nullptr, this, nullptr);
        m_icpAlgorithm.reset();
    }
}

void AlignmentEngine::onProgressWidgetClosed(bool success, const QString& message) {
    Q_UNUSED(success)
    Q_UNUSED(message)
    
    if (m_progressWidget) {
        m_progressWidget->stopMonitoring();
    }
}

void AlignmentEngine::calculateManualAlignmentError() {
    if (m_correspondences.empty()) {
        m_currentRMSError = 0.0f;
        return;
    }
    
    float sumSquaredErrors = 0.0f;
    int validCount = 0;
    
    for (const auto& pair : m_correspondences) {
        QVector3D transformedSource = m_currentTransform.map(pair.first);
        QVector3D error = transformedSource - pair.second;
        sumSquaredErrors += error.lengthSquared();
        validCount++;
    }
    
    if (validCount > 0) {
        m_currentRMSError = std::sqrt(sumSquaredErrors / validCount);
    } else {
        m_currentRMSError = 0.0f;
    }
}

std::unique_ptr<ICPRegistration> AlignmentEngine::createICPAlgorithm(ICPAlgorithmType type) {
    switch (type) {
        case ICPAlgorithmType::PointToPoint:
            return std::make_unique<ICPRegistration>();
            
        case ICPAlgorithmType::PointToPlane:
            return std::make_unique<PointToPlaneICP>();
            
        default:
            qWarning() << "AlignmentEngine: Unknown ICP algorithm type";
            return nullptr;
    }
}

QString AlignmentEngine::algorithmTypeToString(ICPAlgorithmType type) const {
    switch (type) {
        case ICPAlgorithmType::PointToPoint:
            return "Point-to-Point";
        case ICPAlgorithmType::PointToPlane:
            return "Point-to-Plane";
        default:
            return "Unknown";
    }
}
