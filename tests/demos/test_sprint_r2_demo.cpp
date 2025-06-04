#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QDebug>
#include <vector>
#include <random>

#include "../../src/pointcloudviewerwidget.h"

class SprintR2DemoWindow : public QMainWindow {
    Q_OBJECT

public:
    SprintR2DemoWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUI();
        generateTestData();
        connectSignals();
    }

private slots:
    void onLODEnabledChanged(bool enabled) {
        m_lodQualitySlider->setEnabled(enabled);
        m_primaryThresholdSpinBox->setEnabled(enabled);
        m_cullThresholdSpinBox->setEnabled(enabled);
        m_viewer->setLODEnabled(enabled);
    }

    void onLODQualityChanged(int value) {
        // Convert quality slider (1-100) to threshold (100-1 pixels)
        float threshold = 101.0f - value;
        m_primaryThresholdSpinBox->setValue(static_cast<int>(threshold));
    }

    void onPrimaryThresholdChanged(int value) {
        // Update quality slider to reflect threshold change
        int qualityValue = 101 - value;
        m_lodQualitySlider->setValue(qualityValue);
        m_viewer->setPrimaryScreenSpaceErrorThreshold(static_cast<float>(value));
    }

    void onCullThresholdChanged(int value) {
        m_viewer->setCullScreenSpaceErrorThreshold(static_cast<float>(value));
    }

private:
    void setupUI() {
        auto *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        auto *mainLayout = new QHBoxLayout(centralWidget);

        // Create point cloud viewer
        m_viewer = new PointCloudViewerWidget(this);
        m_viewer->setMinimumSize(800, 600);
        mainLayout->addWidget(m_viewer, 1);

        // Create control panel
        auto *controlPanel = new QWidget();
        controlPanel->setMaximumWidth(300);
        controlPanel->setMinimumWidth(250);
        mainLayout->addWidget(controlPanel);

        auto *controlLayout = new QVBoxLayout(controlPanel);

        // LOD Controls Group
        auto *lodGroupBox = new QGroupBox("Sprint R2: Screen-Space Error LOD Controls", this);
        auto *lodLayout = new QVBoxLayout(lodGroupBox);

        // Enable LOD checkbox
        m_lodEnableCheckbox = new QCheckBox("Enable Level of Detail (LOD)", this);
        m_lodEnableCheckbox->setChecked(true);
        lodLayout->addWidget(m_lodEnableCheckbox);

        // LOD Quality Slider (inverse of primary threshold)
        auto *qualityLabel = new QLabel("LOD Quality (Higher = More Detail):", this);
        m_lodQualitySlider = new QSlider(Qt::Horizontal, this);
        m_lodQualitySlider->setRange(1, 100);
        m_lodQualitySlider->setValue(50); // Default to medium quality
        m_lodQualitySlider->setTickPosition(QSlider::TicksBelow);
        m_lodQualitySlider->setTickInterval(10);

        auto *qualityLayout = new QHBoxLayout();
        qualityLayout->addWidget(qualityLabel);
        qualityLayout->addWidget(m_lodQualitySlider);
        lodLayout->addLayout(qualityLayout);

        // Primary threshold control
        auto *primaryLabel = new QLabel("Primary Threshold (pixels):", this);
        m_primaryThresholdSpinBox = new QSpinBox(this);
        m_primaryThresholdSpinBox->setRange(1, 200);
        m_primaryThresholdSpinBox->setValue(50);

        auto *primaryLayout = new QHBoxLayout();
        primaryLayout->addWidget(primaryLabel);
        primaryLayout->addWidget(m_primaryThresholdSpinBox);
        lodLayout->addLayout(primaryLayout);

        // Cull threshold control
        auto *cullLabel = new QLabel("Cull Threshold (pixels):", this);
        m_cullThresholdSpinBox = new QSpinBox(this);
        m_cullThresholdSpinBox->setRange(1, 10);
        m_cullThresholdSpinBox->setValue(2);

        auto *cullLayout = new QHBoxLayout();
        cullLayout->addWidget(cullLabel);
        cullLayout->addWidget(m_cullThresholdSpinBox);
        lodLayout->addLayout(cullLayout);

        // Statistics display
        m_lodStatsLabel = new QLabel("LOD Statistics will appear here", this);
        m_lodStatsLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; }");
        m_lodStatsLabel->setWordWrap(true);
        lodLayout->addWidget(m_lodStatsLabel);

        controlLayout->addWidget(lodGroupBox);

        // Instructions
        auto *instructionsLabel = new QLabel(
            "Instructions:\n"
            "• Use mouse to rotate view\n"
            "• Mouse wheel to zoom\n"
            "• Adjust LOD settings to see performance impact\n"
            "• Higher quality = more detail, lower FPS\n"
            "• Lower quality = less detail, higher FPS\n"
            "• Watch debug output for statistics",
            this
        );
        instructionsLabel->setWordWrap(true);
        instructionsLabel->setStyleSheet("QLabel { background-color: #e0e0e0; padding: 10px; }");
        controlLayout->addWidget(instructionsLabel);

        controlLayout->addStretch();

        setWindowTitle("Sprint R2: Screen-Space Error LOD Demo");
        resize(1200, 800);
    }

    void generateTestData() {
        // Generate a large test point cloud with varying density
        std::vector<float> testPoints;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-100.0f, 100.0f);

        // Create multiple clusters of points at different distances
        for (int cluster = 0; cluster < 5; ++cluster) {
            float clusterX = dis(gen);
            float clusterY = dis(gen);
            float clusterZ = dis(gen);

            // Dense cluster
            for (int i = 0; i < 2000; ++i) {
                std::uniform_real_distribution<float> localDis(-10.0f, 10.0f);
                testPoints.push_back(clusterX + localDis(gen));
                testPoints.push_back(clusterY + localDis(gen));
                testPoints.push_back(clusterZ + localDis(gen));
            }
        }

        // Add some scattered points
        for (int i = 0; i < 5000; ++i) {
            testPoints.push_back(dis(gen));
            testPoints.push_back(dis(gen));
            testPoints.push_back(dis(gen));
        }

        qDebug() << "Generated" << (testPoints.size() / 3) << "test points";
        m_viewer->loadPointCloud(testPoints);
    }

    void connectSignals() {
        connect(m_lodEnableCheckbox, &QCheckBox::toggled,
                this, &SprintR2DemoWindow::onLODEnabledChanged);
        connect(m_lodQualitySlider, &QSlider::valueChanged,
                this, &SprintR2DemoWindow::onLODQualityChanged);
        connect(m_primaryThresholdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &SprintR2DemoWindow::onPrimaryThresholdChanged);
        connect(m_cullThresholdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &SprintR2DemoWindow::onCullThresholdChanged);

        // Initialize with default values
        onLODEnabledChanged(true);
        onPrimaryThresholdChanged(50);
        onCullThresholdChanged(2);
    }

    PointCloudViewerWidget *m_viewer;
    QCheckBox *m_lodEnableCheckbox;
    QSlider *m_lodQualitySlider;
    QSpinBox *m_primaryThresholdSpinBox;
    QSpinBox *m_cullThresholdSpinBox;
    QLabel *m_lodStatsLabel;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    qDebug() << "Starting Sprint R2 Screen-Space Error LOD Demo";

    SprintR2DemoWindow window;
    window.show();

    qDebug() << "Demo window shown. Use the controls to test Sprint R2 functionality.";

    return app.exec();
}

#include "test_sprint_r2_demo.moc"
