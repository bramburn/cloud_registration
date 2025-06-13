#include "BundleAdjustment.h"

#include <QDebug>
#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

namespace Optimization
{
BundleAdjustment::BundleAdjustment(QObject* parent) : QObject(parent), m_isCancelled(false) {}

std::pair<std::unique_ptr<Registration::PoseGraph>, BundleAdjustment::Result>
BundleAdjustment::optimize(const Registration::PoseGraph& initialGraph, const Parameters& params)
{
    QElapsedTimer timer;
    timer.start();

    // Reset cancellation flag
    m_isCancelled = false;

    Result result;

    try
    {
        if (initialGraph.isEmpty() || initialGraph.edgeCount() == 0)
        {
            result.statusMessage = "Empty or disconnected pose graph";
            emit errorOccurred(result.statusMessage);
            return {std::make_unique<Registration::PoseGraph>(initialGraph), result};
        }

        // Initialize state vector
        StateVector state(initialGraph, params.fixFirstPose);

        if (state.getParameterCount() == 0)
        {
            result.statusMessage = "No parameters to optimize";
            result.converged = true;
            return {std::make_unique<Registration::PoseGraph>(initialGraph), result};
        }

        // Calculate initial error
        result.initialError = calculateTotalError(state, initialGraph);
        double currentError = result.initialError;
        double lambda = params.initialLambda;

        qDebug() << "Starting bundle adjustment with" << state.getParameterCount()
                 << "parameters, initial error:" << result.initialError;

        // Levenberg-Marquardt optimization loop
        for (int iteration = 0; iteration < params.maxIterations; ++iteration)
        {
            // Check for cancellation
            if (m_isCancelled)
            {
                result.statusMessage = "Optimization cancelled by user";
                result.optimizationTimeSeconds = timer.elapsed() / 1000.0;
                emit optimizationCompleted(result);
                return {std::make_unique<Registration::PoseGraph>(initialGraph), result};
            }
            // Compute Jacobian and error vector
            std::vector<std::vector<double>> jacobian = computeJacobian(state, initialGraph);

            // Calculate error vector for each edge
            std::vector<double> errorVector;
            for (const auto& edge : initialGraph.edges())
            {
                QMatrix4x4 sourcePose = state.getPoseMatrix(edge.fromNodeIndex);
                QMatrix4x4 targetPose = state.getPoseMatrix(edge.toNodeIndex);
                double edgeError = calculateEdgeError(edge, sourcePose, targetPose);
                errorVector.push_back(std::sqrt(edgeError * edge.informationMatrix));
            }

            // Solve linear system
            std::vector<double> delta = solveLinearSystem(jacobian, errorVector, lambda);

            // Create candidate state
            StateVector candidateState = state;
            candidateState.updatePoses(delta);

            // Calculate new error
            double newError = calculateTotalError(candidateState, initialGraph);

            emit optimizationProgress(iteration, currentError, lambda);

            if (params.verbose)
            {
                qDebug() << "Iteration" << iteration << "Error:" << currentError << "Lambda:" << lambda << "Delta norm:"
                         << std::sqrt(std::inner_product(delta.begin(), delta.end(), delta.begin(), 0.0));
            }

            // Check for improvement
            if (newError < currentError)
            {
                // Accept the step
                state = candidateState;
                double errorReduction = currentError - newError;
                currentError = newError;
                lambda /= params.lambdaFactor;

                // Check convergence
                if (errorReduction < params.convergenceThreshold)
                {
                    result.converged = true;
                    result.statusMessage = "Converged due to small error reduction";
                    break;
                }
            }
            else
            {
                // Reject the step, increase damping
                lambda *= params.lambdaFactor;
                if (lambda > params.maxLambda)
                {
                    result.statusMessage = "Lambda exceeded maximum value";
                    break;
                }
            }

            result.iterations = iteration + 1;
        }

        if (!result.converged && result.iterations >= params.maxIterations)
        {
            result.statusMessage = "Reached maximum iterations";
        }

        result.finalError = currentError;
        result.improvementRatio =
            (result.initialError > 0) ? (result.initialError - result.finalError) / result.initialError : 0.0;
        result.optimizationTimeSeconds = timer.elapsed() / 1000.0;

        qDebug() << "Bundle adjustment completed:" << result.statusMessage << "Iterations:" << result.iterations
                 << "Error reduction:" << (result.improvementRatio * 100) << "%"
                 << "Time:" << result.optimizationTimeSeconds << "s";

        // Create optimized pose graph
        auto optimizedGraph = createOptimizedGraph(initialGraph, state);

        emit optimizationCompleted(result);
        return {std::move(optimizedGraph), result};
    }
    catch (const std::exception& e)
    {
        result.statusMessage = QString("Optimization failed: %1").arg(e.what());
        result.optimizationTimeSeconds = timer.elapsed() / 1000.0;
        emit errorOccurred(result.statusMessage);
        return {std::make_unique<Registration::PoseGraph>(initialGraph), result};
    }
}

BundleAdjustment::Parameters BundleAdjustment::getRecommendedParameters(const Registration::PoseGraph& graph) const
{
    Parameters params;

    // Adjust parameters based on graph size
    int nodeCount = graph.nodeCount();
    int edgeCount = graph.edgeCount();

    if (nodeCount > 20)
    {
        params.maxIterations = 200;
        params.convergenceThreshold = 1e-7;
    }
    else if (nodeCount > 10)
    {
        params.maxIterations = 150;
        params.convergenceThreshold = 1e-6;
    }

    // Adjust lambda based on connectivity
    if (graph.hasLoopClosures())
    {
        params.initialLambda = 1e-4;  // More aggressive for well-connected graphs
    }

    params.verbose = (nodeCount <= 10);  // Enable verbose for small graphs

    return params;
}

void BundleAdjustment::cancel()
{
    m_isCancelled = true;
    qDebug() << "Bundle adjustment cancellation requested";
}

double BundleAdjustment::calculateTotalError(const StateVector& state, const Registration::PoseGraph& graph) const
{
    double totalError = 0.0;

    for (const auto& edge : graph.edges())
    {
        QMatrix4x4 sourcePose = state.getPoseMatrix(edge.fromNodeIndex);
        QMatrix4x4 targetPose = state.getPoseMatrix(edge.toNodeIndex);
        double edgeError = calculateEdgeError(edge, sourcePose, targetPose);
        totalError += edgeError * edge.informationMatrix;
    }

    return totalError;
}

double BundleAdjustment::calculateEdgeError(const Registration::PoseEdge& edge,
                                            const QMatrix4x4& sourcePose,
                                            const QMatrix4x4& targetPose) const
{
    // Calculate relative transformation error
    QMatrix4x4 measuredTransform = edge.relativeTransform;
    QMatrix4x4 currentTransform = targetPose * sourcePose.inverted();

    // Compute transformation difference
    QMatrix4x4 errorTransform = currentTransform * measuredTransform.inverted();

    // Extract translation and rotation errors
    QVector3D translationError(errorTransform(0, 3), errorTransform(1, 3), errorTransform(2, 3));

    // Simple rotation error approximation (could be improved with proper SO(3) distance)
    double rotationError = std::abs(errorTransform(0, 0) + errorTransform(1, 1) + errorTransform(2, 2) - 3.0);

    return translationError.lengthSquared() + rotationError * rotationError;
}

std::vector<std::vector<double>> BundleAdjustment::computeJacobian(const StateVector& state,
                                                                   const Registration::PoseGraph& graph) const
{
    int numEdges = graph.edgeCount();
    int numParams = state.getParameterCount();

    std::vector<std::vector<double>> jacobian(numEdges, std::vector<double>(numParams, 0.0));

    // Numerical differentiation
    std::vector<double> currentPoses = state.getPoseVector();
    double baseError = calculateTotalError(state, graph);

    for (int paramIdx = 0; paramIdx < numParams; ++paramIdx)
    {
        // Perturb parameter
        std::vector<double> perturbedPoses = currentPoses;
        perturbedPoses[paramIdx] += NUMERICAL_EPSILON;

        StateVector perturbedState = state;
        perturbedState.setPoseVector(perturbedPoses);

        double perturbedError = calculateTotalError(perturbedState, graph);

        // Compute derivative for each edge
        int edgeIdx = 0;
        for (const auto& edge : graph.edges())
        {
            QMatrix4x4 sourcePose = perturbedState.getPoseMatrix(edge.fromNodeIndex);
            QMatrix4x4 targetPose = perturbedState.getPoseMatrix(edge.toNodeIndex);
            double edgeError = calculateEdgeError(edge, sourcePose, targetPose);

            QMatrix4x4 baseSrcPose = state.getPoseMatrix(edge.fromNodeIndex);
            QMatrix4x4 baseTgtPose = state.getPoseMatrix(edge.toNodeIndex);
            double baseEdgeError = calculateEdgeError(edge, baseSrcPose, baseTgtPose);

            jacobian[edgeIdx][paramIdx] = (edgeError - baseEdgeError) / NUMERICAL_EPSILON;
            edgeIdx++;
        }
    }

    return jacobian;
}

std::vector<double> BundleAdjustment::solveLinearSystem(const std::vector<std::vector<double>>& jacobian,
                                                        const std::vector<double>& errorVector,
                                                        double lambda) const
{
    int numParams = jacobian.empty() ? 0 : jacobian[0].size();
    int numEdges = jacobian.size();

    if (numParams == 0 || numEdges == 0)
    {
        return std::vector<double>();
    }

    // Compute J^T * J
    std::vector<std::vector<double>> JtJ(numParams, std::vector<double>(numParams, 0.0));
    for (int i = 0; i < numParams; ++i)
    {
        for (int j = 0; j < numParams; ++j)
        {
            for (int k = 0; k < numEdges; ++k)
            {
                JtJ[i][j] += jacobian[k][i] * jacobian[k][j];
            }
        }
    }

    // Add damping term (lambda * I)
    for (int i = 0; i < numParams; ++i)
    {
        JtJ[i][i] += lambda;
    }

    // Compute J^T * error
    std::vector<double> JtError(numParams, 0.0);
    for (int i = 0; i < numParams; ++i)
    {
        for (int k = 0; k < numEdges; ++k)
        {
            JtError[i] += jacobian[k][i] * errorVector[k];
        }
    }

    // Solve using simple Gaussian elimination (could be improved with better solver)
    std::vector<double> delta(numParams, 0.0);

    // Forward elimination
    for (int i = 0; i < numParams; ++i)
    {
        // Find pivot
        int maxRow = i;
        for (int k = i + 1; k < numParams; ++k)
        {
            if (std::abs(JtJ[k][i]) > std::abs(JtJ[maxRow][i]))
            {
                maxRow = k;
            }
        }

        if (std::abs(JtJ[maxRow][i]) < 1e-12)
        {
            continue;  // Singular matrix
        }

        // Swap rows
        if (maxRow != i)
        {
            std::swap(JtJ[i], JtJ[maxRow]);
            std::swap(JtError[i], JtError[maxRow]);
        }

        // Eliminate
        for (int k = i + 1; k < numParams; ++k)
        {
            double factor = JtJ[k][i] / JtJ[i][i];
            for (int j = i; j < numParams; ++j)
            {
                JtJ[k][j] -= factor * JtJ[i][j];
            }
            JtError[k] -= factor * JtError[i];
        }
    }

    // Back substitution
    for (int i = numParams - 1; i >= 0; --i)
    {
        delta[i] = JtError[i];
        for (int j = i + 1; j < numParams; ++j)
        {
            delta[i] -= JtJ[i][j] * delta[j];
        }
        if (std::abs(JtJ[i][i]) > 1e-12)
        {
            delta[i] /= JtJ[i][i];
        }
    }

    return delta;
}

std::unique_ptr<Registration::PoseGraph>
BundleAdjustment::createOptimizedGraph(const Registration::PoseGraph& originalGraph,
                                       const StateVector& optimizedState) const
{
    auto optimizedGraph = std::make_unique<Registration::PoseGraph>(originalGraph);

    // Update node poses with optimized values
    for (auto& node : optimizedGraph->nodes())
    {
        QMatrix4x4 optimizedPose = optimizedState.getPoseMatrix(node.nodeIndex);
        node.transform = optimizedPose;
    }

    return optimizedGraph;
}

// Pose6DOF implementation
BundleAdjustment::Pose6DOF::Pose6DOF(const QMatrix4x4& transform)
{
    // Extract translation
    translation = QVector3D(transform(0, 3), transform(1, 3), transform(2, 3));

    // Extract rotation as axis-angle (simplified implementation)
    // This is a basic implementation - could be improved with proper rotation extraction
    QMatrix3x3 rotMatrix;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            rotMatrix(i, j) = transform(i, j);
        }
    }

    // Convert to axis-angle (simplified)
    double trace = rotMatrix(0, 0) + rotMatrix(1, 1) + rotMatrix(2, 2);
    double angle = std::acos(std::max(-1.0, std::min(1.0, (trace - 1.0) / 2.0)));

    if (std::abs(angle) < 1e-6)
    {
        rotation = QVector3D(0, 0, 0);
    }
    else
    {
        QVector3D axis(
            rotMatrix(2, 1) - rotMatrix(1, 2), rotMatrix(0, 2) - rotMatrix(2, 0), rotMatrix(1, 0) - rotMatrix(0, 1));
        axis.normalize();
        rotation = axis * static_cast<float>(angle);
    }
}

