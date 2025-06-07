#pragma once

#include "../registration/PoseGraph.h"
#include <QObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QElapsedTimer>
#include <memory>
#include <vector>

namespace Optimization {

/**
 * @brief Bundle adjustment optimization for global pose graph optimization
 * 
 * Implements Levenberg-Marquardt algorithm to minimize global registration error
 * across all scan poses simultaneously.
 */
class BundleAdjustment : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Optimization parameters
     */
    struct Parameters {
        int maxIterations = 100;
        double convergenceThreshold = 1e-6;
        double initialLambda = 1e-3;
        double lambdaFactor = 10.0;
        double maxLambda = 1e10;
        bool verbose = false;
        
        // Pose parameterization (6-DOF: 3 translation + 3 rotation)
        bool fixFirstPose = true; // Keep first scan at origin
    };
    
    /**
     * @brief Optimization result
     */
    struct Result {
        bool converged = false;
        int iterations = 0;
        double finalError = 0.0;
        double initialError = 0.0;
        double improvementRatio = 0.0;
        QString statusMessage;
        
        // Timing information
        double optimizationTimeSeconds = 0.0;
    };
    
    explicit BundleAdjustment(QObject* parent = nullptr);
    
    /**
     * @brief Optimize pose graph using bundle adjustment
     * @param initialGraph Input pose graph with initial poses
     * @param params Optimization parameters
     * @return Optimized pose graph and result information
     */
    std::pair<std::unique_ptr<Registration::PoseGraph>, Result> 
    optimize(const Registration::PoseGraph& initialGraph, 
             const Parameters& params = Parameters());
    
    /**
     * @brief Get recommended parameters based on graph characteristics
     * @param graph Input pose graph
     * @return Recommended optimization parameters
     */
    Parameters getRecommendedParameters(const Registration::PoseGraph& graph) const;
    
signals:
    void optimizationProgress(int iteration, double currentError, double lambda);
    void optimizationCompleted(const Result& result);
    void errorOccurred(const QString& message);
    
private:
    /**
     * @brief 6-DOF pose representation (translation + axis-angle rotation)
     */
    struct Pose6DOF {
        QVector3D translation;
        QVector3D rotation; // Axis-angle representation
        
        Pose6DOF() = default;
        Pose6DOF(const QMatrix4x4& transform);
        QMatrix4x4 toMatrix() const;
        
        // Vector operations for optimization
        std::vector<double> toVector() const;
        void fromVector(const std::vector<double>& vec);
    };
    
    /**
     * @brief State vector containing all pose parameters
     */
    class StateVector {
    public:
        StateVector(const Registration::PoseGraph& graph, bool fixFirst = true);
        
        void updatePoses(const std::vector<double>& delta);
        std::vector<double> getPoseVector() const;
        void setPoseVector(const std::vector<double>& poses);
        
        QMatrix4x4 getPoseMatrix(int nodeIndex) const;
        void setPoseMatrix(int nodeIndex, const QMatrix4x4& transform);
        
        int getParameterCount() const { return m_parameterCount; }
        const QList<int>& getOptimizedNodes() const { return m_optimizedNodes; }
        
    private:
        QList<Pose6DOF> m_poses;
        QList<int> m_optimizedNodes; // Node indices being optimized
        int m_parameterCount;
        bool m_fixFirstPose;
    };
    
    /**
     * @brief Calculate total error for current state
     */
    double calculateTotalError(const StateVector& state, 
                              const Registration::PoseGraph& graph) const;
    
    /**
     * @brief Calculate error for a single edge constraint
     */
    double calculateEdgeError(const Registration::PoseEdge& edge,
                             const QMatrix4x4& sourcePose,
                             const QMatrix4x4& targetPose) const;
    
    /**
     * @brief Compute Jacobian matrix numerically
     */
    std::vector<std::vector<double>> computeJacobian(const StateVector& state,
                                                    const Registration::PoseGraph& graph) const;
    
    /**
     * @brief Solve linear system (J^T * J + lambda * I) * delta = J^T * error
     */
    std::vector<double> solveLinearSystem(const std::vector<std::vector<double>>& jacobian,
                                         const std::vector<double>& errorVector,
                                         double lambda) const;
    
    /**
     * @brief Convert pose graph back to optimized result
     */
    std::unique_ptr<Registration::PoseGraph> 
    createOptimizedGraph(const Registration::PoseGraph& originalGraph,
                        const StateVector& optimizedState) const;
    
    // Numerical differentiation step size
    static constexpr double NUMERICAL_EPSILON = 1e-8;
};

} // namespace Optimization
