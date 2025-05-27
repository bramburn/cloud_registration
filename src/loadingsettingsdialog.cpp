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
    , m_qSettings("CloudRegistration", "PointCloudViewer")
{
    setWindowTitle("Point Cloud Loading Settings");
    setModal(true);
    setFixedSize(400, 200);

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
    m_methodComboBox->setToolTip("Full Load: Loads all point data\nHeader-Only: Reads only file metadata for quick inspection");
    methodLayout->addWidget(m_methodComboBox);

    m_mainLayout->addWidget(methodGroup);

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
    
    // Sync to ensure settings are written immediately
    m_qSettings.sync();
    
    qDebug() << "Saved loading settings - Method:" << static_cast<int>(m_currentSettings.method);
}

void LoadingSettingsDialog::updateUIForMethod(LoadingMethod method)
{
    // Update UI elements based on selected method
    // For now, just update tooltips and enable/disable controls as needed
    
    switch (method) {
        case LoadingMethod::FullLoad:
            m_methodComboBox->setToolTip("Full Load: Loads all point data for complete visualization");
            break;
        case LoadingMethod::HeaderOnly:
            m_methodComboBox->setToolTip("Header-Only: Reads only file metadata for quick inspection");
            break;
    }
}
