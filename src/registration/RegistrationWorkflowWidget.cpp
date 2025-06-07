#include "RegistrationWorkflowWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QSplitter>

namespace Registration {

RegistrationWorkflowWidget::RegistrationWorkflowWidget(QWidget* parent) 
    : QWidget(parent) {
    
    // Initialize core components
    m_poseGraphBuilder = std::make_unique<PoseGraphBuilder>(this);
    m_bundleAdjustment = std::make_unique<Optimization::BundleAdjustment>(this);
    m_featureExtractor = std::make_unique<Features::FeatureExtractor>(this);
    m_featureRegistration = std::make_unique<FeatureBasedRegistration>(this);
    m_differenceAnalysis = std::make_unique<Analysis::DifferenceAnalysis>(this);
    
    // Connect signals
    connect(m_bundleAdjustment.get(), &Optimization::BundleAdjustment::optimizationProgress,
            this, &RegistrationWorkflowWidget::onBundleAdjustmentProgress);
    connect(m_bundleAdjustment.get(), &Optimization::BundleAdjustment::optimizationCompleted,
            this, &RegistrationWorkflowWidget::onBundleAdjustmentCompleted);
    
    connect(m_featureRegistration.get(), &FeatureBasedRegistration::registrationProgress,
            this, &RegistrationWorkflowWidget::onFeatureRegistrationProgress);
    connect(m_featureRegistration.get(), &FeatureBasedRegistration::registrationCompleted,
            this, &RegistrationWorkflowWidget::onFeatureRegistrationCompleted);
    
    connect(m_differenceAnalysis.get(), &Analysis::DifferenceAnalysis::analysisCompleted,
            this, &RegistrationWorkflowWidget::onDifferenceAnalysisCompleted);
    
    setupUI();
    updateUIState();
}

void RegistrationWorkflowWidget::setProject(const Project& project) {
    m_currentProject = project;
    m_hasValidProject = true;
    m_currentPoseGraph.reset();
    
    logMessage(QString("Project loaded: %1").arg(project.getName()));
    updateUIState();
}

void RegistrationWorkflowWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    
    // Create main splitter
    auto* splitter = new QSplitter(Qt::Vertical, this);
    
    // Create control panels
    auto* controlsWidget = new QWidget();
    auto* controlsLayout = new QHBoxLayout(controlsWidget);
    
    controlsLayout->addWidget(createGlobalOptimizationGroup());
    controlsLayout->addWidget(createFeatureRegistrationGroup());
    controlsLayout->addWidget(createVisualAnalysisGroup());
    
    splitter->addWidget(controlsWidget);
    splitter->addWidget(createResultsDisplay());
    
    // Set splitter proportions
    splitter->setSizes({300, 400});
    
    mainLayout->addWidget(splitter);
}

QGroupBox* RegistrationWorkflowWidget::createGlobalOptimizationGroup() {
    m_globalOptGroup = new QGroupBox("Global Optimization (Bundle Adjustment)");
    auto* layout = new QVBoxLayout(m_globalOptGroup);
    
    // Parameters
    auto* paramsLayout = new QHBoxLayout();
    
    paramsLayout->addWidget(new QLabel("Max Iterations:"));
    m_maxIterationsSpin = new QSpinBox();
    m_maxIterationsSpin->setRange(10, 1000);
    m_maxIterationsSpin->setValue(100);
    paramsLayout->addWidget(m_maxIterationsSpin);
    
    paramsLayout->addWidget(new QLabel("Convergence:"));
    m_convergenceThresholdSpin = new QDoubleSpinBox();
    m_convergenceThresholdSpin->setRange(1e-8, 1e-3);
    m_convergenceThresholdSpin->setDecimals(8);
    m_convergenceThresholdSpin->setValue(1e-6);
    m_convergenceThresholdSpin->setSingleStep(1e-7);
    paramsLayout->addWidget(m_convergenceThresholdSpin);
    
    m_fixFirstPoseCheck = new QCheckBox("Fix First Pose");
    m_fixFirstPoseCheck->setChecked(true);
    paramsLayout->addWidget(m_fixFirstPoseCheck);
    
    layout->addLayout(paramsLayout);
    
    // Control button
    m_globalOptimizeButton = new QPushButton("Globally Optimize Project");
    connect(m_globalOptimizeButton, &QPushButton::clicked,
            this, &RegistrationWorkflowWidget::onGloballyOptimizeProject);
    layout->addWidget(m_globalOptimizeButton);
    
    // Progress and status
    m_globalOptProgress = new QProgressBar();
    m_globalOptProgress->setVisible(false);
    layout->addWidget(m_globalOptProgress);
    
    m_globalOptStatus = new QLabel("Ready");
    layout->addWidget(m_globalOptStatus);
    
    return m_globalOptGroup;
}