QMatrix4x4 BundleAdjustment::Pose6DOF::toMatrix() const
{
    QMatrix4x4 result;
    result.setToIdentity();

    // Set translation
    result(0, 3) = translation.x();
    result(1, 3) = translation.y();
    result(2, 3) = translation.z();

    // Set rotation from axis-angle
    float angle = rotation.length();
    if (angle > 1e-6f)
    {
        QVector3D axis = rotation.normalized();

        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1.0f - c;

        float x = axis.x(), y = axis.y(), z = axis.z();

        result(0, 0) = t * x * x + c;
        result(0, 1) = t * x * y - s * z;
        result(0, 2) = t * x * z + s * y;
        result(1, 0) = t * x * y + s * z;
        result(1, 1) = t * y * y + c;
        result(1, 2) = t * y * z - s * x;
        result(2, 0) = t * x * z - s * y;
        result(2, 1) = t * y * z + s * x;
        result(2, 2) = t * z * z + c;
    }

    return result;
}

std::vector<double> BundleAdjustment::Pose6DOF::toVector() const
{
    return {static_cast<double>(translation.x()),
            static_cast<double>(translation.y()),
            static_cast<double>(translation.z()),
            static_cast<double>(rotation.x()),
            static_cast<double>(rotation.y()),
            static_cast<double>(rotation.z())};
}

