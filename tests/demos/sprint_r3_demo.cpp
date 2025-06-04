#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <vector>
#include <random>

#include "../../src/pointcloudviewerwidget.h"
#include "../../src/pointdata.h"

class SprintR3Demo : public QMainWindow
{
    Q_OBJECT

public:
    SprintR3Demo(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        generateTestData();
        connectSignals();
        
        // Auto-demo timer
        m_demoTimer = new QTimer(this);
        connect(m_demoTimer, &QTimer::timeout, this, &SprintR3Demo::runAutomaticDemo);
    }

private slots:
    void generateTestData()
    {
        qDebug() << "Generating Sprint R3 test data...";
        
        // Generate test point cloud with color and intensity attributes
        std::vector<float> points;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
        std::uniform_real_distribution<float> intensityDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> colorDist(0, 255);
        
        const int numPoints = 10000;
        
        for (int i = 0; i < numPoints; ++i) {
            // Position
            float x = posDist(gen);
            float y = posDist(gen);
            float z = posDist(gen);
            
            // Create point with attributes
            PointFullData point(x, y, z);
            
            // Add color based on position (rainbow effect)
            float normalizedX = (x + 10.0f) / 20.0f; // Normalize to 0-1
            uint8_t r = static_cast<uint8_t>(255 * normalizedX);
            uint8_t g = static_cast<uint8_t>(255 * (1.0f - normalizedX));
            uint8_t b = static_cast<uint8_t>(128);
            point.r = r;
            point.g = g;
            point.b = b;
            
            // Add intensity based on distance from origin
            float distance = std::sqrt(x*x + y*y + z*z);
            point.intensity = std::max(0.0f, std::min(1.0f, 1.0f - distance / 20.0f));
            
            // Convert to flat array format for viewer
            points.push_back(x);
            points.push_back(y);
            points.push_back(z);
        }
        
        // Load into viewer
        m_viewer->loadPointCloud(points);
        
        qDebug() << "Generated" << numPoints << "points with color and intensity attributes";
    }
    
    void onColorToggled(bool enabled)
    {
        m_viewer->setRenderWithColor(enabled);
        qDebug() << "Color rendering:" << (enabled ? "enabled" : "disabled");
    }
    
    void onIntensityToggled(bool enabled)
    {
        m_viewer->setRenderWithIntensity(enabled);
        qDebug() << "Intensity rendering:" << (enabled ? "enabled" : "disabled");
    }
    
    void onAttenuationToggled(bool enabled)
    {
        m_viewer->setPointSizeAttenuationEnabled(enabled);
        updateAttenuationParams();
        qDebug() << "Point size attenuation:" << (enabled ? "enabled" : "disabled");
    }
    
    void updateAttenuationParams()
    {
        float minSize = m_minSizeSlider->value() / 10.0f;
        float maxSize = m_maxSizeSlider->value() / 10.0f;
        float factor = m_attenuationFactorSlider->value() / 100.0f;
        
        m_viewer->setPointSizeAttenuationParams(minSize, maxSize, factor);
        
        m_minSizeLabel->setText(QString("Min Size: %1").arg(minSize, 0, 'f', 1));
        m_maxSizeLabel->setText(QString("Max Size: %1").arg(maxSize, 0, 'f', 1));
        m_attenuationFactorLabel->setText(QString("Factor: %1").arg(factor, 0, 'f', 2));
    }
    
    void startAutomaticDemo()
    {
        qDebug() << "Starting automatic Sprint R3 feature demonstration...";
        m_demoStep = 0;
        m_demoTimer->start(3000); // 3 second intervals
    }
    
