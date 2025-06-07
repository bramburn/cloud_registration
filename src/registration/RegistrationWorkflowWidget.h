#pragma once

#include "PoseGraph.h"
#include "PoseGraphBuilder.h"
#include "../optimization/BundleAdjustment.h"
#include "../features/FeatureExtractor.h"
#include "FeatureBasedRegistration.h"
#include "../analysis/DifferenceAnalysis.h"
#include "../project.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTabWidget>
#include <memory>

namespace Registration {

/**
 * @brief Main widget for Sprint 9 advanced registration workflow
 * 
 * Integrates global optimization, feature-based registration, and visual analysis tools
 */
class RegistrationWorkflowWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit RegistrationWorkflowWidget(QWidget* parent = nullptr);
    
    /**
     * @brief Set the current project for registration operations
     * @param project Project containing scan data
     */
    void setProject(const Project& project);
    
    /**
     * @brief Get current registration results
     * @return Current pose graph if available
     */
    std::shared_ptr<PoseGraph> getCurrentPoseGraph() const { return m_currentPoseGraph; }
    
signals:
    void registrationCompleted(bool success);
    void globalOptimizationCompleted(bool success);
    void featureRegistrationCompleted(bool success);
    void analysisCompleted(const Analysis::DifferenceAnalysis::Statistics& stats);
    
public slots:
    void onGloballyOptimizeProject();
    void onAlignByFeatures();
    void onShowDifferenceHeatMap(bool enabled);
    void onAnalyzeRegistrationQuality();
    
private slots:
    void onBundleAdjustmentProgress(int iteration, double currentError, double lambda);
    void onBundleAdjustmentCompleted(const Optimization::BundleAdjustment::Result& result);
    void onFeatureRegistrationProgress(int percentage);
    void onFeatureRegistrationCompleted(const FeatureBasedRegistration::Result& result);
    void onDifferenceAnalysisCompleted(const Analysis::DifferenceAnalysis::Statistics& stats);
    
private:
    /**
     * @brief Setup the user interface
     */
    void setupUI();
    
    /**
     * @brief Create global optimization controls
     */
    QGroupBox* createGlobalOptimizationGroup();
    
    /**
     * @brief Create feature-based registration controls
     */
    QGroupBox* createFeatureRegistrationGroup();
    
    /**
     * @brief Create visual analysis controls
     */
    QGroupBox* createVisualAnalysisGroup();
    
    /**
     * @brief Create results display area
     */
    QWidget* createResultsDisplay();
    
    /**
     * @brief Update UI state based on current project
     */
    void updateUIState();
    
    /**
     * @brief Log message to results display
     */
    void logMessage(const QString& message);
    
    /**
     * @brief Clear results display
     */
    void clearResults();
    
    // Core components
    std::unique_ptr<PoseGraphBuilder> m_poseGraphBuilder;
    std::unique_ptr<Optimization::BundleAdjustment> m_bundleAdjustment;
    std::unique_ptr<Features::FeatureExtractor> m_featureExtractor;
    std::unique_ptr<FeatureBasedRegistration> m_featureRegistration;
    std::unique_ptr<Analysis::DifferenceAnalysis> m_differenceAnalysis;
    
    // Current state
    Project m_currentProject;
    std::shared_ptr<PoseGraph> m_currentPoseGraph;
    bool m_hasValidProject = false;
    
    // UI Components - Global Optimization
    QGroupBox* m_globalOptGroup;
    QPushButton* m_globalOptimizeButton;
    QSpinBox* m_maxIterationsSpin;
    QDoubleSpinBox* m_convergenceThresholdSpin;
    QCheckBox* m_fixFirstPoseCheck;
    QProgressBar* m_globalOptProgress;
    QLabel* m_globalOptStatus;
    
    // UI Components - Feature Registration
    QGroupBox* m_featureRegGroup;
    QPushButton* m_alignByFeaturesButton;
    QSpinBox* m_maxPlanesSpin;
    QDoubleSpinBox* m_planeDistanceThresholdSpin;
    QSpinBox* m_minInliersSpin;
    QProgressBar* m_featureRegProgress;
    QLabel* m_featureRegStatus;
    
    // UI Components - Visual Analysis
    QGroupBox* m_visualAnalysisGroup;
    QCheckBox* m_showDifferenceHeatMapCheck;
    QPushButton* m_analyzeQualityButton;
    QDoubleSpinBox* m_maxSearchDistanceSpin;
    QCheckBox* m_useKDTreeCheck;
    QLabel* m_analysisStatus;
    
    // UI Components - Results
    QTabWidget* m_resultsTabWidget;
    QTextEdit* m_logTextEdit;
    QTextEdit* m_statisticsTextEdit;
    QLabel* m_summaryLabel;
};

} // namespace Registration
