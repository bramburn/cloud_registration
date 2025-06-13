/**
 * @file pose_graph_example.cpp
 * @brief Example demonstrating Sprint 7.1 pose graph construction and visualization
 * 
 * This example shows how to:
 * 1. Create a registration project with multiple scans
 * 2. Add registration results between scans
 * 3. Build a pose graph from the registration data
 * 4. Display the pose graph in a viewer widget
 * 5. Integrate everything into a main application window
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

#include "registration/RegistrationProject.h"
#include "registration/PoseGraphBuilder.h"
#include "registration/PoseGraph.h"
#include "ui/PoseGraphViewerWidget.h"
#include "ui/RegistrationTabWidget.h"
#include "app/MainPresenter.h"
#include "core/ScanInfo.h"

class PoseGraphExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PoseGraphExampleWindow(QWidget* parent = nullptr)
        : QMainWindow(parent),
          m_project(nullptr),
          m_poseGraphBuilder(nullptr),
          m_registrationTabWidget(nullptr),
          m_presenter(nullptr)
    {
        setupUI();
        createExampleProject();
    }

private slots:
    void onCreateSimpleChain()
    {
        clearProject();
        createSimpleChainExample();
        rebuildAndDisplay();
    }

    void onCreateLoopClosure()
    {
        clearProject();
        createLoopClosureExample();
        rebuildAndDisplay();
    }

    void onCreateDisconnectedComponents()
    {
        clearProject();
        createDisconnectedComponentsExample();
        rebuildAndDisplay();
    }

    void onClearGraph()
    {
        clearProject();
        m_registrationTabWidget->getPoseGraphViewer()->clearGraph();
        updateStatus("Graph cleared");
    }

    void onNodeSelected(const QString& scanId)
    {
        updateStatus(QString("Selected node: %1").arg(scanId));
    }

    void onEdgeSelected(const QString& sourceScanId, const QString& targetScanId)
    {
        updateStatus(QString("Selected edge: %1 → %2").arg(sourceScanId, targetScanId));
    }

private:
    void setupUI()
    {
        setWindowTitle("Pose Graph Example - Sprint 7.1");
        setMinimumSize(1000, 700);

        // Central widget
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

        // Control panel
        createControlPanel(mainLayout);

        // Registration tab widget (includes pose graph viewer)
        m_registrationTabWidget = new RegistrationTabWidget(this);
        mainLayout->addWidget(m_registrationTabWidget, 1);

        // Status bar
        statusBar()->showMessage("Ready");

        // Connect signals
        connect(m_registrationTabWidget->getPoseGraphViewer(), &PoseGraphViewerWidget::nodeSelected,
                this, &PoseGraphExampleWindow::onNodeSelected);
        connect(m_registrationTabWidget->getPoseGraphViewer(), &PoseGraphViewerWidget::edgeSelected,
                this, &PoseGraphExampleWindow::onEdgeSelected);
    }

    void createControlPanel(QHBoxLayout* mainLayout)
    {
        QWidget* controlPanel = new QWidget();
        controlPanel->setMaximumWidth(200);
        controlPanel->setMinimumWidth(200);

        QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

        // Title
        QLabel* title = new QLabel("Pose Graph Examples");
        title->setStyleSheet("font-weight: bold; font-size: 14px; margin-bottom: 10px;");
        controlLayout->addWidget(title);

        // Example buttons
        QPushButton* simpleChainBtn = new QPushButton("Simple Chain\n(A→B→C)");
        QPushButton* loopClosureBtn = new QPushButton("Loop Closure\n(A→B→C→A)");
        QPushButton* disconnectedBtn = new QPushButton("Disconnected\n(A→B, C→D)");
        QPushButton* clearBtn = new QPushButton("Clear Graph");

        controlLayout->addWidget(simpleChainBtn);
        controlLayout->addWidget(loopClosureBtn);
        controlLayout->addWidget(disconnectedBtn);
        controlLayout->addWidget(clearBtn);
        controlLayout->addStretch();

        // Connect buttons
        connect(simpleChainBtn, &QPushButton::clicked, this, &PoseGraphExampleWindow::onCreateSimpleChain);
        connect(loopClosureBtn, &QPushButton::clicked, this, &PoseGraphExampleWindow::onCreateLoopClosure);
        connect(disconnectedBtn, &QPushButton::clicked, this, &PoseGraphExampleWindow::onCreateDisconnectedComponents);
        connect(clearBtn, &QPushButton::clicked, this, &PoseGraphExampleWindow::onClearGraph);

        mainLayout->addWidget(controlPanel);
    }

    void createExampleProject()
    {
        m_project = std::make_unique<Registration::RegistrationProject>("Example Project", "/tmp/example");
        m_poseGraphBuilder = std::make_unique<Registration::PoseGraphBuilder>();
        
        // Set project in the tab widget
        m_registrationTabWidget->setRegistrationProject(m_project.get());
        
        // Create a simple presenter to handle pose graph updates
        // Note: In a real application, this would be properly integrated
        m_presenter = std::make_unique<MainPresenter>(nullptr, nullptr, nullptr, nullptr, nullptr);
        m_presenter->setRegistrationProject(m_project.get());
        m_presenter->setPoseGraphViewer(m_registrationTabWidget->getPoseGraphViewer());
    }

    void clearProject()
    {
        if (m_project) {
            // Clear existing registration results
            auto results = m_project->getRegistrationResults();
            for (const auto& result : results) {
                m_project->removeRegistrationResult(result.sourceScanId, result.targetScanId);
            }
            
            // Clear scans
            auto scanIds = m_project->getScanIds();
            for (const QString& scanId : scanIds) {
                m_project->removeScan(scanId);
            }
        }
    }

    void addScan(const QString& scanId)
    {
        ScanInfo scan;
        scan.scanId = scanId;
        scan.filePath = QString("/tmp/%1.e57").arg(scanId);
        scan.transform.setToIdentity();
        m_project->addScan(scan);
    }

    void addRegistration(const QString& source, const QString& target, float rmsError = 0.01f)
    {
        Registration::RegistrationProject::RegistrationResult result;
        result.sourceScanId = source;
        result.targetScanId = target;
        result.transformation.setToIdentity();
        result.rmsError = rmsError;
        result.correspondenceCount = 100;
        result.isValid = true;
        result.algorithm = "Example";
        
        m_project->addRegistrationResult(result);
    }

    void createSimpleChainExample()
    {
        addScan("ScanA");
        addScan("ScanB");
        addScan("ScanC");
        
        addRegistration("ScanA", "ScanB", 0.01f);
        addRegistration("ScanB", "ScanC", 0.02f);
        
        updateStatus("Created simple chain: A → B → C");
    }

    void createLoopClosureExample()
    {
        addScan("ScanA");
        addScan("ScanB");
        addScan("ScanC");
        
        addRegistration("ScanA", "ScanB", 0.01f);
        addRegistration("ScanB", "ScanC", 0.02f);
        addRegistration("ScanC", "ScanA", 0.015f);  // Loop closure
        
        updateStatus("Created loop closure: A → B → C → A");
    }

    void createDisconnectedComponentsExample()
    {
        addScan("ScanA");
        addScan("ScanB");
        addScan("ScanC");
        addScan("ScanD");
        
        addRegistration("ScanA", "ScanB", 0.01f);
        addRegistration("ScanC", "ScanD", 0.02f);
        
        updateStatus("Created disconnected components: A → B, C → D");
    }

    void rebuildAndDisplay()
    {
        if (!m_project || !m_poseGraphBuilder) {
            return;
        }

        try {
            // Build pose graph
            auto graph = m_poseGraphBuilder->build(*m_project);
            
            // Display in viewer
            m_registrationTabWidget->getPoseGraphViewer()->displayGraph(*graph);
            
            // Switch to pose graph tab
            m_registrationTabWidget->showPoseGraphTab();
            
            updateStatus(QString("Graph built: %1 nodes, %2 edges")
                        .arg(graph->nodeCount())
                        .arg(graph->edgeCount()));
                        
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString("Failed to build pose graph: %1").arg(e.what()));
        }
    }

    void updateStatus(const QString& message)
    {
        statusBar()->showMessage(message);
        qDebug() << "PoseGraphExample:" << message;
    }

    // Member variables
    std::unique_ptr<Registration::RegistrationProject> m_project;
    std::unique_ptr<Registration::PoseGraphBuilder> m_poseGraphBuilder;
    RegistrationTabWidget* m_registrationTabWidget;
    std::unique_ptr<MainPresenter> m_presenter;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PoseGraphExampleWindow window;
    window.show();

    return app.exec();
}

#include "pose_graph_example.moc"