QGroupBox* RegistrationWorkflowWidget::createFeatureRegistrationGroup() {
    m_featureRegGroup = new QGroupBox("Feature-Based Registration");
    auto* layout = new QVBoxLayout(m_featureRegGroup);
    
    // Parameters
    auto* paramsLayout = new QHBoxLayout();
    
    paramsLayout->addWidget(new QLabel("Max Planes:"));
    m_maxPlanesSpin = new QSpinBox();
    m_maxPlanesSpin->setRange(3, 20);
    m_maxPlanesSpin->setValue(10);
    paramsLayout->addWidget(m_maxPlanesSpin);
    
    paramsLayout->addWidget(new QLabel("Distance Threshold:"));
    m_planeDistanceThresholdSpin = new QDoubleSpinBox();
    m_planeDistanceThresholdSpin->setRange(0.001, 0.1);
    m_planeDistanceThresholdSpin->setDecimals(3);
    m_planeDistanceThresholdSpin->setValue(0.02);
    m_planeDistanceThresholdSpin->setSingleStep(0.001);
    paramsLayout->addWidget(m_planeDistanceThresholdSpin);
    
    paramsLayout->addWidget(new QLabel("Min Inliers:"));
    m_minInliersSpin = new QSpinBox();
    m_minInliersSpin->setRange(50, 1000);
    m_minInliersSpin->setValue(100);
    paramsLayout->addWidget(m_minInliersSpin);
    
    layout->addLayout(paramsLayout);
    
    // Control button
    m_alignByFeaturesButton = new QPushButton("Align by Features");
    connect(m_alignByFeaturesButton, &QPushButton::clicked,
            this, &RegistrationWorkflowWidget::onAlignByFeatures);
    layout->addWidget(m_alignByFeaturesButton);
    
    // Progress and status
    m_featureRegProgress = new QProgressBar();
    m_featureRegProgress->setVisible(false);
    layout->addWidget(m_featureRegProgress);
    
    m_featureRegStatus = new QLabel("Ready");
    layout->addWidget(m_featureRegStatus);
    
    return m_featureRegGroup;
}

QGroupBox* RegistrationWorkflowWidget::createVisualAnalysisGroup() {
    m_visualAnalysisGroup = new QGroupBox("Visual Registration Analysis");
    auto* layout = new QVBoxLayout(m_visualAnalysisGroup);
    
    // Heat map toggle
    m_showDifferenceHeatMapCheck = new QCheckBox("Show Difference Heat Map");
    connect(m_showDifferenceHeatMapCheck, &QCheckBox::toggled,
            this, &RegistrationWorkflowWidget::onShowDifferenceHeatMap);
    layout->addWidget(m_showDifferenceHeatMapCheck);
    
    // Parameters
    auto* paramsLayout = new QHBoxLayout();
    
    paramsLayout->addWidget(new QLabel("Max Search Distance:"));
    m_maxSearchDistanceSpin = new QDoubleSpinBox();
    m_maxSearchDistanceSpin->setRange(0.01, 10.0);
    m_maxSearchDistanceSpin->setDecimals(2);
    m_maxSearchDistanceSpin->setValue(1.0);
    m_maxSearchDistanceSpin->setSingleStep(0.1);
    paramsLayout->addWidget(m_maxSearchDistanceSpin);
    
    m_useKDTreeCheck = new QCheckBox("Use KD-Tree");
    m_useKDTreeCheck->setChecked(true);
    paramsLayout->addWidget(m_useKDTreeCheck);
    
    layout->addLayout(paramsLayout);
    
    // Analysis button
    m_analyzeQualityButton = new QPushButton("Analyze Registration Quality");
    connect(m_analyzeQualityButton, &QPushButton::clicked,
            this, &RegistrationWorkflowWidget::onAnalyzeRegistrationQuality);
    layout->addWidget(m_analyzeQualityButton);
    
    // Status
    m_analysisStatus = new QLabel("Ready");
    layout->addWidget(m_analysisStatus);
    
    return m_visualAnalysisGroup;
}