void BundleAdjustment::Pose6DOF::fromVector(const std::vector<double>& vec)
{
    if (vec.size() >= 6)
    {
        translation = QVector3D(static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2]));
        rotation = QVector3D(static_cast<float>(vec[3]), static_cast<float>(vec[4]), static_cast<float>(vec[5]));
    }
}

// StateVector implementation
BundleAdjustment::StateVector::StateVector(const Registration::PoseGraph& graph, bool fixFirst)
    : m_fixFirstPose(fixFirst), m_parameterCount(0)
{
    const auto& nodes = graph.nodes();

    for (int i = 0; i < nodes.size(); ++i)
    {
        const auto& node = nodes[i];

        if (m_fixFirstPose && i == 0)
        {
            // Keep first pose fixed at identity
            m_poses.append(Pose6DOF(QMatrix4x4()));
        }
        else
        {
            m_poses.append(Pose6DOF(node.transform));
            m_optimizedNodes.append(node.nodeIndex);
            m_parameterCount += 6;  // 6 DOF per pose
        }
    }
}

void BundleAdjustment::StateVector::updatePoses(const std::vector<double>& delta)
{
    if (delta.size() != static_cast<size_t>(m_parameterCount))
    {
        qWarning() << "Delta size mismatch:" << delta.size() << "vs" << m_parameterCount;
        return;
    }

    int deltaIdx = 0;
    for (int i = 0; i < m_poses.size(); ++i)
    {
        if (m_fixFirstPose && i == 0)
        {
            continue;  // Skip first pose
        }

        std::vector<double> currentPose = m_poses[i].toVector();
        for (int j = 0; j < 6; ++j)
        {
            currentPose[j] += delta[deltaIdx++];
        }
        m_poses[i].fromVector(currentPose);
    }
}

