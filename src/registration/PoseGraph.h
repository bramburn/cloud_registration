#pragma once

#include <QString>
#include <QMatrix4x4>
#include <QList>

namespace Registration {

/**
 * @brief Represents a node in the pose graph containing scan pose information
 */
struct PoseNode {
    QString scanId;
    QMatrix4x4 transform;
    int nodeIndex;
    
    PoseNode() : nodeIndex(-1) {}
    PoseNode(const QString& id, const QMatrix4x4& t, int idx = -1) 
        : scanId(id), transform(t), nodeIndex(idx) {}
};

/**
 * @brief Represents an edge in the pose graph connecting two scans
 */
struct PoseEdge {
    int fromNodeIndex;
    int toNodeIndex;
    QMatrix4x4 relativeTransform;
    float informationMatrix; // Confidence weight (inverse of RMS error)
    float rmsError;
    
    PoseEdge() : fromNodeIndex(-1), toNodeIndex(-1), informationMatrix(1.0f), rmsError(0.0f) {}
    PoseEdge(int from, int to, const QMatrix4x4& transform, float info = 1.0f, float rms = 0.0f)
        : fromNodeIndex(from), toNodeIndex(to), relativeTransform(transform), 
          informationMatrix(info), rmsError(rms) {}
};

/**
 * @brief Graph structure representing scan poses and their relationships
 */
class PoseGraph {
public:
    PoseGraph() = default;
    
    // Node management
    int addNode(const QString& scanId, const QMatrix4x4& transform = QMatrix4x4());
    bool removeNode(int nodeIndex);
    PoseNode* getNode(int nodeIndex);
    const PoseNode* getNode(int nodeIndex) const;
    int findNodeByScanId(const QString& scanId) const;
    
    // Edge management
    bool addEdge(int fromNode, int toNode, const QMatrix4x4& relativeTransform, 
                 float rmsError = 0.0f);
    bool removeEdge(int fromNode, int toNode);
    QList<PoseEdge> getEdgesFromNode(int nodeIndex) const;
    QList<PoseEdge> getEdgesToNode(int nodeIndex) const;
    
    // Graph properties
    int nodeCount() const { return m_nodes.size(); }
    int edgeCount() const { return m_edges.size(); }
    bool isEmpty() const { return m_nodes.isEmpty(); }
    
    // Access to all nodes and edges
    const QList<PoseNode>& nodes() const { return m_nodes; }
    const QList<PoseEdge>& edges() const { return m_edges; }
    QList<PoseNode>& nodes() { return m_nodes; }
    QList<PoseEdge>& edges() { return m_edges; }
    
    // Graph validation
    bool isValid() const;
    bool hasLoopClosures() const;
    
    // Clear all data
    void clear();
    
private:
    QList<PoseNode> m_nodes;
    QList<PoseEdge> m_edges;
    int m_nextNodeIndex = 0;
    
    // Helper methods
    bool isValidNodeIndex(int index) const;
    int getEdgeIndex(int fromNode, int toNode) const;
};

} // namespace Registration
