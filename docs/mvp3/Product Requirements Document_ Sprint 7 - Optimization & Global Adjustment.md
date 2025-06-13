# **Product Requirements Document: Sprint 7 - Optimization & Global Adjustment**

This PRD details the requirements for **Sprint 7: Optimization & Global
Adjustment**, the first sprint in Phase 3 (Production Hardening) of our
project plan. The primary goal of this sprint is to significantly
improve application performance and enable robust handling of large,
multi-scan projects through global optimization techniques.

## Sprint 7 Overview

- **Sprint Goal:** Integrate global pose graph optimization
  (BundleAdjustment) to enhance overall project accuracy and implement
  performance profiling tools to identify and address bottlenecks.

- **Context:** This sprint builds upon the successful implementation of
  individual scan registrations (manual, ICP, target-based) from
  previous sprints. It moves from pair-wise registration to a global
  optimization framework.

## Sub-Sprint 7.1: Pose Graph Construction & Basic Display

### Goal

Implement the logic to dynamically build a PoseGraph from the existing
RegistrationProject data and display a basic representation of this
graph in the UI.

### User Stories

- **As a user,** I want to visualize the connectivity of my registered
  scans (which scans are linked) to understand the global structure of
  my project.

- **As a user,** I want the system to build and display a PoseGraph
  automatically based on my existing registrations.

### UI/UX

- **SidebarWidget (or new dedicated PoseGraphViewerWidget)**:

  - A new UI element (e.g., a \"Graph View\" tab in the sidebar or a
    separate modal dialog accessible from the \"Quality\" menu) will be
    introduced.

  - This view will display a simplified 2D representation of the
    PoseGraph. Each node will represent a scan, and each edge a
    registration between scans.

  - Nodes should be labeled with scan IDs/names.

  - Edges can be color-coded or annotated with their associated RMS
    error (e.g., thicker/red for high error, thinner/green for low
    error).

  - A \"Build Pose Graph\" button (or automatic generation when project
    is loaded/registered) will trigger the graph construction.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleBuildPoseGraph() slot, triggered by a UI action.

  - Inside handleBuildPoseGraph():

    - Retrieve the current RegistrationProject instance.

    - Call Registration::PoseGraphBuilder::build(currentProject) to
      construct the PoseGraph.

    - If successful, pass the constructed PoseGraph to the
      PoseGraphViewerWidget::displayGraph().

    - Update UI status messages.

