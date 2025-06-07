#include "ViewerToolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QStyle>

ViewerToolbar::ViewerToolbar(QWidget* parent)
    : QToolBar(parent)
    , m_fitToViewAction(nullptr)
    , m_resetViewAction(nullptr)
    , m_viewActionGroup(nullptr)
    , m_topViewAction(nullptr)
    , m_frontViewAction(nullptr)
    , m_sideViewAction(nullptr)
    , m_isometricViewAction(nullptr)
    , m_pointSizeLabel(nullptr)
    , m_pointSizeSpinBox(nullptr)
    , m_lodCheckBox(nullptr)
    , m_lodQualityLabel(nullptr)
    , m_lodQualitySlider(nullptr)
    , m_wireframeCheckBox(nullptr)
    , m_boundingBoxCheckBox(nullptr)
    , m_performanceStatsCheckBox(nullptr)
{
    setObjectName("ViewerToolbar");
    setWindowTitle("3D Viewer Controls");
    
    setupCameraActions();
    addSeparatorWithSpacing();
    setupRenderingControls();
    addSeparatorWithSpacing();
    setupPerformanceControls();
}

void ViewerToolbar::setupCameraActions()
{
    // Fit to view action
    m_fitToViewAction = addAction("Fit to View");
    m_fitToViewAction->setToolTip("Fit the entire point cloud in view");
    m_fitToViewAction->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    connect(m_fitToViewAction, &QAction::triggered, this, &ViewerToolbar::onFitToView);

    // Reset view action
    m_resetViewAction = addAction("Reset");
    m_resetViewAction->setToolTip("Reset camera to default position");
    m_resetViewAction->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    connect(m_resetViewAction, &QAction::triggered, this, &ViewerToolbar::onResetView);

    addSeparator();

    // View preset actions
    m_viewActionGroup = new QActionGroup(this);
    
    m_topViewAction = addAction("Top");
    m_topViewAction->setToolTip("Set camera to top view");
    m_topViewAction->setCheckable(true);
    m_viewActionGroup->addAction(m_topViewAction);
    connect(m_topViewAction, &QAction::triggered, this, &ViewerToolbar::onTopView);

    m_frontViewAction = addAction("Front");
    m_frontViewAction->setToolTip("Set camera to front view");
    m_frontViewAction->setCheckable(true);
    m_viewActionGroup->addAction(m_frontViewAction);
    connect(m_frontViewAction, &QAction::triggered, this, &ViewerToolbar::onFrontView);

    m_sideViewAction = addAction("Side");
    m_sideViewAction->setToolTip("Set camera to side view");
    m_sideViewAction->setCheckable(true);
    m_viewActionGroup->addAction(m_sideViewAction);
    connect(m_sideViewAction, &QAction::triggered, this, &ViewerToolbar::onSideView);

    m_isometricViewAction = addAction("Isometric");
    m_isometricViewAction->setToolTip("Set camera to isometric view");
    m_isometricViewAction->setCheckable(true);
    m_isometricViewAction->setChecked(true); // Default view
    m_viewActionGroup->addAction(m_isometricViewAction);
    connect(m_isometricViewAction, &QAction::triggered, this, &ViewerToolbar::onIsometricView);
}

