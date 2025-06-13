#pragma once

#include "PoseGraph.h"
#include "core/project.h"
#include <QObject>
#include <memory>

namespace Registration {

/**
 * @brief Builds pose graphs from registration project data
 */
class PoseGraphBuilder : public QObject {
    Q_OBJECT
    
public:
    explicit PoseGraphBuilder(QObject* parent = nullptr);
    
    /**
     * @brief Build a pose graph from a registration project
     * @param project The project containing scan data and registrations
     * @return Constructed pose graph
     */
    std::unique_ptr<PoseGraph> build(const Project& project);
    
    /**
     * @brief Build pose graph from scan list with identity transforms
     * @param scanIds List of scan identifiers
     * @return Pose graph with identity transforms
     */
    std::unique_ptr<PoseGraph> buildFromScans(const QStringList& scanIds);
    
    /**
     * @brief Add registration result as edge to existing pose graph
     * @param graph Target pose graph
     * @param sourceScanId Source scan identifier
     * @param targetScanId Target scan identifier
     * @param transform Relative transformation
     * @param rmsError Registration RMS error
     * @return Success status
     */
    bool addRegistrationEdge(PoseGraph& graph, 
                           const QString& sourceScanId,
                           const QString& targetScanId,
                           const QMatrix4x4& transform,
                           float rmsError = 0.0f);
    
    /**
     * @brief Validate pose graph connectivity and structure
     * @param graph Pose graph to validate
     * @return Validation result with details
     */
    struct ValidationResult {
        bool isValid = false;
        QString errorMessage;
        int connectedComponents = 0;
        bool hasLoops = false;
        QStringList isolatedScans;
    };
    
    ValidationResult validateGraph(const PoseGraph& graph) const;
    
signals:
    void buildProgress(int percentage);
    void buildCompleted(bool success);
    void validationCompleted(const ValidationResult& result);
    
private:
    /**
     * @brief Extract registration data from project
     * @param project Source project
     * @return List of registration relationships
     */
    struct RegistrationData {
        QString sourceScanId;
        QString targetScanId;
        QMatrix4x4 transform;
        float rmsError;
    };
    
    QList<RegistrationData> extractRegistrations(const Project& project) const;
    
    /**
     * @brief Find connected components in the pose graph
     * @param graph Input pose graph
     * @return List of connected component node sets
     */
    QList<QList<int>> findConnectedComponents(const PoseGraph& graph) const;
    
    /**
     * @brief Perform depth-first search for connectivity analysis
     * @param graph Input pose graph
     * @param startNode Starting node index
     * @param visited Set of visited nodes
     * @param component Current component being built
     */
    void dfsVisit(const PoseGraph& graph, int startNode, 
                  QSet<int>& visited, QList<int>& component) const;
};

} // namespace Registration