- **SidebarWidget (or PoseGraphViewerWidget.h/.cpp)**:

  - If using the sidebar, add a new tab/section for \"Pose Graph\".

  - Implement displayGraph(const Registration::PoseGraph& graph) method
    in the viewer widget to render the nodes and edges. This might
    involve a simple QGraphicsScene/QGraphicsView based visualization.

  - Nodes can be drawn as circles/rectangles, and edges as lines.

  - Implement basic interaction (e.g., clicking a node highlights it in
    the SidebarWidget\'s tree view).

### Backend (Core Logic/Services)

- **Registration::PoseGraphBuilder**:

  - build(const Project& project): This method (already exists in
    src/registration/PoseGraphBuilder.h) should be fully implemented. It
    needs to read existing RegistrationResult entries from the Project
    (or RegistrationProject) and create corresponding PoseNodes and
    PoseEdges.

  - Ensure the transformation in PoseEdge is correctly derived from
    RegistrationResult.

- **Registration::PoseGraph**:

  - Ensure addNode(), addEdge(), nodes(), edges() methods (already
    exist) are correctly implemented and support necessary data for
    visualization.

- **RegistrationProject**:

  - Confirm getRegistrationResults() provides all necessary data to
    build the PoseGraph.

### Tests

- **Unit Tests:**

  - tests/registration/test_pose_graph.cpp (new test file for explicit
    PoseGraph testing, if not already existing from Sprint 9):

    - Test PoseGraphBuilder::build() correctness with various
      RegistrationProject setups (e.g., two scans, three scans forming a
      chain, a loop closure).

    - Verify correct number of nodes and edges are created.

    - Verify PoseNode transformations and PoseEdge relative transforms
      are correctly derived from RegistrationResult.

  - test_mainpresenter.cpp:

    - Test handleBuildPoseGraph() correctly calls PoseGraphBuilder and
      passes the resulting graph to the UI component.

- **Integration Tests:**

  - Create a new integration test (e.g.,
    tests/integration/global_optimization_e2e_test.cpp).

  - Simulate loading a project with multiple registered scans (e.g.,
    manually set up RegistrationResults in a mocked
    RegistrationProject).

  - Simulate triggering \"Build Pose Graph\".

  - Verify (mocked UI verification) that the PoseGraphViewerWidget
    receives and displays the graph.

### Acceptance Criteria

- A user interface element (e.g., button, tab) is available to trigger
  pose graph visualization.

- The system constructs a PoseGraph from existing project registrations.

- A basic 2D representation of the PoseGraph (nodes for scans, lines for
  registrations) is displayed.

- The graph visualization reflects the connectivity of registered scans.

## Sub-Sprint 7.2: Bundle Adjustment Integration & Execution

### Goal

Integrate the BundleAdjustment algorithm to perform global optimization
on the PoseGraph and display its execution progress and final impact on
accuracy.

### User Stories

- **As a user,** I want to optimize the overall accuracy of my
  multi-scan project using BundleAdjustment to minimize accumulated
  errors.

- **As a user,** I want to see the progress of the BundleAdjustment
  algorithm (e.g., iterations, error reduction) and be able to cancel
  it.

- **As a user,** I want to see how BundleAdjustment improved the overall
  registration accuracy (initial vs. final error).

### UI/UX

- **PoseGraphViewerWidget (or Quality Menu)**:

  - Add a \"Run Bundle Adjustment\" button. This button should be
    enabled when a PoseGraph with at least 3 nodes and 2 edges is
    available.

  - Clicking this button will launch a BundleAdjustmentProgressDialog
    (similar to ICPProgressWidget).

- **BundleAdjustmentProgressDialog (New UI Component)**:

  - Displays progress of BundleAdjustment: current iteration,
    initial/current/final error, convergence status, elapsed time.

  - Includes a \"Cancel\" button.

- **PointCloudViewerWidget**:

  - Optionally, the viewer could show a subtle preview of the globally
    adjusted poses during BundleAdjustment execution (e.g., by
    dynamically updating the transform for each scan in the viewer).

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleRunBundleAdjustment() slot.

  - Inside handleRunBundleAdjustment():

    - Retrieve the current PoseGraph (from PoseGraphBuilder).

    - Instantiate BundleAdjustmentProgressDialog and show it.

    - Connect Optimization::BundleAdjustment::optimizationProgress to
      BundleAdjustmentProgressDialog::updateProgress.

    - Connect BundleAdjustmentProgressDialog::cancelRequested to
      Optimization::BundleAdjustment::cancel().

    - Call Optimization::BundleAdjustment::optimize(currentPoseGraph).

    - Upon optimizationCompleted:

      - Update the RegistrationProject with the new, optimized scan
        poses (iterating through optimizedGraph-\>nodes() and calling
        RegistrationProject::setScanTransform() for each).

      - Update AlignmentEngine with the final error statistics from
        BundleAdjustment to reflect global accuracy.

      - Update the PoseGraphViewerWidget to reflect the new, optimized
        graph geometry (if applicable).

- **BundleAdjustmentProgressDialog.h/.cpp (New UI Component)**:

  - Basic UI: QProgressBar, QLabels for iteration, error, time,
    \"Cancel\" QPushButton.

  - startMonitoring(BundleAdjustment\* optimizer, int maxIterations):
    Connects to BundleAdjustment signals.

  - updateProgress(int iteration, double currentError, double lambda):
    Updates UI.

  - onComputationFinished(const BundleAdjustment::Result& result):
    Displays final summary.

  - Emits cancelRequested().

### Backend (Core Logic/Services)

- **Optimization::BundleAdjustment**:

  - optimize(const Registration::PoseGraph& initialGraph, const
    Parameters& params): (Already exists in
    src/optimization/BundleAdjustment.h). Needs full implementation.

  - It should internally emit optimizationProgress(int iteration, double
    currentError, double lambda) during its loop.

  - It should return the optimizedGraph and Result struct upon
    completion.

  - It needs a cancel() method to allow premature termination.

- **Registration::PoseGraphBuilder**:

  - The build() method must be reliable to provide the input graph for
    optimization.

- **RegistrationProject**:

  - setScanTransform(const QString& scanId, const QMatrix4x4& transform)
    is crucial to apply the new poses.

### Tests

- **Unit Tests:**

  - tests/optimization/test_bundle_adjustment.cpp (new test file for
    explicit BundleAdjustment testing, if not already existing from
    Sprint 9):

    - Test optimize() method with various synthetic pose graphs (e.g.,
      chain, loop closures) to verify convergence and accuracy.

    - Verify optimizationProgress signal is emitted correctly with
      expected data.

    - Test cancel() functionality, ensuring the optimization stops
      prematurely.

  - test_bundleadjustmentprogressdialog.cpp (new test file):

    - Verify UI updates correctly with progress.

    - Test cancelRequested signal emission.

  - test_mainpresenter.cpp:

    - Test handleRunBundleAdjustment() correctly launches dialog,
      connects signals, and triggers BundleAdjustment::optimize().

    - Verify RegistrationProject::setScanTransform() is called for each
      optimized scan.

- **Integration Tests:**

  - Extend tests/integration/global_optimization_e2e_test.cpp.

  - Simulate building a PoseGraph.

  - Simulate clicking \"Run Bundle Adjustment\", observing (mocked)
    progress updates.

  - Verify the final, optimized scan poses are applied to the
    RegistrationProject.

  - Verify the overall project accuracy is improved (if quantifiable in
    test data).

### Acceptance Criteria

- A \"Run Bundle Adjustment\" button is available in the UI.

- A progress dialog displays real-time BundleAdjustment progress.

- Users can cancel the BundleAdjustment process.

- Upon completion, the application applies the optimized poses to the
  project\'s scans.

- The PoseGraph visualization can optionally be updated to reflect the
  optimized graph.

## Sub-Sprint 7.3: Performance Profiling Integration & Reporting

### Goal

Integrate the PerformanceProfiler into key application workflows to
automatically identify and report performance bottlenecks.

### User Stories

- **As a developer/advanced user,** I want the application to passively
  collect performance metrics during critical operations so I can
  diagnose bottlenecks.

- **As a user,** I want to generate a performance report that summarizes
  the execution times of different application modules.

### UI/UX

- **File Menu (or Debug Menu)**:

  - Add a new QAction labeled \"Generate Performance Report\...\". This
    action is primarily for developers or advanced users.

- **PerformanceProfiler**:

  - No direct UI in this sprint. The generateReport() method will output
    to a file or debug console.

- **Status Bar (Optional)**:

  - A small status bar indicator could show if profiling is active.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleGeneratePerformanceReportClicked() slot, connected
    to the menu action.

  - Inside handleGeneratePerformanceReportClicked():

    - Call PerformanceProfiler::instance().generateReport(filePath).

    - Prompt user for a save location using
      IMainView::askForSaveFilePath().

    - Display success/failure message.

- **MainWindow (as IMainView)**:

  - Add \"Generate Performance Report\...\" action to the menu.

### Backend (Core Logic/Services)

- **PerformanceProfiler**:

  - The startSection(), endSection(), generateReport() (already exists
    in src/core/performance_profiler.h) should be fully implemented and
    robust.

  - **Instrumentation**: Key performance-critical code paths from
    previous sprints (E57ParserLib::performParsing(),
    LASParser::parse(), ICPRegistration::compute(),
    LeastSquaresAlignment::computeTransformation(),
    DifferenceAnalysis::calculateDistances(),
    FeatureExtractor::extractPlanes(), BundleAdjustment::optimize(),
    PointCloudViewerWidget::loadPointCloud(),
    PointCloudViewerWidget::paintGL()) should be instrumented with
    PROFILE_SECTION() or PROFILE_FUNCTION() macros.

    - This requires modifying existing source files.

- **UserPreferences**:

  - Add a new boolean preference advanced/profilingEnabled to control
    whether PerformanceProfiler is active.
    PerformanceProfiler::instance().setEnabled() will check this
    preference.

### Tests

- **Unit Tests:**

  - tests/core/test_performance_profiler.cpp:

    - Verify generateReport() outputs correctly to file/console.

    - Test that setEnabled() correctly activates/deactivates profiling
      without crashing.

    - Test the PROFILE_SECTION and PROFILE_FUNCTION macros actually
      record data.

  - test_mainpresenter.cpp:

    - Test handleGeneratePerformanceReportClicked() correctly calls
      PerformanceProfiler::generateReport() and handles file saving.

- **Integration Tests:**

  - Extend tests/integration/global_optimization_e2e_test.cpp or create
    a new tests/integration/performance_profiling_e2e_test.cpp.

  - Simulate a full workflow (e.g., loading large scans, performing
    registration).

  - Before starting, enable profiling via
    UserPreferences::instance().setBool(\"advanced/profilingEnabled\",
    true).

  - At the end, simulate clicking \"Generate Performance Report\".

  - Verify (mocked file system verification) that a performance report
    file is generated and contains meaningful data (e.g., non-zero times
    for profiled sections).

### Acceptance Criteria

- PerformanceProfiler is integrated into core application workflows.

- A \"Generate Performance Report\" menu action is available.

- The generated report provides measurable data on execution times of
  instrumented code sections.

- Profiling can be enabled/disabled via user preferences.