void ViewerToolbar::setupRenderingControls()
{
    // Point size control
    m_pointSizeLabel = new QLabel("Point Size:");
    addWidget(m_pointSizeLabel);
    
    m_pointSizeSpinBox = new QSpinBox();
    m_pointSizeSpinBox->setRange(1, 20);
    m_pointSizeSpinBox->setValue(2);
    m_pointSizeSpinBox->setToolTip("Adjust point rendering size");
    addWidget(m_pointSizeSpinBox);
    connect(m_pointSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ViewerToolbar::onPointSizeChanged);

    addSeparator();

    // LOD controls
    m_lodCheckBox = new QCheckBox("LOD");
    m_lodCheckBox->setToolTip("Enable Level of Detail optimization");
    m_lodCheckBox->setChecked(false);
    addWidget(m_lodCheckBox);
    connect(m_lodCheckBox, &QCheckBox::toggled, this, &ViewerToolbar::onLODToggled);

    m_lodQualityLabel = new QLabel("Quality:");
    addWidget(m_lodQualityLabel);
    
    m_lodQualitySlider = new QSlider(Qt::Horizontal);
    m_lodQualitySlider->setRange(10, 100);
    m_lodQualitySlider->setValue(100);
    m_lodQualitySlider->setMaximumWidth(100);
    m_lodQualitySlider->setToolTip("Adjust LOD quality level");
    addWidget(m_lodQualitySlider);
    connect(m_lodQualitySlider, &QSlider::valueChanged, this, &ViewerToolbar::onLODQualityChanged);

    addSeparator();

    // Wireframe and bounding box
    m_wireframeCheckBox = new QCheckBox("Wireframe");
    m_wireframeCheckBox->setToolTip("Toggle wireframe rendering");
    addWidget(m_wireframeCheckBox);
    connect(m_wireframeCheckBox, &QCheckBox::toggled, this, &ViewerToolbar::onWireframeToggled);

    m_boundingBoxCheckBox = new QCheckBox("Bounding Box");
    m_boundingBoxCheckBox->setToolTip("Show point cloud bounding box");
    addWidget(m_boundingBoxCheckBox);
    connect(m_boundingBoxCheckBox, &QCheckBox::toggled, this, &ViewerToolbar::onBoundingBoxToggled);

    // Initially disable LOD quality controls
    setLODControlsEnabled(false);
}

void ViewerToolbar::setupPerformanceControls()
{
    m_performanceStatsCheckBox = new QCheckBox("Performance Stats");
    m_performanceStatsCheckBox->setToolTip("Show performance statistics overlay");
    addWidget(m_performanceStatsCheckBox);
    connect(m_performanceStatsCheckBox, &QCheckBox::toggled, 
            this, &ViewerToolbar::onPerformanceStatsToggled);
}

void ViewerToolbar::setLODControlsEnabled(bool enabled)
{
    m_lodQualityLabel->setEnabled(enabled);
    m_lodQualitySlider->setEnabled(enabled);
}

void ViewerToolbar::setPointSize(int size)
{
    m_pointSizeSpinBox->setValue(size);
}

int ViewerToolbar::getPointSize() const
{
    return m_pointSizeSpinBox->value();
}

void ViewerToolbar::setLODEnabled(bool enabled)
{
    m_lodCheckBox->setChecked(enabled);
    setLODControlsEnabled(enabled);
}

bool ViewerToolbar::isLODEnabled() const
{
    return m_lodCheckBox->isChecked();
}

void ViewerToolbar::addSeparatorWithSpacing()
{
    addSeparator();
    // Add some visual spacing
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth(10);
    addWidget(spacer);
}

// Slot implementations
void ViewerToolbar::onFitToView()
{
    emit fitToViewRequested();
}

void ViewerToolbar::onResetView()
{
    emit resetViewRequested();
}

void ViewerToolbar::onTopView()
{
    emit topViewRequested();
}

void ViewerToolbar::onFrontView()
{
    emit frontViewRequested();
}

void ViewerToolbar::onSideView()
{
    emit sideViewRequested();
}

void ViewerToolbar::onIsometricView()
{
    emit isometricViewRequested();
}

void ViewerToolbar::onPointSizeChanged(int value)
{
    emit pointSizeChanged(value);
}

void ViewerToolbar::onLODToggled(bool enabled)
{
    setLODControlsEnabled(enabled);
    emit lodEnabledChanged(enabled);
}

void ViewerToolbar::onLODQualityChanged(int value)
{
    float quality = static_cast<float>(value) / 100.0f;
    emit lodQualityChanged(quality);
}

void ViewerToolbar::onWireframeToggled(bool enabled)
{
    emit wireframeToggled(enabled);
}

void ViewerToolbar::onBoundingBoxToggled(bool enabled)
{
    emit showBoundingBoxToggled(enabled);
}

void ViewerToolbar::onPerformanceStatsToggled(bool enabled)
{
    emit performanceStatsToggled(enabled);
}