QWidget* RegistrationWorkflowWidget::createResultsDisplay() {
    m_resultsTabWidget = new QTabWidget();
    
    // Log tab
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumBlockCount(1000); // Limit log size
    m_resultsTabWidget->addTab(m_logTextEdit, "Log");
    
    // Statistics tab
    m_statisticsTextEdit = new QTextEdit();
    m_statisticsTextEdit->setReadOnly(true);
    m_resultsTabWidget->addTab(m_statisticsTextEdit, "Statistics");
    
    // Summary
    auto* summaryWidget = new QWidget();
    auto* summaryLayout = new QVBoxLayout(summaryWidget);
    
    m_summaryLabel = new QLabel("No registration results available");
    m_summaryLabel->setWordWrap(true);
    m_summaryLabel->setAlignment(Qt::AlignTop);
    summaryLayout->addWidget(m_summaryLabel);
    summaryLayout->addStretch();
    
    m_resultsTabWidget->addTab(summaryWidget, "Summary");
    
    return m_resultsTabWidget;
}

void RegistrationWorkflowWidget::updateUIState() {
    bool hasProject = m_hasValidProject;
    bool hasGraph = (m_currentPoseGraph != nullptr);
    
    // Global optimization requires a project with scans
    m_globalOptimizeButton->setEnabled(hasProject);
    
    // Feature registration requires a project with scans
    m_alignByFeaturesButton->setEnabled(hasProject);
    
    // Analysis requires registration results
    m_analyzeQualityButton->setEnabled(hasGraph);
    m_showDifferenceHeatMapCheck->setEnabled(hasGraph);
    
    if (!hasProject) {
        m_globalOptStatus->setText("No project loaded");
        m_featureRegStatus->setText("No project loaded");
        m_analysisStatus->setText("No project loaded");
    } else {
        m_globalOptStatus->setText("Ready");
        m_featureRegStatus->setText("Ready");
        m_analysisStatus->setText(hasGraph ? "Ready" : "No registration results");
    }
}

void RegistrationWorkflowWidget::logMessage(const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    m_logTextEdit->append(logEntry);
    qDebug() << logEntry;
}

void RegistrationWorkflowWidget::clearResults() {
    m_logTextEdit->clear();
    m_statisticsTextEdit->clear();
    m_summaryLabel->setText("No registration results available");
}

