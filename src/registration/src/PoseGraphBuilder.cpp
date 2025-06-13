#include "registration/PoseGraphBuilder.h"
#include "registration/RegistrationProject.h"

#include <QDebug>
#include <QSet>

#include <algorithm>

namespace Registration
{
PoseGraphBuilder::PoseGraphBuilder(QObject* parent) : QObject(parent) {}

std::unique_ptr<PoseGraph> PoseGraphBuilder::build(const RegistrationProject& project)
{
    emit buildProgress(0);

    auto graph = std::make_unique<PoseGraph>();

    try
    {
        // Get all scans from project
        QStringList scanIds = project.getScanIds();
        if (scanIds.isEmpty())
        {
            qWarning() << "No scans found in project";
            emit buildCompleted(false);
            return graph;
        }

        emit buildProgress(20);

        // Add nodes for each scan with identity transforms initially
        for (const QString& scanId : scanIds)
        {
            graph->addNode(scanId, QMatrix4x4());  // Identity transform
        }

        emit buildProgress(40);

        // Extract registration data from project
        QList<RegistrationData> registrations = extractRegistrations(project);

        emit buildProgress(60);

        // Add edges for each registration
        int processedRegistrations = 0;
        for (const auto& reg : registrations)
        {
            int sourceNode = graph->findNodeByScanId(reg.sourceScanId);
            int targetNode = graph->findNodeByScanId(reg.targetScanId);

            if (sourceNode >= 0 && targetNode >= 0)
            {
                graph->addEdge(sourceNode, targetNode, reg.transform, reg.rmsError);
            }
            else
            {
                qWarning() << "Could not find nodes for registration:" << reg.sourceScanId << "->" << reg.targetScanId;
            }

            processedRegistrations++;
            int progress = 60 + (processedRegistrations * 30) / registrations.size();
            emit buildProgress(progress);
        }

        emit buildProgress(100);

        qDebug() << "Built pose graph with" << graph->nodeCount() << "nodes and" << graph->edgeCount() << "edges";

        emit buildCompleted(true);
    }
    catch (const std::exception& e)
    {
        qCritical() << "Error building pose graph:" << e.what();
        emit buildCompleted(false);
    }

    return graph;
}

std::unique_ptr<PoseGraph> PoseGraphBuilder::buildFromScans(const QStringList& scanIds)
{
    auto graph = std::make_unique<PoseGraph>();

    for (const QString& scanId : scanIds)
    {
        graph->addNode(scanId, QMatrix4x4());  // Identity transform
    }

    qDebug() << "Built basic pose graph with" << scanIds.size() << "scans";
    return graph;
}

bool PoseGraphBuilder::addRegistrationEdge(PoseGraph& graph,
                                           const QString& sourceScanId,
                                           const QString& targetScanId,
                                           const QMatrix4x4& transform,
                                           float rmsError)
{
    int sourceNode = graph.findNodeByScanId(sourceScanId);
    int targetNode = graph.findNodeByScanId(targetScanId);

    if (sourceNode < 0)
    {
        sourceNode = graph.addNode(sourceScanId);
    }

    if (targetNode < 0)
    {
        targetNode = graph.addNode(targetScanId);
    }

    return graph.addEdge(sourceNode, targetNode, transform, rmsError);
}

PoseGraphBuilder::ValidationResult PoseGraphBuilder::validateGraph(const PoseGraph& graph) const
{
    ValidationResult result;

    if (graph.isEmpty())
    {
        result.errorMessage = "Pose graph is empty";
        return result;
    }

    if (!graph.isValid())
    {
        result.errorMessage = "Pose graph has invalid edge references";
        return result;
    }

    // Find connected components
    QList<QList<int>> components = findConnectedComponents(graph);
    result.connectedComponents = components.size();

    // Check for isolated scans (components with only one node)
    for (const auto& component : components)
    {
        if (component.size() == 1)
        {
            const PoseNode* node = graph.getNode(component.first());
            if (node)
            {
                result.isolatedScans.append(node->scanId);
            }
        }
    }

    // Check for loop closures
    result.hasLoops = graph.hasLoopClosures();

    // Graph is valid if it has proper connectivity
    result.isValid =
        (result.connectedComponents <= 1 || (result.connectedComponents == result.isolatedScans.size() + 1));

    if (!result.isValid && result.connectedComponents > 1)
    {
        result.errorMessage = QString("Graph has %1 disconnected components").arg(result.connectedComponents);
    }

    qDebug() << "Graph validation:" << result.isValid << "Components:" << result.connectedComponents
             << "Loops:" << result.hasLoops << "Isolated:" << result.isolatedScans.size();

    emit const_cast<PoseGraphBuilder*>(this)->validationCompleted(result);
    return result;
}

QList<PoseGraphBuilder::RegistrationData> PoseGraphBuilder::extractRegistrations(const RegistrationProject& project) const
{
    QList<RegistrationData> registrations;

    // Extract registration results from the project
    QList<RegistrationProject::RegistrationResult> results = project.getRegistrationResults();

    for (const auto& result : results)
    {
        if (result.isValid)
        {
            RegistrationData regData;
            regData.sourceScanId = result.sourceScanId;
            regData.targetScanId = result.targetScanId;
            regData.transform = result.transformation;
            regData.rmsError = result.rmsError;

            registrations.append(regData);
        }
    }

    qDebug() << "Extracted" << registrations.size() << "valid registration results from project";

    return registrations;
}

QList<QList<int>> PoseGraphBuilder::findConnectedComponents(const PoseGraph& graph) const
{
    QList<QList<int>> components;
    QSet<int> visited;

    for (const auto& node : graph.nodes())
    {
        if (!visited.contains(node.nodeIndex))
        {
            QList<int> component;
            dfsVisit(graph, node.nodeIndex, visited, component);
            if (!component.isEmpty())
            {
                components.append(component);
            }
        }
    }

    return components;
}

void PoseGraphBuilder::dfsVisit(const PoseGraph& graph, int startNode, QSet<int>& visited, QList<int>& component) const
{
    visited.insert(startNode);
    component.append(startNode);

    // Visit all connected nodes
    QList<PoseEdge> outgoingEdges = graph.getEdgesFromNode(startNode);
    QList<PoseEdge> incomingEdges = graph.getEdgesToNode(startNode);

    for (const auto& edge : outgoingEdges)
    {
        if (!visited.contains(edge.toNodeIndex))
        {
            dfsVisit(graph, edge.toNodeIndex, visited, component);
        }
    }

    for (const auto& edge : incomingEdges)
    {
        if (!visited.contains(edge.fromNodeIndex))
        {
            dfsVisit(graph, edge.fromNodeIndex, visited, component);
        }
    }
}

}  // namespace Registration
