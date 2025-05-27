#include "loadingsettingsdialog.h"
#include <QGroupBox>
#include <QDebug>

LoadingSettingsDialog::LoadingSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_methodComboBox(nullptr)
    , m_applyButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_methodLabel(nullptr)
    , m_voxelParametersGroup(nullptr)
    , m_voxelParametersLayout(nullptr)
    , m_leafSizeLabel(nullptr)
    , m_leafSizeSpinBox(nullptr)
    , m_minPointsLabel(nullptr)
    , m_minPointsSpinBox(nullptr)
    , m_qSettings("CloudRegistration", "PointCloudViewer")
{
    setWindowTitle("Point Cloud Loading Settings");
    setModal(true);
    setFixedSize(450, 300);

    setupUI();
    loadSettings();
}

LoadingSettingsDialog::~LoadingSettingsDialog()
{
    // Qt handles cleanup automatically through parent-child relationships
}

void LoadingSettingsDialog::setupUI()
{
    // Create main layout
    m_mainLayout = new QVBoxLayout(this);

    // Create method selection group
    QGroupBox *methodGroup = new QGroupBox("Loading Method", this);
    QVBoxLayout *methodLayout = new QVBoxLayout(methodGroup);

    m_methodLabel = new QLabel("Select how point cloud files should be loaded:", this);
    methodLayout->addWidget(m_methodLabel);

    // Create method combo box
    m_methodComboBox = new QComboBox(this);
    m_methodComboBox->addItem("Full Load", static_cast<int>(LoadingMethod::FullLoad));
    m_methodComboBox->addItem("Header-Only", static_cast<int>(LoadingMethod::HeaderOnly));
    m_methodComboBox->addItem("Voxel Grid", static_cast<int>(LoadingMethod::VoxelGrid));
    m_methodComboBox->setToolTip("Full Load: Loads all point data\nHeader-Only: Reads only file metadata\nVoxel Grid: Applies subsampling for reduced point count");
    methodLayout->addWidget(m_methodComboBox);

    m_mainLayout->addWidget(methodGroup);

    // Create voxel parameters group (initially hidden)
    m_voxelParametersGroup = new QGroupBox("Voxel Grid Parameters", this);
    m_voxelParametersLayout = new QVBoxLayout(m_voxelParametersGroup);

    // Leaf Size control
    m_leafSizeLabel = new QLabel("Leaf Size (m):", this);
    m_leafSizeSpinBox = new QDoubleSpinBox(this);
    m_leafSizeSpinBox->setRange(0.01, 5.0);
    m_leafSizeSpinBox->setSingleStep(0.1);
    m_leafSizeSpinBox->setDecimals(2);
    m_leafSizeSpinBox->setValue(0.1);
    m_leafSizeSpinBox->setToolTip("Controls the size of each 3D voxel cube.\nSmaller values preserve more detail but result in more points;\nlarger values drastically reduce point count for faster processing.");

    m_voxelParametersLayout->addWidget(m_leafSizeLabel);
    m_voxelParametersLayout->addWidget(m_leafSizeSpinBox);

    // Min Points Per Voxel control
    m_minPointsLabel = new QLabel("Min Points Per Voxel:", this);
    m_minPointsSpinBox = new QSpinBox(this);
    m_minPointsSpinBox->setRange(1, 10);
    m_minPointsSpinBox->setValue(1);
    m_minPointsSpinBox->setToolTip("Sets the minimum number of original points required within a voxel\nfor it to contribute a point to the subsampled cloud.\nUseful for filtering noise.");

    m_voxelParametersLayout->addWidget(m_minPointsLabel);
    m_voxelParametersLayout->addWidget(m_minPointsSpinBox);

    // Initially hide the voxel parameters group
    m_voxelParametersGroup->setVisible(false);
    m_mainLayout->addWidget(m_voxelParametersGroup);

    // Create button layout
    m_buttonLayout = new QHBoxLayout();

    m_applyButton = new QPushButton("Apply", this);
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Cancel", this);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_applyButton);
    m_buttonLayout->addWidget(m_okButton);
    m_buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addLayout(m_buttonLayout);

    // Connect signals
    connect(m_methodComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LoadingSettingsDialog::onMethodChanged);
    connect(m_applyButton, &QPushButton::clicked, this, &LoadingSettingsDialog::onApplyClicked);
    connect(m_okButton, &QPushButton::clicked, this, &LoadingSettingsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoadingSettingsDialog::onCancelClicked);

    // Connect voxel parameter controls to update settings
    connect(m_leafSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
                m_currentSettings.parameters["leafSize"] = value;
            });
    connect(m_minPointsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) {
                m_currentSettings.parameters["minPointsPerVoxel"] = value;
            });
}