void RegistrationWorkflowWidget::onGloballyOptimizeProject() {
    if (!m_hasValidProject) {
        QMessageBox::warning(this, "Warning", "No project loaded for optimization");
        return;
    }

    logMessage("Starting global optimization (bundle adjustment)...");

    // Disable UI during optimization
    m_globalOptimizeButton->setEnabled(false);
    m_globalOptProgress->setVisible(true);
    m_globalOptProgress->setValue(0);
    m_globalOptStatus->setText("Building pose graph...");

    try {
        // Build pose graph from project
        auto poseGraph = m_poseGraphBuilder->build(m_currentProject);

        if (!poseGraph || poseGraph->isEmpty()) {
            logMessage("ERROR: Failed to build pose graph from project");
            m_globalOptStatus->setText("Failed to build pose graph");
            m_globalOptimizeButton->setEnabled(true);
            m_globalOptProgress->setVisible(false);
            return;
        }

        logMessage(QString("Built pose graph with %1 nodes and %2 edges")
                  .arg(poseGraph->nodeCount()).arg(poseGraph->edgeCount()));

        // Setup optimization parameters
        Optimization::BundleAdjustment::Parameters params;
        params.maxIterations = m_maxIterationsSpin->value();
        params.convergenceThreshold = m_convergenceThresholdSpin->value();
        params.fixFirstPose = m_fixFirstPoseCheck->isChecked();
        params.verbose = true;

        m_globalOptStatus->setText("Optimizing poses...");

        // Start optimization (this will trigger progress signals)
        auto [optimizedGraph, result] = m_bundleAdjustment->optimize(*poseGraph, params);

        // The completion will be handled in onBundleAdjustmentCompleted

    } catch (const std::exception& e) {
        logMessage(QString("ERROR: Global optimization failed: %1").arg(e.what()));
        m_globalOptStatus->setText("Optimization failed");
        m_globalOptimizeButton->setEnabled(true);
        m_globalOptProgress->setVisible(false);
    }
}

void RegistrationWorkflowWidget::onAlignByFeatures() {
    if (!m_hasValidProject) {
        QMessageBox::warning(this, "Warning", "No project loaded for feature alignment");
        return;
    }

    logMessage("Starting feature-based registration...");

    // Disable UI during registration
    m_alignByFeaturesButton->setEnabled(false);
    m_featureRegProgress->setVisible(true);
    m_featureRegProgress->setValue(0);
    m_featureRegStatus->setText("Loading point clouds...");

    try {
        // Get scan list from project
        QStringList scanIds = m_currentProject.getScans();

        if (scanIds.size() < 2) {
            logMessage("ERROR: Need at least 2 scans for feature-based registration");
            m_featureRegStatus->setText("Insufficient scans");
            m_alignByFeaturesButton->setEnabled(true);
            m_featureRegProgress->setVisible(false);
            return;
        }

        // For demonstration, align first two scans
        // In a full implementation, this would iterate through all scan pairs
        QString sourceScanId = scanIds[0];
        QString targetScanId = scanIds[1];

        logMessage(QString("Aligning scans: %1 -> %2").arg(sourceScanId, targetScanId));

        // Load point clouds (placeholder - would load actual data)
        std::vector<Point3D> sourcePoints; // TODO: Load from project
        std::vector<Point3D> targetPoints; // TODO: Load from project

        // Setup registration parameters
        FeatureBasedRegistration::Parameters params;
        params.extractionParams.maxPlanes = m_maxPlanesSpin->value();
        params.extractionParams.distanceThreshold = static_cast<float>(m_planeDistanceThresholdSpin->value());
        params.extractionParams.minInliers = m_minInliersSpin->value();

        m_featureRegStatus->setText("Extracting features...");

        // Start registration (this will trigger progress signals)
        auto result = m_featureRegistration->registerPointClouds(sourcePoints, targetPoints, params);

        // The completion will be handled in onFeatureRegistrationCompleted

    } catch (const std::exception& e) {
        logMessage(QString("ERROR: Feature registration failed: %1").arg(e.what()));
        m_featureRegStatus->setText("Registration failed");
        m_alignByFeaturesButton->setEnabled(true);
        m_featureRegProgress->setVisible(false);
    }
}

void RegistrationWorkflowWidget::onShowDifferenceHeatMap(bool enabled) {
    if (!m_currentPoseGraph) {
        m_showDifferenceHeatMapCheck->setChecked(false);
        QMessageBox::information(this, "Information", "No registration results available for heat map");
        return;
    }

    logMessage(QString("Difference heat map %1").arg(enabled ? "enabled" : "disabled"));

    // TODO: Implement heat map visualization in 3D viewer
    // This would typically involve:
    // 1. Calculate point-to-point distances
    // 2. Generate color map values
    // 3. Apply colors to point cloud visualization

    if (enabled) {
        m_analysisStatus->setText("Heat map enabled");
    } else {
        m_analysisStatus->setText("Heat map disabled");
    }
}

