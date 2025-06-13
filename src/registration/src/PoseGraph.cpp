#include "registration/PoseGraph.h"

#include <QDebug>

#include <algorithm>

namespace Registration
{
int PoseGraph::addNode(const QString& scanId, const QMatrix4x4& transform)
{
    // Check if scan already exists
    int existingIndex = findNodeByScanId(scanId);
    if (existingIndex >= 0)
    {
        qWarning() << "Scan" << scanId << "already exists in pose graph at index" << existingIndex;
        return existingIndex;
    }

    int nodeIndex = m_nextNodeIndex++;
    m_nodes.append(PoseNode(scanId, transform, nodeIndex));

    qDebug() << "Added node" << nodeIndex << "for scan" << scanId;
    return nodeIndex;
}

bool PoseGraph::removeNode(int nodeIndex)
{
    if (!isValidNodeIndex(nodeIndex))
    {
        return false;
    }

    // Remove all edges connected to this node
    auto it = m_edges.begin();
    while (it != m_edges.end())
    {
        if (it->fromNodeIndex == nodeIndex || it->toNodeIndex == nodeIndex)
        {
            it = m_edges.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Remove the node
    auto nodeIt = std::find_if(
        m_nodes.begin(), m_nodes.end(), [nodeIndex](const PoseNode& node) { return node.nodeIndex == nodeIndex; });

    if (nodeIt != m_nodes.end())
    {
        m_nodes.erase(nodeIt);
        qDebug() << "Removed node" << nodeIndex;
        return true;
    }

    return false;
}

PoseNode* PoseGraph::getNode(int nodeIndex)
{
    auto it = std::find_if(
        m_nodes.begin(), m_nodes.end(), [nodeIndex](const PoseNode& node) { return node.nodeIndex == nodeIndex; });

    return (it != m_nodes.end()) ? &(*it) : nullptr;
}

const PoseNode* PoseGraph::getNode(int nodeIndex) const
{
    auto it = std::find_if(
        m_nodes.begin(), m_nodes.end(), [nodeIndex](const PoseNode& node) { return node.nodeIndex == nodeIndex; });

    return (it != m_nodes.end()) ? &(*it) : nullptr;
}

int PoseGraph::findNodeByScanId(const QString& scanId) const
{
    auto it =
        std::find_if(m_nodes.begin(), m_nodes.end(), [&scanId](const PoseNode& node) { return node.scanId == scanId; });

    return (it != m_nodes.end()) ? it->nodeIndex : -1;
}

bool PoseGraph::addEdge(int fromNode, int toNode, const QMatrix4x4& relativeTransform, float rmsError)
{
    if (!isValidNodeIndex(fromNode) || !isValidNodeIndex(toNode))
    {
        qWarning() << "Invalid node indices for edge:" << fromNode << "->" << toNode;
        return false;
    }

    if (fromNode == toNode)
    {
        qWarning() << "Cannot create self-loop edge for node" << fromNode;
        return false;
    }

    // Check if edge already exists
    int existingEdgeIndex = getEdgeIndex(fromNode, toNode);
    if (existingEdgeIndex >= 0)
    {
        // Update existing edge
        m_edges[existingEdgeIndex].relativeTransform = relativeTransform;
        m_edges[existingEdgeIndex].rmsError = rmsError;
        m_edges[existingEdgeIndex].informationMatrix = (rmsError > 0.0f) ? 1.0f / rmsError : 1.0f;
        qDebug() << "Updated edge" << fromNode << "->" << toNode;
        return true;
    }

    // Create new edge
    float information = (rmsError > 0.0f) ? 1.0f / rmsError : 1.0f;
    m_edges.append(PoseEdge(fromNode, toNode, relativeTransform, information, rmsError));

    qDebug() << "Added edge" << fromNode << "->" << toNode << "with RMS error" << rmsError;
    return true;
}

bool PoseGraph::removeEdge(int fromNode, int toNode)
{
    int edgeIndex = getEdgeIndex(fromNode, toNode);
    if (edgeIndex >= 0)
    {
        m_edges.removeAt(edgeIndex);
        qDebug() << "Removed edge" << fromNode << "->" << toNode;
        return true;
    }
    return false;
}

QList<PoseEdge> PoseGraph::getEdgesFromNode(int nodeIndex) const
{
    QList<PoseEdge> result;
    for (const auto& edge : m_edges)
    {
        if (edge.fromNodeIndex == nodeIndex)
        {
            result.append(edge);
        }
    }
    return result;
}

QList<PoseEdge> PoseGraph::getEdgesToNode(int nodeIndex) const
{
    QList<PoseEdge> result;
    for (const auto& edge : m_edges)
    {
        if (edge.toNodeIndex == nodeIndex)
        {
            result.append(edge);
        }
    }
    return result;
}

bool PoseGraph::isValid() const
{
    // Check that all edge node indices are valid
    for (const auto& edge : m_edges)
    {
        if (!isValidNodeIndex(edge.fromNodeIndex) || !isValidNodeIndex(edge.toNodeIndex))
        {
            return false;
        }
    }
    return true;
}

bool PoseGraph::hasLoopClosures() const
{
    // Simple check: if we have more edges than nodes-1, we likely have loops
    return m_edges.size() > (m_nodes.size() - 1);
}

void PoseGraph::clear()
{
    m_nodes.clear();
    m_edges.clear();
    m_nextNodeIndex = 0;
    qDebug() << "Cleared pose graph";
}

bool PoseGraph::isValidNodeIndex(int index) const
{
    return std::any_of(
        m_nodes.begin(), m_nodes.end(), [index](const PoseNode& node) { return node.nodeIndex == index; });
}

int PoseGraph::getEdgeIndex(int fromNode, int toNode) const
{
    for (int i = 0; i < m_edges.size(); ++i)
    {
        if (m_edges[i].fromNodeIndex == fromNode && m_edges[i].toNodeIndex == toNode)
        {
            return i;
        }
    }
    return -1;
}

void PoseGraph::updateNodeTransforms(const QMap<QString, QMatrix4x4>& newTransforms)
{
    for (auto& node : m_nodes) {
        if (newTransforms.contains(node.scanId)) {
            node.transform = newTransforms[node.scanId];
            qDebug() << "Updated transform for node" << node.scanId;
        }
    }
    qDebug() << "Updated transforms for" << newTransforms.size() << "nodes";
}

}  // namespace Registration