LoadingSettings LoadingSettingsDialog::getSettings() const
{
    return m_currentSettings;
}

void LoadingSettingsDialog::onApplyClicked()
{
    saveSettings();
}

void LoadingSettingsDialog::onOkClicked()
{
    saveSettings();
    accept();
}

void LoadingSettingsDialog::onCancelClicked()
{
    // Reload settings to discard any changes
    loadSettings();
    reject();
}

void LoadingSettingsDialog::onMethodChanged(int index)
{
    if (index >= 0 && index < m_methodComboBox->count()) {
        LoadingMethod method = static_cast<LoadingMethod>(m_methodComboBox->itemData(index).toInt());
        m_currentSettings.method = method;

        // Update parameters based on method
        if (method == LoadingMethod::VoxelGrid) {
            m_currentSettings.parameters["leafSize"] = m_leafSizeSpinBox->value();
            m_currentSettings.parameters["minPointsPerVoxel"] = m_minPointsSpinBox->value();
        }

        updateUIForMethod(method);
    }
}

void LoadingSettingsDialog::loadSettings()
{
    // Read settings from QSettings
    int methodValue = m_qSettings.value("PointCloudLoading/DefaultMethod",
                                       static_cast<int>(LoadingMethod::FullLoad)).toInt();

    LoadingMethod method = static_cast<LoadingMethod>(methodValue);
    m_currentSettings.method = method;

    // Load VoxelGrid parameters
    double leafSize = m_qSettings.value("PointCloudLoading/VoxelGrid/LeafSize", 0.1).toDouble();
    int minPointsPerVoxel = m_qSettings.value("PointCloudLoading/VoxelGrid/MinPointsPerVoxel", 1).toInt();

    m_leafSizeSpinBox->setValue(leafSize);
    m_minPointsSpinBox->setValue(minPointsPerVoxel);

    m_currentSettings.parameters["leafSize"] = leafSize;
    m_currentSettings.parameters["minPointsPerVoxel"] = minPointsPerVoxel;

    // Update UI to reflect loaded settings
    for (int i = 0; i < m_methodComboBox->count(); ++i) {
        if (m_methodComboBox->itemData(i).toInt() == static_cast<int>(method)) {
            m_methodComboBox->setCurrentIndex(i);
            break;
        }
    }

    updateUIForMethod(method);
}

void LoadingSettingsDialog::saveSettings()
{
    // Save current settings to QSettings
    m_qSettings.setValue("PointCloudLoading/DefaultMethod",
                        static_cast<int>(m_currentSettings.method));

    // Save VoxelGrid parameters
    m_qSettings.setValue("PointCloudLoading/VoxelGrid/LeafSize", m_leafSizeSpinBox->value());
    m_qSettings.setValue("PointCloudLoading/VoxelGrid/MinPointsPerVoxel", m_minPointsSpinBox->value());

    // Update current settings parameters
    m_currentSettings.parameters["leafSize"] = m_leafSizeSpinBox->value();
    m_currentSettings.parameters["minPointsPerVoxel"] = m_minPointsSpinBox->value();

    // Sync to ensure settings are written immediately
    m_qSettings.sync();

    qDebug() << "Saved loading settings - Method:" << static_cast<int>(m_currentSettings.method);
}

void LoadingSettingsDialog::updateUIForMethod(LoadingMethod method)
{
    // Update UI elements based on selected method
    // Show/hide voxel parameters group based on method selection
    bool showVoxelParams = (method == LoadingMethod::VoxelGrid);
    m_voxelParametersGroup->setVisible(showVoxelParams);

    switch (method) {
        case LoadingMethod::FullLoad:
            m_methodComboBox->setToolTip("Full Load: Loads all point data for complete visualization");
            break;
        case LoadingMethod::HeaderOnly:
            m_methodComboBox->setToolTip("Header-Only: Reads only file metadata for quick inspection");
            break;
        case LoadingMethod::VoxelGrid:
            m_methodComboBox->setToolTip("Voxel Grid: Applies subsampling for reduced point count with uniform density");
            break;
    }
}
