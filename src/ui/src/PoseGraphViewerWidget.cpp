#include "ui/PoseGraphViewerWidget.h"
#include "registration/PoseGraph.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QResizeEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

PoseGraphViewerWidget::PoseGraphViewerWidget(QWidget* parent)
    : QWidget(parent),
      m_mainLayout(nullptr),
      m_toolbarLayout(nullptr),
      m_graphicsView(nullptr),
      m_graphicsScene(nullptr),
      m_zoomInButton(nullptr),
      m_zoomOutButton(nullptr),
      m_fitToViewButton(nullptr),
      m_resetViewButton(nullptr),
      m_exportButton(nullptr),
      m_showLabelsCheckBox(nullptr),
      m_showWeightsCheckBox(nullptr),
      m_statusLabel(nullptr),
      m_showNodeLabels(true),
      m_showEdgeWeights(false),
      m_currentZoom(1.0)
{
    setupUI();
}

void PoseGraphViewerWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);

    createToolbar();
    createGraphicsView();
    setupConnections();

    setLayout(m_mainLayout);
}

void PoseGraphViewerWidget::createToolbar()
{
    m_toolbarLayout = new QHBoxLayout();
    m_toolbarLayout->setSpacing(5);

    // Zoom controls
    m_zoomInButton = new QPushButton("Zoom In");
    m_zoomOutButton = new QPushButton("Zoom Out");
    m_fitToViewButton = new QPushButton("Fit to View");
    m_resetViewButton = new QPushButton("Reset View");

    // Export button
    m_exportButton = new QPushButton("Export");

    // Display options
    m_showLabelsCheckBox = new QCheckBox("Show Labels");
    m_showLabelsCheckBox->setChecked(m_showNodeLabels);
    
    m_showWeightsCheckBox = new QCheckBox("Show Weights");
    m_showWeightsCheckBox->setChecked(m_showEdgeWeights);

    // Status label
    m_statusLabel = new QLabel("No graph loaded");

    // Add to layout
    m_toolbarLayout->addWidget(m_zoomInButton);
    m_toolbarLayout->addWidget(m_zoomOutButton);
    m_toolbarLayout->addWidget(m_fitToViewButton);
    m_toolbarLayout->addWidget(m_resetViewButton);
    m_toolbarLayout->addSeparator();
    m_toolbarLayout->addWidget(m_exportButton);
    m_toolbarLayout->addSeparator();
    m_toolbarLayout->addWidget(m_showLabelsCheckBox);
    m_toolbarLayout->addWidget(m_showWeightsCheckBox);
    m_toolbarLayout->addStretch();
    m_toolbarLayout->addWidget(m_statusLabel);

    m_mainLayout->addLayout(m_toolbarLayout);
}

void PoseGraphViewerWidget::createGraphicsView()
{
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new QGraphicsView(m_graphicsScene, this);
    
    // Configure view
    m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_mainLayout->addWidget(m_graphicsView);
}

void PoseGraphViewerWidget::setupConnections()
{
    // Button connections
    connect(m_zoomInButton, &QPushButton::clicked, this, &PoseGraphViewerWidget::zoomIn);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &PoseGraphViewerWidget::zoomOut);
    connect(m_fitToViewButton, &QPushButton::clicked, this, &PoseGraphViewerWidget::onFitToView);
    connect(m_resetViewButton, &QPushButton::clicked, this, &PoseGraphViewerWidget::onResetView);
    connect(m_exportButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Export Graph", "", "PNG Files (*.png)");
        if (!fileName.isEmpty()) {
            exportAsImage(fileName);
        }
    });

    // Checkbox connections
    connect(m_showLabelsCheckBox, &QCheckBox::toggled, this, &PoseGraphViewerWidget::onToggleNodeLabels);
    connect(m_showWeightsCheckBox, &QCheckBox::toggled, this, &PoseGraphViewerWidget::onToggleEdgeWeights);

    // Scene connections
    connect(m_graphicsScene, &QGraphicsScene::selectionChanged, this, &PoseGraphViewerWidget::onSceneSelectionChanged);
}

void PoseGraphViewerWidget::displayGraph(const Registration::PoseGraph& graph)
{
    clearGraph();

    if (graph.isEmpty()) {
        m_statusLabel->setText("Empty graph");
        return;
    }

    qDebug() << "Displaying pose graph with" << graph.nodeCount() << "nodes and" << graph.edgeCount() << "edges";

    // Calculate node positions
    calculateNodePositions(graph);

    // Render nodes and edges
    renderNodes(graph);
    renderEdges(graph);

    // Update scene rect and fit to view
    m_graphicsScene->setSceneRect(m_graphicsScene->itemsBoundingRect());
    fitToView();

    // Update status
    m_statusLabel->setText(QString("Nodes: %1, Edges: %2").arg(graph.nodeCount()).arg(graph.edgeCount()));

    emit viewUpdated();
}

