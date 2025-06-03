/**
 * Sprint 3.4 Demo: Advanced Memory Management & Registration Data Storage
 * 
 * This demo showcases the key features implemented in Sprint 3.4:
 * 1. Level of Detail (LOD) prototype with random subsampling
 * 2. Memory usage statistics and tracking
 * 3. SQLite schema extension for registration data
 */

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QTimer>
#include <QDebug>
#include <QTemporaryDir>
#include <memory>

#include "pointcloudloadmanager.h"
#include "pointcloudviewerwidget.h"
#include "sqlitemanager.h"

class Sprint34DemoWindow : public QMainWindow {
    Q_OBJECT

public:
    Sprint34DemoWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupConnections();
        initializeDemo();
    }

private slots:
    void onLODToggled(bool enabled) {
        m_viewer->toggleLOD(enabled);
        m_lodStatusLabel->setText(enabled ? "LOD: Enabled" : "LOD: Disabled");
        updateLog(QString("LOD %1").arg(enabled ? "enabled" : "disabled"));
    }
    
    void onLODRateChanged(int value) {
        float rate = value / 100.0f;
        m_viewer->setLODSubsampleRate(rate);
        m_lodRateLabel->setText(QString("Rate: %1%").arg(value));
        updateLog(QString("LOD rate changed to %1%").arg(value));
    }
    
    void onGenerateTestData() {
        int pointCount = m_pointCountSpinBox->value();
        generateTestPointCloud(pointCount);
        updateLog(QString("Generated test point cloud with %1 points").arg(pointCount));
    }
    
    void onMemoryUsageChanged(size_t totalBytes) {
        double megabytes = totalBytes / (1024.0 * 1024.0);
        QString text;
        
        if (megabytes >= 1024.0) {
            double gigabytes = megabytes / 1024.0;
            text = QString("%1 GB").arg(gigabytes, 0, 'f', 1);
        } else {
            text = QString("%1 MB").arg(megabytes, 0, 'f', 1);
        }
        
        m_memoryLabel->setText("Memory: " + text);
        
        // Update progress bar (assuming 2GB limit)
        int percentage = static_cast<int>((megabytes / 2048.0) * 100);
        m_memoryProgressBar->setValue(qMin(percentage, 100));
        
        updateLog(QString("Memory usage: %1").arg(text));
    }
    
    void onTestDatabaseSchema() {
        updateLog("Testing database schema extension...");
        
        // Test registration status table
        QSqlQuery query;
        if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='registration_status'")) {
            if (query.next()) {
                updateLog("✓ RegistrationStatus table found");
            } else {
                updateLog("✗ RegistrationStatus table not found");
            }
        }
        
        // Test transformation matrices table
        if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='transformation_matrices'")) {
            if (query.next()) {
                updateLog("✓ TransformationMatrices table found");
            } else {
                updateLog("✗ TransformationMatrices table not found");
            }
        }
    }

