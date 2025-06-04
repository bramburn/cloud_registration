/**
 * Sprint R1 LOD System Demonstration
 * 
 * This demo showcases the octree-based Level of Detail system
 * implemented in Sprint R1. It demonstrates:
 * 
 * 1. Octree construction from point cloud data
 * 2. View-frustum culling capabilities
 * 3. Distance-based LOD performance
 * 4. Performance monitoring and metrics
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QTimer>
#include <QDebug>
#include <random>
#include <chrono>

#include "../../src/pointcloudviewerwidget.h"
#include "../../src/octree.h"

class LODDemoWindow : public QMainWindow {
    Q_OBJECT

public:
    LODDemoWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupConnections();
        generateTestData();
        updateMetrics();
        
        // Start metrics update timer
        m_metricsTimer = new QTimer(this);
        connect(m_metricsTimer, &QTimer::timeout, this, &LODDemoWindow::updateMetrics);
        m_metricsTimer->start(1000); // Update every second
    }

private slots:
    void toggleLOD(bool enabled) {
        m_viewer->setLODEnabled(enabled);
        qDebug() << "LOD system" << (enabled ? "enabled" : "disabled");
    }
    
    void updateLODDistance1(int value) {
        float distance1 = static_cast<float>(value);
        float distance2;
        m_viewer->getLODDistances(distance2, distance2); // Get current distance2
        m_viewer->setLODDistances(distance1, distance2);
        m_distance1Label->setText(QString("Close Distance: %1").arg(distance1));
    }
    
    void updateLODDistance2(int value) {
        float distance1, distance2 = static_cast<float>(value);
        m_viewer->getLODDistances(distance1, distance2); // Get current distance1
        m_viewer->setLODDistances(distance1, distance2);
        m_distance2Label->setText(QString("Far Distance: %1").arg(distance2));
    }
    
    void generateSmallDataset() {
        generatePointCloud(1000, 10.0f);
    }
    
    void generateMediumDataset() {
        generatePointCloud(50000, 50.0f);
    }
    
    void generateLargeDataset() {
        generatePointCloud(200000, 100.0f);
    }
    
    void updateMetrics() {
        if (!m_viewer) return;
        
        float fps = m_viewer->getCurrentFPS();
        size_t visiblePoints = m_viewer->getVisiblePointCount();
        size_t totalPoints = m_viewer->getPointCount();
        size_t octreeNodes = m_viewer->getOctreeNodeCount();
        
        m_fpsLabel->setText(QString("FPS: %1").arg(QString::number(fps, 'f', 1)));
        m_visiblePointsLabel->setText(QString("Visible Points: %1").arg(visiblePoints));
        m_totalPointsLabel->setText(QString("Total Points: %1").arg(totalPoints));
        m_octreeNodesLabel->setText(QString("Octree Nodes: %1").arg(octreeNodes));
        
        // Calculate culling efficiency
        if (totalPoints > 0) {
            float efficiency = 100.0f * (1.0f - static_cast<float>(visiblePoints) / static_cast<float>(totalPoints));
            m_cullingEfficiencyLabel->setText(QString("Culling Efficiency: %1%").arg(QString::number(efficiency, 'f', 1)));
        }
    }

private:
    void setupUI() {
        auto* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        auto* mainLayout = new QHBoxLayout(centralWidget);
        
        // Create viewer widget
        m_viewer = new PointCloudViewerWidget(this);
        mainLayout->addWidget(m_viewer, 3); // 3/4 of the space
        
        // Create control panel
        auto* controlPanel = new QWidget(this);
        controlPanel->setMaximumWidth(300);
        controlPanel->setMinimumWidth(250);
        mainLayout->addWidget(controlPanel, 1); // 1/4 of the space
        
        auto* controlLayout = new QVBoxLayout(controlPanel);
        
        // LOD Controls
        controlLayout->addWidget(new QLabel("<b>LOD System Controls</b>"));
        
        m_lodCheckBox = new QCheckBox("Enable LOD System");
        m_lodCheckBox->setChecked(false);
        controlLayout->addWidget(m_lodCheckBox);
        
        // Distance controls
        controlLayout->addWidget(new QLabel("LOD Distances:"));
        
        m_distance1Label = new QLabel("Close Distance: 50");
        controlLayout->addWidget(m_distance1Label);
        m_distance1Slider = new QSlider(Qt::Horizontal);
        m_distance1Slider->setRange(10, 200);
        m_distance1Slider->setValue(50);
        controlLayout->addWidget(m_distance1Slider);
        
        m_distance2Label = new QLabel("Far Distance: 200");
        controlLayout->addWidget(m_distance2Label);
        m_distance2Slider = new QSlider(Qt::Horizontal);
        m_distance2Slider->setRange(50, 500);
        m_distance2Slider->setValue(200);
        controlLayout->addWidget(m_distance2Slider);
        
        // Dataset generation
        controlLayout->addWidget(new QLabel("<b>Test Datasets</b>"));
        
        auto* smallDataBtn = new QPushButton("Small (1K points)");
        controlLayout->addWidget(smallDataBtn);
        connect(smallDataBtn, &QPushButton::clicked, this, &LODDemoWindow::generateSmallDataset);
        
        auto* mediumDataBtn = new QPushButton("Medium (50K points)");
        controlLayout->addWidget(mediumDataBtn);
        connect(mediumDataBtn, &QPushButton::clicked, this, &LODDemoWindow::generateMediumDataset);
        
        auto* largeDataBtn = new QPushButton("Large (200K points)");
        controlLayout->addWidget(largeDataBtn);
        connect(largeDataBtn, &QPushButton::clicked, this, &LODDemoWindow::generateLargeDataset);
        
        // Performance metrics
        controlLayout->addWidget(new QLabel("<b>Performance Metrics</b>"));
        
        m_fpsLabel = new QLabel("FPS: 0.0");
        controlLayout->addWidget(m_fpsLabel);
        
        m_visiblePointsLabel = new QLabel("Visible Points: 0");
        controlLayout->addWidget(m_visiblePointsLabel);
        
        m_totalPointsLabel = new QLabel("Total Points: 0");
        controlLayout->addWidget(m_totalPointsLabel);
        
        m_octreeNodesLabel = new QLabel("Octree Nodes: 0");
        controlLayout->addWidget(m_octreeNodesLabel);
        
        m_cullingEfficiencyLabel = new QLabel("Culling Efficiency: 0%");
        controlLayout->addWidget(m_cullingEfficiencyLabel);
        
        // Instructions
        controlLayout->addWidget(new QLabel("<b>Instructions</b>"));
        auto* instructionsLabel = new QLabel(
            "1. Generate a test dataset\n"
            "2. Enable LOD system\n"
            "3. Use mouse to navigate:\n"
            "   - Left: Orbit camera\n"
            "   - Right: Pan camera\n"
            "   - Wheel: Zoom\n"
            "4. Adjust LOD distances\n"
            "5. Monitor performance"
        );
        instructionsLabel->setWordWrap(true);
        instructionsLabel->setStyleSheet("QLabel { font-size: 10px; }");
        controlLayout->addWidget(instructionsLabel);
        
        controlLayout->addStretch();
        
        setWindowTitle("Sprint R1 LOD System Demo");
        resize(1200, 800);
    }
    
    void setupConnections() {
        connect(m_lodCheckBox, &QCheckBox::toggled, this, &LODDemoWindow::toggleLOD);
        connect(m_distance1Slider, &QSlider::valueChanged, this, &LODDemoWindow::updateLODDistance1);
        connect(m_distance2Slider, &QSlider::valueChanged, this, &LODDemoWindow::updateLODDistance2);
    }
    
    void generateTestData() {
        // Generate initial small dataset
        generatePointCloud(1000, 10.0f);
    }
    
    void generatePointCloud(int numPoints, float spread) {
        qDebug() << "Generating" << numPoints << "points with spread" << spread;
        
        std::vector<float> points;
        points.reserve(numPoints * 3);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-spread, spread);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numPoints; i++) {
            points.push_back(dis(gen));
            points.push_back(dis(gen));
            points.push_back(dis(gen));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        qDebug() << "Point generation took" << duration.count() << "ms";
        
        // Load into viewer
        m_viewer->setState(PointCloudViewerWidget::ViewerState::Loading, "Loading point cloud...");
        m_viewer->loadPointCloud(points);
        m_viewer->setState(PointCloudViewerWidget::ViewerState::DisplayingData, "Point cloud loaded");
        
        qDebug() << "Point cloud loaded successfully";
    }

private:
    PointCloudViewerWidget* m_viewer;
    QTimer* m_metricsTimer;
    
    // Controls
    QCheckBox* m_lodCheckBox;
    QSlider* m_distance1Slider;
    QSlider* m_distance2Slider;
    
    // Labels
    QLabel* m_distance1Label;
    QLabel* m_distance2Label;
    QLabel* m_fpsLabel;
    QLabel* m_visiblePointsLabel;
    QLabel* m_totalPointsLabel;
    QLabel* m_octreeNodesLabel;
    QLabel* m_cullingEfficiencyLabel;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    qDebug() << "Starting Sprint R1 LOD System Demo";
    
    LODDemoWindow window;
    window.show();
    
    return app.exec();
}

#include "test_sprint_r1_lod_demo.moc"