void RegistrationWorkflowWidget::onAnalyzeRegistrationQuality() {
    if (!m_currentPoseGraph) {
        QMessageBox::warning(this, "Warning", "No registration results available for analysis");
        return;
    }

    logMessage("Starting registration quality analysis...");
    m_analysisStatus->setText("Analyzing registration quality...");

    try {
        // Setup analysis parameters
        Analysis::DifferenceAnalysis::Parameters params;
        params.maxSearchDistance = static_cast<float>(m_maxSearchDistanceSpin->value());
        params.useKDTree = m_useKDTreeCheck->isChecked();

        // TODO: Load actual point cloud data for analysis
        std::vector<Point3D> sourcePoints; // TODO: Load from project
        std::vector<Point3D> targetPoints; // TODO: Load from project

        // Calculate distances (this will trigger completion signal)
        QVector<float> distances = m_differenceAnalysis->calculateDistances(
            sourcePoints, targetPoints, QMatrix4x4(), params);

        // The completion will be handled in onDifferenceAnalysisCompleted

    } catch (const std::exception& e) {
        logMessage(QString("ERROR: Quality analysis failed: %1").arg(e.what()));
        m_analysisStatus->setText("Analysis failed");
    }
}

void RegistrationWorkflowWidget::onBundleAdjustmentProgress(int iteration, double currentError, double lambda) {
    // Update progress bar based on iteration count
    int maxIterations = m_maxIterationsSpin->value();
    int progress = (iteration * 100) / maxIterations;
    m_globalOptProgress->setValue(std::min(progress, 99)); // Keep at 99% until completion

    m_globalOptStatus->setText(QString("Iteration %1: Error=%2, λ=%3")
                              .arg(iteration)
                              .arg(currentError, 0, 'e', 3)
                              .arg(lambda, 0, 'e', 3));

    if (iteration % 10 == 0) { // Log every 10th iteration
        logMessage(QString("Bundle adjustment iteration %1: error=%2")
                  .arg(iteration).arg(currentError, 0, 'e', 3));
    }
}

void RegistrationWorkflowWidget::onBundleAdjustmentCompleted(const Optimization::BundleAdjustment::Result& result) {
    // Re-enable UI
    m_globalOptimizeButton->setEnabled(true);
    m_globalOptProgress->setVisible(false);

    if (result.converged) {
        logMessage(QString("Bundle adjustment completed successfully in %1 iterations")
                  .arg(result.iterations));
        logMessage(QString("Error reduction: %1% (from %2 to %3)")
                  .arg(result.improvementRatio * 100.0, 0, 'f', 2)
                  .arg(result.initialError, 0, 'e', 3)
                  .arg(result.finalError, 0, 'e', 3));

        m_globalOptStatus->setText(QString("Completed: %1% improvement")
                                  .arg(result.improvementRatio * 100.0, 0, 'f', 1));

        // Update current pose graph (would be set from optimization result)
        // m_currentPoseGraph = optimizedGraph;
        updateUIState();

        // Update summary
        QString summary = QString(
            "Global Optimization Results:\n"
            "• Converged: %1\n"
            "• Iterations: %2\n"
            "• Initial Error: %3\n"
            "• Final Error: %4\n"
            "• Improvement: %5%\n"
            "• Time: %6 seconds"
        ).arg(result.converged ? "Yes" : "No")
         .arg(result.iterations)
         .arg(result.initialError, 0, 'e', 3)
         .arg(result.finalError, 0, 'e', 3)
         .arg(result.improvementRatio * 100.0, 0, 'f', 2)
         .arg(result.optimizationTimeSeconds, 0, 'f', 2);

        m_summaryLabel->setText(summary);

        emit globalOptimizationCompleted(true);

    } else {
        logMessage(QString("Bundle adjustment failed: %1").arg(result.statusMessage));
        m_globalOptStatus->setText("Optimization failed");

        emit globalOptimizationCompleted(false);
    }
}