    void runAutomaticDemo()
    {
        switch (m_demoStep) {
        case 0:
            qDebug() << "Demo Step 1: Enable color rendering";
            m_colorCheckbox->setChecked(true);
            break;
        case 1:
            qDebug() << "Demo Step 2: Enable intensity rendering";
            m_intensityCheckbox->setChecked(true);
            break;
        case 2:
            qDebug() << "Demo Step 3: Enable point size attenuation";
            m_attenuationCheckbox->setChecked(true);
            break;
        case 3:
            qDebug() << "Demo Step 4: Adjust attenuation parameters";
            m_minSizeSlider->setValue(5);
            m_maxSizeSlider->setValue(150);
            m_attenuationFactorSlider->setValue(20);
            break;
        case 4:
            qDebug() << "Demo Step 5: Reset to default view";
            m_colorCheckbox->setChecked(false);
            m_intensityCheckbox->setChecked(false);
            m_attenuationCheckbox->setChecked(false);
            break;
        default:
            qDebug() << "Demo completed. Restarting...";
            m_demoStep = -1; // Will be incremented to 0
            break;
        }
        
        m_demoStep++;
    }

private:
    void setupUI()
    {
        auto *centralWidget = new QWidget();
        setCentralWidget(centralWidget);
        
        auto *mainLayout = new QVBoxLayout(centralWidget);
        
        // Title
        auto *titleLabel = new QLabel("Sprint R3 Feature Demonstration");
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        
        // Point cloud viewer
        m_viewer = new PointCloudViewerWidget();
        m_viewer->setMinimumSize(800, 600);
        mainLayout->addWidget(m_viewer);
        
        // Controls
        auto *controlsWidget = new QWidget();
        auto *controlsLayout = new QHBoxLayout(controlsWidget);
        
        // Attribute rendering controls
        auto *attributeGroup = new QGroupBox("Attribute Rendering");
        auto *attributeLayout = new QVBoxLayout(attributeGroup);
        
        m_colorCheckbox = new QCheckBox("Color Rendering");
        m_intensityCheckbox = new QCheckBox("Intensity Rendering");
        
        attributeLayout->addWidget(m_colorCheckbox);
        attributeLayout->addWidget(m_intensityCheckbox);
        
        // Point size attenuation controls
        auto *attenuationGroup = new QGroupBox("Point Size Attenuation");
        auto *attenuationLayout = new QVBoxLayout(attenuationGroup);
        
        m_attenuationCheckbox = new QCheckBox("Enable Attenuation");
        attenuationLayout->addWidget(m_attenuationCheckbox);
        
        // Sliders
        auto *slidersLayout = new QHBoxLayout();
        
        auto *minSizeLayout = new QVBoxLayout();
        m_minSizeLabel = new QLabel("Min Size: 1.0");
        m_minSizeSlider = new QSlider(Qt::Horizontal);
        m_minSizeSlider->setRange(1, 50);
        m_minSizeSlider->setValue(10);
        minSizeLayout->addWidget(m_minSizeLabel);
        minSizeLayout->addWidget(m_minSizeSlider);
        
        auto *maxSizeLayout = new QVBoxLayout();
        m_maxSizeLabel = new QLabel("Max Size: 10.0");
        m_maxSizeSlider = new QSlider(Qt::Horizontal);
        m_maxSizeSlider->setRange(10, 200);
        m_maxSizeSlider->setValue(100);
        maxSizeLayout->addWidget(m_maxSizeLabel);
        maxSizeLayout->addWidget(m_maxSizeSlider);
        
        auto *factorLayout = new QVBoxLayout();
        m_attenuationFactorLabel = new QLabel("Factor: 0.1");
        m_attenuationFactorSlider = new QSlider(Qt::Horizontal);
        m_attenuationFactorSlider->setRange(1, 100);
        m_attenuationFactorSlider->setValue(10);
        factorLayout->addWidget(m_attenuationFactorLabel);
        factorLayout->addWidget(m_attenuationFactorSlider);
        
        slidersLayout->addLayout(minSizeLayout);
        slidersLayout->addLayout(maxSizeLayout);
        slidersLayout->addLayout(factorLayout);
        attenuationLayout->addLayout(slidersLayout);
        
        // Demo controls
        auto *demoGroup = new QGroupBox("Demo Controls");
        auto *demoLayout = new QHBoxLayout(demoGroup);
        
        auto *generateButton = new QPushButton("Generate New Data");
        auto *autoDemoButton = new QPushButton("Start Auto Demo");
        
        demoLayout->addWidget(generateButton);
        demoLayout->addWidget(autoDemoButton);
        
        // Add groups to controls
        controlsLayout->addWidget(attributeGroup);
        controlsLayout->addWidget(attenuationGroup);
        controlsLayout->addWidget(demoGroup);
        
        mainLayout->addWidget(controlsWidget);
        
        // Connect demo buttons
        connect(generateButton, &QPushButton::clicked, this, &SprintR3Demo::generateTestData);
        connect(autoDemoButton, &QPushButton::clicked, this, &SprintR3Demo::startAutomaticDemo);
        
        setWindowTitle("Sprint R3 Feature Demo - Point Cloud Attribute Rendering");
        resize(1000, 800);
    }
    
    void connectSignals()
    {
        connect(m_colorCheckbox, &QCheckBox::toggled, this, &SprintR3Demo::onColorToggled);
        connect(m_intensityCheckbox, &QCheckBox::toggled, this, &SprintR3Demo::onIntensityToggled);
        connect(m_attenuationCheckbox, &QCheckBox::toggled, this, &SprintR3Demo::onAttenuationToggled);
        
        connect(m_minSizeSlider, &QSlider::valueChanged, this, &SprintR3Demo::updateAttenuationParams);
        connect(m_maxSizeSlider, &QSlider::valueChanged, this, &SprintR3Demo::updateAttenuationParams);
        connect(m_attenuationFactorSlider, &QSlider::valueChanged, this, &SprintR3Demo::updateAttenuationParams);
    }
    
    PointCloudViewerWidget *m_viewer;
    QCheckBox *m_colorCheckbox;
    QCheckBox *m_intensityCheckbox;
    QCheckBox *m_attenuationCheckbox;
    QSlider *m_minSizeSlider;
    QSlider *m_maxSizeSlider;
    QSlider *m_attenuationFactorSlider;
    QLabel *m_minSizeLabel;
    QLabel *m_maxSizeLabel;
    QLabel *m_attenuationFactorLabel;
    QTimer *m_demoTimer;
    int m_demoStep = 0;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    SprintR3Demo demo;
    demo.show();
    
    return app.exec();
}

#include "sprint_r3_demo.moc"