void PoseGraphViewerWidget::clearGraph()
{
    m_graphicsScene->clear();
    m_nodePositions.clear();
    m_nodeItems.clear();
    m_labelItems.clear();
    m_edgeItems.clear();
    m_weightItems.clear();
    
    m_statusLabel->setText("No graph loaded");
}

void PoseGraphViewerWidget::calculateNodePositions(const Registration::PoseGraph& graph)
{
    m_nodePositions.clear();

    if (graph.nodeCount() == 0) {
        return;
    }

    // For now, use a simple circular layout
    applyCircularLayout(graph);
}

void PoseGraphViewerWidget::applyCircularLayout(const Registration::PoseGraph& graph)
{
    const QList<Registration::PoseNode>& nodes = graph.nodes();
    int nodeCount = nodes.size();
    
    if (nodeCount == 0) return;

    double radius = qMax(100.0, nodeCount * 30.0);
    double angleStep = 2.0 * M_PI / nodeCount;

    for (int i = 0; i < nodeCount; ++i) {
        double angle = i * angleStep;
        double x = radius * qCos(angle);
        double y = radius * qSin(angle);
        
        m_nodePositions[nodes[i].scanId] = QPointF(x, y);
    }
}

void PoseGraphViewerWidget::renderNodes(const Registration::PoseGraph& graph)
{
    const QList<Registration::PoseNode>& nodes = graph.nodes();
    
    for (const auto& node : nodes) {
        QPointF position = getNodePosition(node.scanId);
        
        // Create node circle
        QGraphicsEllipseItem* nodeItem = m_graphicsScene->addEllipse(
            position.x() - NODE_RADIUS, position.y() - NODE_RADIUS,
            2 * NODE_RADIUS, 2 * NODE_RADIUS,
            QPen(Qt::black, 2), QBrush(Qt::lightBlue));
        
        nodeItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        nodeItem->setData(0, node.scanId); // Store scan ID
        m_nodeItems[node.scanId] = nodeItem;

        // Create node label
        if (m_showNodeLabels) {
            QGraphicsTextItem* labelItem = m_graphicsScene->addText(node.scanId, QFont("Arial", 10));
            labelItem->setPos(position.x() - labelItem->boundingRect().width() / 2,
                             position.y() + NODE_RADIUS + 5);
            m_labelItems[node.scanId] = labelItem;
        }
    }
}

void PoseGraphViewerWidget::renderEdges(const Registration::PoseGraph& graph)
{
    const QList<Registration::PoseEdge>& edges = graph.edges();
    const QList<Registration::PoseNode>& nodes = graph.nodes();
    
    for (const auto& edge : edges) {
        // Find source and target nodes
        QString sourceScanId, targetScanId;
        for (const auto& node : nodes) {
            if (node.nodeIndex == edge.fromNodeIndex) {
                sourceScanId = node.scanId;
            }
            if (node.nodeIndex == edge.toNodeIndex) {
                targetScanId = node.scanId;
            }
        }

        if (sourceScanId.isEmpty() || targetScanId.isEmpty()) {
            continue;
        }

        QPointF sourcePos = getNodePosition(sourceScanId);
        QPointF targetPos = getNodePosition(targetScanId);

        // Create edge line
        QColor edgeColor = getEdgeColor(edge.rmsError);
        QGraphicsLineItem* edgeItem = m_graphicsScene->addLine(
            sourcePos.x(), sourcePos.y(), targetPos.x(), targetPos.y(),
            QPen(edgeColor, 2));
        
        edgeItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        edgeItem->setData(0, sourceScanId);
        edgeItem->setData(1, targetScanId);
        m_edgeItems.append(edgeItem);

        // Create weight label if enabled
        if (m_showEdgeWeights && edge.rmsError > 0.0f) {
            QPointF midPoint = (sourcePos + targetPos) / 2.0;
            QString weightText = formatRmsError(edge.rmsError);
            
            QGraphicsTextItem* weightItem = m_graphicsScene->addText(weightText, QFont("Arial", 8));
            weightItem->setPos(midPoint.x() - weightItem->boundingRect().width() / 2,
                              midPoint.y() - weightItem->boundingRect().height() / 2);
            weightItem->setDefaultTextColor(Qt::red);
            m_weightItems.append(weightItem);
        }
    }
}

QPointF PoseGraphViewerWidget::getNodePosition(const QString& scanId) const
{
    return m_nodePositions.value(scanId, QPointF(0, 0));
}

void PoseGraphViewerWidget::setNodePosition(const QString& scanId, const QPointF& position)
{
    m_nodePositions[scanId] = position;
}

QColor PoseGraphViewerWidget::getEdgeColor(float rmsError) const
{
    if (rmsError <= 0.0f) {
        return Qt::black;
    }
    
    // Color coding based on RMS error
    if (rmsError < 0.01f) {
        return Qt::green;
    } else if (rmsError < 0.05f) {
        return Qt::yellow;
    } else {
        return Qt::red;
    }
}