private:
    void setupUI() {
        auto *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        auto *mainLayout = new QHBoxLayout(centralWidget);
        
        // Left panel - Controls
        auto *controlPanel = new QWidget();
        controlPanel->setMaximumWidth(300);
        auto *controlLayout = new QVBoxLayout(controlPanel);
        
        // LOD Controls
        auto *lodGroup = new QGroupBox("Level of Detail (LOD)");
        auto *lodLayout = new QVBoxLayout(lodGroup);
        
        m_lodCheckBox = new QCheckBox("Enable LOD");
        lodLayout->addWidget(m_lodCheckBox);
        
        m_lodStatusLabel = new QLabel("LOD: Disabled");
        lodLayout->addWidget(m_lodStatusLabel);
        
        auto *rateLayout = new QHBoxLayout();
        rateLayout->addWidget(new QLabel("Subsample Rate:"));
        m_lodRateSlider = new QSlider(Qt::Horizontal);
        m_lodRateSlider->setRange(10, 100);
        m_lodRateSlider->setValue(50);
        rateLayout->addWidget(m_lodRateSlider);
        m_lodRateLabel = new QLabel("Rate: 50%");
        rateLayout->addWidget(m_lodRateLabel);
        lodLayout->addLayout(rateLayout);
        
        controlLayout->addWidget(lodGroup);
        
        // Memory Statistics
        auto *memoryGroup = new QGroupBox("Memory Statistics");
        auto *memoryLayout = new QVBoxLayout(memoryGroup);
        
        m_memoryLabel = new QLabel("Memory: 0 MB");
        memoryLayout->addWidget(m_memoryLabel);
        
        m_memoryProgressBar = new QProgressBar();
        m_memoryProgressBar->setRange(0, 100);
        memoryLayout->addWidget(m_memoryProgressBar);
        
        controlLayout->addWidget(memoryGroup);
        
        // Test Data Generation
        auto *testGroup = new QGroupBox("Test Data");
        auto *testLayout = new QVBoxLayout(testGroup);
        
        auto *pointLayout = new QHBoxLayout();
        pointLayout->addWidget(new QLabel("Point Count:"));
        m_pointCountSpinBox = new QSpinBox();
        m_pointCountSpinBox->setRange(1000, 1000000);
        m_pointCountSpinBox->setValue(10000);
        pointLayout->addWidget(m_pointCountSpinBox);
        testLayout->addLayout(pointLayout);
        
        m_generateButton = new QPushButton("Generate Test Data");
        testLayout->addWidget(m_generateButton);
        
        m_testSchemaButton = new QPushButton("Test Database Schema");
        testLayout->addWidget(m_testSchemaButton);
        
        controlLayout->addWidget(testGroup);
        
        // Log
        auto *logGroup = new QGroupBox("Activity Log");
        auto *logLayout = new QVBoxLayout(logGroup);
        
        m_logTextEdit = new QTextEdit();
        m_logTextEdit->setMaximumHeight(200);
        logLayout->addWidget(m_logTextEdit);
        
        controlLayout->addWidget(logGroup);
        
        controlLayout->addStretch();
        mainLayout->addWidget(controlPanel);
        
        // Right panel - 3D Viewer
        m_viewer = new PointCloudViewerWidget();
        mainLayout->addWidget(m_viewer, 1);
        
        setWindowTitle("Sprint 3.4 Demo - Advanced Memory Management & Registration Data");
        resize(1200, 800);
    }
    
    void setupConnections() {
        connect(m_lodCheckBox, &QCheckBox::toggled, this, &Sprint34DemoWindow::onLODToggled);
        connect(m_lodRateSlider, &QSlider::valueChanged, this, &Sprint34DemoWindow::onLODRateChanged);
        connect(m_generateButton, &QPushButton::clicked, this, &Sprint34DemoWindow::onGenerateTestData);
        connect(m_testSchemaButton, &QPushButton::clicked, this, &Sprint34DemoWindow::onTestDatabaseSchema);
    }
    
    void initializeDemo() {
        // Create temporary database for demo
        m_tempDir = std::make_unique<QTemporaryDir>();
        QString dbPath = m_tempDir->path() + "/demo.sqlite";
        
        m_sqliteManager = std::make_unique<SQLiteManager>();
        if (m_sqliteManager->createDatabase(dbPath)) {
            updateLog("✓ Demo database created with registration tables");
        } else {
            updateLog("✗ Failed to create demo database");
        }
        
        // Create load manager
        m_loadManager = std::make_unique<PointCloudLoadManager>();
        m_loadManager->setSQLiteManager(m_sqliteManager.get());
        
        // Connect memory usage signal
        connect(m_loadManager.get(), &PointCloudLoadManager::memoryUsageChanged,
                this, &Sprint34DemoWindow::onMemoryUsageChanged);
        
        updateLog("Sprint 3.4 Demo initialized");
        updateLog("Features: LOD prototype, Memory tracking, Registration schema");
    }
    
    void generateTestPointCloud(int pointCount) {
        std::vector<float> points;
        points.reserve(pointCount * 3);
        
        // Generate random point cloud in a cube
        for (int i = 0; i < pointCount; ++i) {
            points.push_back((rand() % 2000 - 1000) / 100.0f); // x: -10 to 10
            points.push_back((rand() % 2000 - 1000) / 100.0f); // y: -10 to 10
            points.push_back((rand() % 1000) / 100.0f);        // z: 0 to 10
        }
        
        m_viewer->loadPointCloud(points);
    }
    
    void updateLog(const QString &message) {
        QString timestamp = QTime::currentTime().toString("hh:mm:ss");
        m_logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
    }
    
    // UI Components
    QCheckBox *m_lodCheckBox;
    QSlider *m_lodRateSlider;
    QLabel *m_lodStatusLabel;
    QLabel *m_lodRateLabel;
    QLabel *m_memoryLabel;
    QProgressBar *m_memoryProgressBar;
    QSpinBox *m_pointCountSpinBox;
    QPushButton *m_generateButton;
    QPushButton *m_testSchemaButton;
    QTextEdit *m_logTextEdit;
    PointCloudViewerWidget *m_viewer;
    
    // Backend Components
    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<SQLiteManager> m_sqliteManager;
    std::unique_ptr<PointCloudLoadManager> m_loadManager;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    Sprint34DemoWindow window;
    window.show();
    
    return app.exec();
}

#include "test_sprint3_4_demo.moc"
