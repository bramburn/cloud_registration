#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QMap>
#include <memory>

// Forward declarations
namespace Registration {
    class PoseGraph;
    struct PoseNode;
    struct PoseEdge;
}

/**
 * @brief Widget for visualizing pose graphs in 2D
 * 
 * This widget provides a graphical representation of scan poses and their
 * relationships using Qt's graphics framework. It displays nodes as circles
 * representing scans and edges as lines representing registrations.
 * 
 * Features:
 * - Interactive pan and zoom
 * - Node labeling with scan IDs
 * - Edge visualization with optional RMS error display
 * - Automatic layout algorithms
 * - Export capabilities
 */
class PoseGraphViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PoseGraphViewerWidget(QWidget* parent = nullptr);
    virtual ~PoseGraphViewerWidget() = default;

    /**
     * @brief Display a pose graph in the viewer
     * @param graph The pose graph to display
     */
    void displayGraph(const Registration::PoseGraph& graph);

    /**
     * @brief Clear the current graph display
     */
    void clearGraph();

    /**
     * @brief Set whether to show node labels
     * @param show True to show labels, false to hide
     */
    void setShowNodeLabels(bool show);

    /**
     * @brief Set whether to show edge weights (RMS errors)
     * @param show True to show weights, false to hide
     */
    void setShowEdgeWeights(bool show);

    /**
     * @brief Fit the graph to the view
     */
    void fitToView();

    /**
     * @brief Reset the view to default zoom and position
     */
    void resetView();

    /**
     * @brief Export the current graph view as an image
     * @param filePath Path to save the image
     * @return True if export was successful
     */
    bool exportAsImage(const QString& filePath);

public slots:
    /**
     * @brief Handle zoom in request
     */
    void zoomIn();

    /**
     * @brief Handle zoom out request
     */
    void zoomOut();

    /**
     * @brief Handle fit to view request
     */
    void onFitToView();

    /**
     * @brief Handle reset view request
     */
    void onResetView();

    /**
     * @brief Handle node label visibility toggle
     * @param visible True to show labels
     */
    void onToggleNodeLabels(bool visible);

    /**
     * @brief Handle edge weight visibility toggle
     * @param visible True to show weights
     */
    void onToggleEdgeWeights(bool visible);

signals:
    /**
     * @brief Emitted when a node is selected
     * @param scanId The ID of the selected scan
     */
    void nodeSelected(const QString& scanId);

    /**
     * @brief Emitted when an edge is selected
     * @param sourceScanId Source scan ID
     * @param targetScanId Target scan ID
     */
    void edgeSelected(const QString& sourceScanId, const QString& targetScanId);

    /**
     * @brief Emitted when the view is updated
     */
    void viewUpdated();

protected:
    /**
     * @brief Handle resize events
     * @param event The resize event
     */
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /**
     * @brief Handle scene selection changes
     */
    void onSceneSelectionChanged();

private:
    // UI setup
    void setupUI();
    void createToolbar();
    void createGraphicsView();
    void setupConnections();

    // Graph rendering
    void renderNodes(const Registration::PoseGraph& graph);
    void renderEdges(const Registration::PoseGraph& graph);
    void calculateNodePositions(const Registration::PoseGraph& graph);
    
    // Layout algorithms
    void applyCircularLayout(const Registration::PoseGraph& graph);
    void applyForceDirectedLayout(const Registration::PoseGraph& graph);
    void applyGridLayout(const Registration::PoseGraph& graph);

    // Utility methods
    QPointF getNodePosition(const QString& scanId) const;
    void setNodePosition(const QString& scanId, const QPointF& position);
    QColor getEdgeColor(float rmsError) const;
    QString formatRmsError(float rmsError) const;

    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_toolbarLayout;
    QGraphicsView* m_graphicsView;
    QGraphicsScene* m_graphicsScene;

    // Toolbar controls
    QPushButton* m_zoomInButton;
    QPushButton* m_zoomOutButton;
    QPushButton* m_fitToViewButton;
    QPushButton* m_resetViewButton;
    QPushButton* m_exportButton;
    QCheckBox* m_showLabelsCheckBox;
    QCheckBox* m_showWeightsCheckBox;
    QLabel* m_statusLabel;

    // Graph data
    QMap<QString, QPointF> m_nodePositions;
    QMap<QString, QGraphicsEllipseItem*> m_nodeItems;
    QMap<QString, QGraphicsTextItem*> m_labelItems;
    QList<QGraphicsLineItem*> m_edgeItems;
    QList<QGraphicsTextItem*> m_weightItems;

    // Display settings
    bool m_showNodeLabels;
    bool m_showEdgeWeights;
    double m_currentZoom;

    // Constants
    static constexpr double NODE_RADIUS = 20.0;
    static constexpr double ZOOM_FACTOR = 1.2;
    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 10.0;
};
