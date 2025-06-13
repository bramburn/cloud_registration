#ifndef VIEWERTOOLBAR_H
#define VIEWERTOOLBAR_H

#include <QAction>
#include <QActionGroup>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QToolBar>

/**
 * @brief ViewerToolbar - 3D viewer-specific toolbar controls
 *
 * This class provides a dedicated UI area for viewer-specific actions,
 * such as camera controls, making them easily accessible to the user.
 *
 * Sprint 1 Requirements:
 * - Camera preset controls (Top, Front, Side, Isometric)
 * - Fit to view functionality
 * - Rendering options and controls
 * - LOD system controls
 * - Point size and color controls
 */
class ViewerToolbar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit ViewerToolbar(QWidget* parent = nullptr);

    /**
     * @brief Get the fit to view action
     * @return Pointer to fit to view action
     */
    QAction* getFitToViewAction() const
    {
        return m_fitToViewAction;
    }

    /**
     * @brief Get the reset view action
     * @return Pointer to reset view action
     */
    QAction* getResetViewAction() const
    {
        return m_resetViewAction;
    }

    /**
     * @brief Enable or disable LOD controls
     * @param enabled True to enable LOD controls
     */
    void setLODControlsEnabled(bool enabled);

    /**
     * @brief Set point size value
     * @param size Point size (1-20)
     */
    void setPointSize(int size);

    /**
     * @brief Get current point size
     * @return Current point size
     */
    int getPointSize() const;

    /**
     * @brief Set LOD enabled state
     * @param enabled True if LOD is enabled
     */
    void setLODEnabled(bool enabled);

    /**
     * @brief Get LOD enabled state
     * @return True if LOD is enabled
     */
    bool isLODEnabled() const;

signals:
    // Camera control signals
    void fitToViewRequested();
    void resetViewRequested();
    void topViewRequested();
    void frontViewRequested();
    void sideViewRequested();
    void isometricViewRequested();

    // Rendering control signals
    void pointSizeChanged(int size);
    void lodEnabledChanged(bool enabled);
    void lodQualityChanged(float quality);
    void wireframeToggled(bool enabled);
    void showBoundingBoxToggled(bool enabled);

    // Performance signals
    void performanceStatsToggled(bool enabled);

private slots:
    void onFitToView();
    void onResetView();
    void onTopView();
    void onFrontView();
    void onSideView();
    void onIsometricView();
    void onPointSizeChanged(int value);
    void onLODToggled(bool enabled);
    void onLODQualityChanged(int value);
    void onWireframeToggled(bool enabled);
    void onBoundingBoxToggled(bool enabled);
    void onPerformanceStatsToggled(bool enabled);

private:
    // Camera actions
    QAction* m_fitToViewAction;
    QAction* m_resetViewAction;
    QActionGroup* m_viewActionGroup;
    QAction* m_topViewAction;
    QAction* m_frontViewAction;
    QAction* m_sideViewAction;
    QAction* m_isometricViewAction;

    // Rendering controls
    QLabel* m_pointSizeLabel;
    QSpinBox* m_pointSizeSpinBox;
    QCheckBox* m_lodCheckBox;
    QLabel* m_lodQualityLabel;
    QSlider* m_lodQualitySlider;
    QCheckBox* m_wireframeCheckBox;
    QCheckBox* m_boundingBoxCheckBox;

    // Performance controls
    QCheckBox* m_performanceStatsCheckBox;

    // Helper methods
    void setupCameraActions();
    void setupRenderingControls();
    void setupPerformanceControls();
    void addSeparatorWithSpacing();
};

#endif  // VIEWERTOOLBAR_H
