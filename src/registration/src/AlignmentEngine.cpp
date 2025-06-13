#include "registration/AlignmentEngine.h"

#include <QDebug>
#include <QElapsedTimer>

#include "../algorithms/LeastSquaresAlignment.h"
#include "../algorithms/ICPRegistration.h"
#include "analysis/DifferenceAnalysis.h"
#include "core/pointdata.h"
#include "registration/SphereDetector.h"
#include "registration/TargetManager.h"
#include "app/pointcloudloadmanager.h"

AlignmentEngine::AlignmentEngine(QObject* parent)
    : QObject(parent),
      m_computationTimer(new QTimer(this)),
      m_loadManager(nullptr),
      m_targetManager(nullptr)
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
    if (index >= 0 && index < m_correspondences.size())
    {
        m_correspondences.removeAt(index);

        qDebug() << "Correspondence removed at index" << index << ". Remaining:" << m_correspondences.size();

        emit correspondencesChanged(m_correspondences.size());
        triggerRecomputeIfEnabled();
    }
    else
    {
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
    if (m_computationPending)
    {
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
    if (!validateCorrespondences())
    {
        return;  // Error state already set by validateCorrespondences()
    }

    updateAlignmentState(AlignmentState::Computing, "Computing transformation...");

    try
    {
        // Compute transformation using least-squares alignment
        QMatrix4x4 transformation = LeastSquaresAlignment::computeTransformation(m_correspondences);

        // Validate computed transformation
        if (!ErrorAnalysis::validateTransformation(transformation))
        {
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
        if (errorStats.meetsQualityThresholds(m_rmsThreshold, m_maxErrorThreshold))
        {
            m_currentResult.state = AlignmentState::Valid;
            m_currentResult.message =
                QString("Alignment computed successfully (RMS: %1 mm)").arg(errorStats.rmsError, 0, 'f', 3);
        }
        else
        {
            m_currentResult.state = AlignmentState::Valid;  // Still valid, just poor quality
            m_currentResult.message =
                QString("Alignment computed with poor quality (RMS: %1 mm)").arg(errorStats.rmsError, 0, 'f', 3);
        }

        qDebug() << "Alignment computation completed in" << m_currentResult.computationTimeMs << "ms";
        qDebug() << "RMS error:" << errorStats.rmsError << "mm";

        // Emit signals for real-time update
        emit transformationUpdated(transformation);
        emit qualityMetricsUpdated(errorStats.rmsError);
        emit alignmentResultUpdated(m_currentResult);
        emit alignmentStateChanged(m_currentResult.state, m_currentResult.message);
    }
    catch (const std::exception& e)
    {
        QString errorMsg = QString("Alignment computation failed: %1").arg(e.what());
        updateAlignmentState(AlignmentState::Error, errorMsg);
        qCritical() << errorMsg;
    }
    catch (...)
    {
        QString errorMsg = "Unknown error during alignment computation";
        updateAlignmentState(AlignmentState::Error, errorMsg);
        qCritical() << errorMsg;
    }
}

bool AlignmentEngine::validateCorrespondences() const
{
    if (m_correspondences.size() < MIN_CORRESPONDENCES)
    {
        QString message =
            QString("Insufficient correspondences: %1 < %2").arg(m_correspondences.size()).arg(MIN_CORRESPONDENCES);

        const_cast<AlignmentEngine*>(this)->updateAlignmentState(AlignmentState::Insufficient, message);
        return false;
    }

    // Check for duplicate or very close points
    for (int i = 0; i < m_correspondences.size(); ++i)
    {
        for (int j = i + 1; j < m_correspondences.size(); ++j)
        {
            float srcDist = m_correspondences[i].first.distanceToPoint(m_correspondences[j].first);
            float tgtDist = m_correspondences[i].second.distanceToPoint(m_correspondences[j].second);

            if (srcDist < 0.001f || tgtDist < 0.001f)
            {  // 1mm minimum separation
                QString message = QString("Duplicate correspondences detected at indices %1 and %2").arg(i).arg(j);
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
    if (state == AlignmentState::Error || state == AlignmentState::Insufficient)
    {
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
    if (m_autoRecompute)
    {
        recomputeAlignment();
    }
}

void AlignmentEngine::setQualityThresholds(float rmsThreshold, float maxErrorThreshold)
{
    m_rmsThreshold = rmsThreshold;
    m_maxErrorThreshold = maxErrorThreshold;

    qDebug() << "Quality thresholds updated - RMS:" << rmsThreshold << "mm, Max:" << maxErrorThreshold << "mm";

    // Revalidate current result if available
    if (m_currentResult.state == AlignmentState::Valid)
    {
        triggerRecomputeIfEnabled();
    }
}

void AlignmentEngine::startTargetDetection(const QString& scanId, int mode, const QVariantMap& params)
{
    qDebug() << "Starting target detection for scan:" << scanId << "with mode:" << mode;

    if (!m_loadManager)
    {
        emit targetDetectionError("Point cloud load manager not available");
        return;
    }

    if (!m_targetManager)
    {
        emit targetDetectionError("Target manager not available");
        return;
    }

    // Emit initial progress
    emit targetDetectionProgress(0, "Initializing target detection...");

    // Get point cloud data from PointCloudLoadManager
    std::vector<PointFullData> points = m_loadManager->getLoadedPointFullData(scanId);

    if (points.empty())
    {
        emit targetDetectionError("No point cloud data available for scan: " + scanId);
        return;
    }

    emit targetDetectionProgress(10, "Point cloud data loaded...");

    // Convert QVariantMap to DetectionParams
    TargetDetectionBase::DetectionParams detectionParams;
    detectionParams.fromVariantMap(params);

    // Check if mode includes automatic spheres (mode 0 = AutomaticSpheres, mode 2 = Both)
    if (mode == 0 || mode == 2)
    {
        // Create and configure SphereDetector
        m_sphereDetector = std::make_unique<SphereDetector>(this);

        // Connect detector signals to relay slots
        connect(m_sphereDetector.get(), &TargetDetectionBase::detectionProgress,
                this, &AlignmentEngine::onDetectionProgress);
        connect(m_sphereDetector.get(), &TargetDetectionBase::detectionCompleted,
                this, &AlignmentEngine::onDetectionCompleted);
        connect(m_sphereDetector.get(), &TargetDetectionBase::detectionError,
                this, &AlignmentEngine::onDetectionError);

        emit targetDetectionProgress(20, "Starting sphere detection...");

        // Start asynchronous detection
        m_sphereDetector->detectAsync(points, detectionParams);
    }
    else
    {
        // For manual modes, just emit completion with empty result
        TargetDetectionBase::DetectionResult emptyResult;
        emptyResult.success = true;
        emptyResult.processedPoints = static_cast<int>(points.size());
        emit targetDetectionCompleted(emptyResult);
    }
}

void AlignmentEngine::cancelTargetDetection()
{
    qDebug() << "Cancelling target detection";

    if (m_sphereDetector)
    {
        m_sphereDetector->cancel();
    }
}

void AlignmentEngine::onDetectionProgress(int percentage, const QString& stage)
{
    // Relay progress signal from detector
    emit targetDetectionProgress(percentage, stage);
}

void AlignmentEngine::onDetectionCompleted(const TargetDetectionBase::DetectionResult& result)
{
    qDebug() << "Target detection completed with" << result.targets.size() << "targets";

    if (m_targetManager && result.success)
    {
        // Add all detected targets to the target manager
        int addedCount = 0;
        for (const auto& target : result.targets)
        {
            if (m_targetManager->addTarget(target->scanId(), target))
            {
                addedCount++;
            }
        }
        qDebug() << "Added" << addedCount << "targets to TargetManager";
    }

    // Relay completion signal
    emit targetDetectionCompleted(result);

    // Clean up detector
    m_sphereDetector.reset();
}

void AlignmentEngine::onDetectionError(const QString& error)
{
    qDebug() << "Target detection error:" << error;

    // Relay error signal
    emit targetDetectionError(error);

    // Clean up detector
    m_sphereDetector.reset();
}

void AlignmentEngine::startAutomaticAlignment(const QString& sourceScanId,
                                            const QString& targetScanId,
                                            const ICPParams& params)
{
    qDebug() << "Starting automatic alignment between" << sourceScanId << "and" << targetScanId;

    // Store scan IDs for reference
    m_currentSourceScanId = sourceScanId;
    m_currentTargetScanId = targetScanId;

    // Create a new ICP algorithm instance
    m_icpAlgorithm = std::make_unique<ICPRegistration>();

    // Connect signals for progress updates
    connect(m_icpAlgorithm.get(), &ICPRegistration::progressUpdated,
            this, [this](int iteration, float rmsError, const QMatrix4x4& transformation) {
        // Update current result with intermediate transformation
        m_currentResult.transformation = transformation;
        m_currentResult.errorStats.rmsError = rmsError;
        m_currentResult.state = AlignmentState::Computing;
        m_currentResult.message = QString("ICP iteration %1, RMS error: %2 mm")
                                    .arg(iteration)
                                    .arg(rmsError, 0, 'f', 3);

        // Emit signals for real-time update
        emit transformationUpdated(transformation);
        emit qualityMetricsUpdated(rmsError);
        emit alignmentResultUpdated(m_currentResult);
        emit alignmentStateChanged(AlignmentState::Computing, m_currentResult.message);

        // Relay progress signal for ICPProgressWidget
        emit progressUpdated(iteration, rmsError, transformation);

        qDebug() << "ICP progress:" << iteration << "iterations, RMS:" << rmsError;
    });

    // Connect signal for completion
    connect(m_icpAlgorithm.get(), &ICPRegistration::computationFinished,
            this, [this](bool success, const QMatrix4x4& finalTransform, float finalError, int iterations) {
        // Update final result
        m_currentResult.transformation = finalTransform;
        m_currentResult.errorStats.rmsError = finalError;

        if (success)
        {
            m_currentResult.state = AlignmentState::Valid;
            m_currentResult.message = QString("ICP completed successfully after %1 iterations (RMS: %2 mm)")
                                        .arg(iterations)
                                        .arg(finalError, 0, 'f', 3);
        }
        else
        {
            m_currentResult.state = AlignmentState::Error;
            m_currentResult.message = "ICP computation failed or was cancelled";
        }

        // Emit signals for final update
        emit transformationUpdated(finalTransform);
        emit qualityMetricsUpdated(finalError);
        emit alignmentResultUpdated(m_currentResult);
        emit alignmentStateChanged(m_currentResult.state, m_currentResult.message);

        // Relay completion signal for ICPProgressWidget
        emit computationFinished(success, finalTransform, finalError, iterations);

        qDebug() << "ICP computation finished. Success:" << success
                 << "Iterations:" << iterations
                 << "Final RMS:" << finalError;
    });

    // TODO: Retrieve point clouds from PointCloudLoadManager
    // For now, we'll use empty point clouds as placeholders
    PointCloud sourceCloud;
    PointCloud targetCloud;

    // Set initial transformation to identity
    QMatrix4x4 initialGuess;
    initialGuess.setToIdentity();

    // Start the computation asynchronously
    QMetaObject::invokeMethod(m_icpAlgorithm.get(), [this, sourceCloud, targetCloud, initialGuess, params]() {
        m_icpAlgorithm->compute(sourceCloud, targetCloud, initialGuess, params);
    }, Qt::QueuedConnection);

    // Update current state
    m_currentResult.state = AlignmentState::Computing;
    m_currentResult.message = "Starting ICP computation...";
    emit alignmentStateChanged(AlignmentState::Computing, m_currentResult.message);
}

void AlignmentEngine::cancelAutomaticAlignment()
{
    if (m_icpAlgorithm && m_icpAlgorithm->isRunning())
    {
        qDebug() << "Cancelling automatic alignment";
        m_icpAlgorithm->cancel();

        // Update state
        m_currentResult.state = AlignmentState::Cancelled;
        m_currentResult.message = "ICP computation cancelled by user";
        emit alignmentStateChanged(AlignmentState::Cancelled, m_currentResult.message);
    }
}
}

// Sprint 6.1: Deviation analysis implementation
std::vector<PointFullData> AlignmentEngine::analyzeDeviation(const std::vector<PointFullData>& source,
                                                            const std::vector<PointFullData>& target,
                                                            const QMatrix4x4& transform)
{
    qDebug() << "Starting deviation analysis with" << source.size() << "source and" << target.size() << "target points";

    std::vector<PointFullData> colorizedPoints;

    if (source.empty() || target.empty())
    {
        qWarning() << "Empty point clouds provided for deviation analysis";
        return colorizedPoints;
    }

    // Convert PointFullData to Point3D for analysis
    std::vector<Point3D> sourcePoints3D;
    std::vector<Point3D> targetPoints3D;

    sourcePoints3D.reserve(source.size());
    targetPoints3D.reserve(target.size());

    for (const auto& point : source)
    {
        sourcePoints3D.emplace_back(point.x, point.y, point.z);
    }

    for (const auto& point : target)
    {
        targetPoints3D.emplace_back(point.x, point.y, point.z);
    }

    // Perform difference analysis
    Analysis::DifferenceAnalysis analyzer;
    QVector<float> distances = analyzer.calculateDistances(sourcePoints3D, targetPoints3D, transform);

    // Set a reasonable max distance (5cm default, or from preferences)
    m_lastDeviationMaxDistance = 0.05f;

    // Generate colors based on distances
    QVector<QColor> colors = analyzer.generateColorMapColors(distances, m_lastDeviationMaxDistance);

    // Create colorized point cloud
    colorizedPoints.reserve(source.size());

    for (size_t i = 0; i < source.size() && i < static_cast<size_t>(colors.size()); ++i)
    {
        PointFullData colorizedPoint = source[i];

        // Apply transformation to position
        QVector3D pos(colorizedPoint.x, colorizedPoint.y, colorizedPoint.z);
        QVector3D transformedPos = transform.map(pos);
        colorizedPoint.x = transformedPos.x();
        colorizedPoint.y = transformedPos.y();
        colorizedPoint.z = transformedPos.z();

        // Set deviation color
        QColor color = colors[static_cast<int>(i)];
        colorizedPoint.r = static_cast<uint8_t>(color.red());
        colorizedPoint.g = static_cast<uint8_t>(color.green());
        colorizedPoint.b = static_cast<uint8_t>(color.blue());

        colorizedPoints.push_back(colorizedPoint);
    }

    qDebug() << "Deviation analysis completed. Generated" << colorizedPoints.size() << "colorized points";
    qDebug() << "Max deviation distance:" << m_lastDeviationMaxDistance << "m";

    return colorizedPoints;
}