void RegistrationWorkflowWidget::onFeatureRegistrationProgress(int percentage) {
    m_featureRegProgress->setValue(percentage);

    if (percentage == 25) {
        m_featureRegStatus->setText("Extracting source features...");
    } else if (percentage == 50) {
        m_featureRegStatus->setText("Extracting target features...");
    } else if (percentage == 75) {
        m_featureRegStatus->setText("Finding correspondences...");
    } else if (percentage == 90) {
        m_featureRegStatus->setText("Computing transformation...");
    }
}

void RegistrationWorkflowWidget::onFeatureRegistrationCompleted(const FeatureBasedRegistration::Result& result) {
    // Re-enable UI
    m_alignByFeaturesButton->setEnabled(true);
    m_featureRegProgress->setVisible(false);

    if (result.success) {
        logMessage(QString("Feature-based registration completed successfully"));
        logMessage(QString("Found %1 source planes, %2 target planes, %3 correspondences")
                  .arg(result.sourcePlanesFound)
                  .arg(result.targetPlanesFound)
                  .arg(result.correspondencesFound));
        logMessage(QString("Registration quality: %1").arg(result.quality, 0, 'f', 3));

        m_featureRegStatus->setText(QString("Completed: Quality=%1")
                                   .arg(result.quality, 0, 'f', 2));

        // Update summary
        QString summary = QString(
            "Feature Registration Results:\n"
            "• Success: %1\n"
            "• Source Planes: %2\n"
            "• Target Planes: %3\n"
            "• Correspondences: %4\n"
            "• Quality Score: %5"
        ).arg(result.success ? "Yes" : "No")
         .arg(result.sourcePlanesFound)
         .arg(result.targetPlanesFound)
         .arg(result.correspondencesFound)
         .arg(result.quality, 0, 'f', 3);

        m_summaryLabel->setText(summary);

        emit featureRegistrationCompleted(true);

    } else {
        logMessage(QString("Feature-based registration failed: %1").arg(result.errorMessage));
        m_featureRegStatus->setText("Registration failed");

        emit featureRegistrationCompleted(false);
    }
}

void RegistrationWorkflowWidget::onDifferenceAnalysisCompleted(const Analysis::DifferenceAnalysis::Statistics& stats) {
    m_analysisStatus->setText("Analysis completed");

    logMessage(QString("Registration quality analysis completed"));
    logMessage(QString("Mean distance: %1m, RMS: %2m, Outliers: %3%")
              .arg(stats.meanDistance, 0, 'f', 4)
              .arg(stats.rmsDistance, 0, 'f', 4)
              .arg(stats.outlierPercentage, 0, 'f', 1));

    // Generate detailed report
    QString report = m_differenceAnalysis->generateAnalysisReport(stats);
    m_statisticsTextEdit->setPlainText(report);

    // Update summary
    float quality = m_differenceAnalysis->assessRegistrationQuality(stats);
    QString summary = QString(
        "Registration Quality Analysis:\n"
        "• Total Points: %1\n"
        "• Valid Distances: %2\n"
        "• Mean Distance: %3 m\n"
        "• RMS Distance: %4 m\n"
        "• Outliers: %5%\n"
        "• Quality Score: %6"
    ).arg(stats.totalPoints)
     .arg(stats.validDistances)
     .arg(stats.meanDistance, 0, 'f', 4)
     .arg(stats.rmsDistance, 0, 'f', 4)
     .arg(stats.outlierPercentage, 0, 'f', 1)
     .arg(quality, 0, 'f', 3);

    m_summaryLabel->setText(summary);

    // Switch to statistics tab to show results
    m_resultsTabWidget->setCurrentIndex(1);

    emit analysisCompleted(stats);
}

} // namespace Registration