QString PoseGraphViewerWidget::formatRmsError(float rmsError) const
{
    return QString::number(rmsError, 'f', 3);
}

void PoseGraphViewerWidget::setShowNodeLabels(bool show)
{
    m_showNodeLabels = show;
    m_showLabelsCheckBox->setChecked(show);

    // Update visibility of existing labels
    for (auto* labelItem : m_labelItems) {
        labelItem->setVisible(show);
    }
}

void PoseGraphViewerWidget::setShowEdgeWeights(bool show)
{
    m_showEdgeWeights = show;
    m_showWeightsCheckBox->setChecked(show);

    // Update visibility of existing weight labels
    for (auto* weightItem : m_weightItems) {
        weightItem->setVisible(show);
    }
}

void PoseGraphViewerWidget::fitToView()
{
    if (m_graphicsScene->items().isEmpty()) {
        return;
    }

    m_graphicsView->fitInView(m_graphicsScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    m_currentZoom = m_graphicsView->transform().m11();
}

void PoseGraphViewerWidget::resetView()
{
    m_graphicsView->resetTransform();
    m_currentZoom = 1.0;
    fitToView();
}

bool PoseGraphViewerWidget::exportAsImage(const QString& filePath)
{
    if (m_graphicsScene->items().isEmpty()) {
        QMessageBox::warning(this, "Export Error", "No graph to export.");
        return false;
    }

    QRectF sceneRect = m_graphicsScene->itemsBoundingRect();
    QPixmap pixmap(sceneRect.size().toSize());
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_graphicsScene->render(&painter, QRectF(), sceneRect);
    painter.end();

    bool success = pixmap.save(filePath);
    if (success) {
        QMessageBox::information(this, "Export Success", "Graph exported successfully.");
    } else {
        QMessageBox::warning(this, "Export Error", "Failed to export graph.");
    }

    return success;
}

void PoseGraphViewerWidget::zoomIn()
{
    if (m_currentZoom < MAX_ZOOM) {
        m_graphicsView->scale(ZOOM_FACTOR, ZOOM_FACTOR);
        m_currentZoom *= ZOOM_FACTOR;
    }
}

void PoseGraphViewerWidget::zoomOut()
{
    if (m_currentZoom > MIN_ZOOM) {
        m_graphicsView->scale(1.0 / ZOOM_FACTOR, 1.0 / ZOOM_FACTOR);
        m_currentZoom /= ZOOM_FACTOR;
    }
}

void PoseGraphViewerWidget::onFitToView()
{
    fitToView();
}

void PoseGraphViewerWidget::onResetView()
{
    resetView();
}

void PoseGraphViewerWidget::onToggleNodeLabels(bool visible)
{
    setShowNodeLabels(visible);
}

void PoseGraphViewerWidget::onToggleEdgeWeights(bool visible)
{
    setShowEdgeWeights(visible);
}

void PoseGraphViewerWidget::onSceneSelectionChanged()
{
    QList<QGraphicsItem*> selectedItems = m_graphicsScene->selectedItems();

    for (QGraphicsItem* item : selectedItems) {
        // Check if it's a node
        if (auto* ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            QString scanId = ellipseItem->data(0).toString();
            if (!scanId.isEmpty()) {
                emit nodeSelected(scanId);
                return;
            }
        }

        // Check if it's an edge
        if (auto* lineItem = qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
            QString sourceScanId = lineItem->data(0).toString();
            QString targetScanId = lineItem->data(1).toString();
            if (!sourceScanId.isEmpty() && !targetScanId.isEmpty()) {
                emit edgeSelected(sourceScanId, targetScanId);
                return;
            }
        }
    }
}

void PoseGraphViewerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Maintain fit to view on resize if there's content
    if (!m_graphicsScene->items().isEmpty()) {
        fitToView();
    }
}

void PoseGraphViewerWidget::applyForceDirectedLayout(const Registration::PoseGraph& graph)
{
    // TODO: Implement force-directed layout algorithm
    // For now, fall back to circular layout
    applyCircularLayout(graph);
}

void PoseGraphViewerWidget::applyGridLayout(const Registration::PoseGraph& graph)
{
    const QList<Registration::PoseNode>& nodes = graph.nodes();
    int nodeCount = nodes.size();

    if (nodeCount == 0) return;

    int cols = qCeil(qSqrt(nodeCount));
    int rows = qCeil(static_cast<double>(nodeCount) / cols);

    double spacing = 100.0;
    double startX = -(cols - 1) * spacing / 2.0;
    double startY = -(rows - 1) * spacing / 2.0;

    for (int i = 0; i < nodeCount; ++i) {
        int row = i / cols;
        int col = i % cols;

        double x = startX + col * spacing;
        double y = startY + row * spacing;

        m_nodePositions[nodes[i].scanId] = QPointF(x, y);
    }
}