std::vector<double> BundleAdjustment::StateVector::getPoseVector() const
{
    std::vector<double> result;
    result.reserve(m_parameterCount);

    for (int i = 0; i < m_poses.size(); ++i)
    {
        if (m_fixFirstPose && i == 0)
        {
            continue;  // Skip first pose
        }

        std::vector<double> poseVec = m_poses[i].toVector();
        result.insert(result.end(), poseVec.begin(), poseVec.end());
    }

    return result;
}

void BundleAdjustment::StateVector::setPoseVector(const std::vector<double>& poses)
{
    if (poses.size() != static_cast<size_t>(m_parameterCount))
    {
        qWarning() << "Pose vector size mismatch:" << poses.size() << "vs" << m_parameterCount;
        return;
    }

    int poseIdx = 0;
    for (int i = 0; i < m_poses.size(); ++i)
    {
        if (m_fixFirstPose && i == 0)
        {
            continue;  // Skip first pose
        }

        std::vector<double> poseVec(poses.begin() + poseIdx, poses.begin() + poseIdx + 6);
        m_poses[i].fromVector(poseVec);
        poseIdx += 6;
    }
}

QMatrix4x4 BundleAdjustment::StateVector::getPoseMatrix(int nodeIndex) const
{
    // Find pose by node index
    for (int i = 0; i < m_poses.size(); ++i)
    {
        if ((m_fixFirstPose && i == 0 && nodeIndex == 0) || (!m_fixFirstPose && i == nodeIndex) ||
            (m_fixFirstPose && i > 0 && m_optimizedNodes[i - 1] == nodeIndex))
        {
            return m_poses[i].toMatrix();
        }
    }

    qWarning() << "Node index not found:" << nodeIndex;
    return QMatrix4x4();
}

void BundleAdjustment::StateVector::setPoseMatrix(int nodeIndex, const QMatrix4x4& transform)
{
    // Find and update pose by node index
    for (int i = 0; i < m_poses.size(); ++i)
    {
        if ((m_fixFirstPose && i == 0 && nodeIndex == 0) || (!m_fixFirstPose && i == nodeIndex) ||
            (m_fixFirstPose && i > 0 && m_optimizedNodes[i - 1] == nodeIndex))
        {
            m_poses[i] = Pose6DOF(transform);
            return;
        }
    }

    qWarning() << "Node index not found for update:" << nodeIndex;
}

}  // namespace Optimization
