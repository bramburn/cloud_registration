# **Product Requirements Document: Sprint 5 - Target-Based Registration Workflow**

This PRD details the requirements for **Sprint 5: Target-Based
Registration Workflow**, focusing on automating registration using
predefined targets like spheres and manually selected natural points.
This sprint is a key step towards achieving the Minimum Competitive
Product (MCP) by providing an alternative to manual point-to-point
alignment and cloud-to-cloud ICP.

## Sprint 5 Overview

- **Sprint Goal:** Implement a user-friendly workflow for automatic
  target-based registration, including target detection, visualization,
  and alignment computation.

- **Context:** This sprint leverages the TargetDetectionDialog,
  SphereDetector, NaturalPointSelector, TargetManager, AlignmentEngine,
  and LeastSquaresAlignment components. It introduces UI elements for
  target detection and display, and integrates the target-based
  alignment logic into the existing workflow.

## Sub-Sprint 5.1: Target Detection UI & Mode Activation

### Goal

Provide a user interface within the RegistrationWorkflowWidget to
initiate target detection and configure its parameters.

### User Stories

- **As a user,** I want to click a button to initiate the target
  detection process on selected scans.

- **As a user,** I want to specify the type of targets to detect (e.g.,
  spheres, natural points) and adjust their detection parameters.

- **As a user,** I want the application to suggest reasonable default
  parameters for target detection.

### UI/UX

- **RegistrationWorkflowWidget**:

  - Add a new button or tab labeled \"Target Detection\" alongside
    \"Manual Alignment\" and \"Automatic Alignment (ICP)\". This button
    should be enabled when at least one scan is loaded in the project.

  - Clicking this button/tab should transition the UI to a \"Target
    Detection Configuration\" view within the RegistrationWorkflowWidget
    or display a modal dialog (TargetDetectionDialog).

- **TargetDetectionDialog (or dedicated panel in
  RegistrationWorkflowWidget)**:

  - This dialog/panel will house controls for target detection.

  - **Detection Mode/Algorithm Selection:** A QComboBox or set of radio
    buttons to choose between:

    - \"Automatic Sphere Detection\" (using SphereDetector).

    - \"Manual Natural Point Selection\" (using NaturalPointSelector).

    - \"Both\" (runs automatic, then allows manual).

  - **Parameter Inputs (using QDoubleSpinBox, QSpinBox, QCheckBox)**:

    - **Common Parameters:** Distance Threshold, Max Iterations, Min
      Quality, Enable Preprocessing.

    - **Sphere-specific Parameters:** Min Radius, Max Radius, Min
      Inliers.

    - **Natural Point-specific Parameters:** Neighborhood Radius,
      Curvature Threshold.

  - **Action Buttons:** \"Start Detection\", \"Cancel\" (to close
    dialog/return to previous step), \"Reset to Defaults\", \"Load
    Parameters\", \"Save Parameters\".

  - A status label and QProgressBar for overall detection progress.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleTargetDetectionClicked() slot, connected to the
    \"Target Detection\" UI button.

  - This slot will:

    - Determine the currently active scan(s) from the project context
      (e.g., loaded scans from PointCloudLoadManager or selected scans
      from SidebarWidget).

    - Instantiate and show the TargetDetectionDialog.

    - Pass the selected scan(s) information (IDs, point cloud data if
      available in memory) to the TargetDetectionDialog.

    - Connect TargetDetectionDialog::startDetectionRequested signal to a
      new MainPresenter::startTargetDetection() slot, passing the
      configured DetectionParams and selected scans.

- **RegistrationWorkflowWidget**:

  - Add the \"Target Detection\" button/tab.

  - Connect its clicked signal to
    MainPresenter::handleTargetDetectionClicked().

- **TargetDetectionDialog.h/.cpp**: (This component is already defined
  in src/ui/include/ui/TargetDetectionDialog.h but needs full
  implementation.)

  - Implement its setupUI() to include all required input fields.

  - Implement getDetectionParameters() to retrieve current UI values.

  - Implement setDetectionParameters() to set UI values (used for
    loading defaults/saved parameters).

  - Implement startDetection() to emit
    startDetectionRequested(DetectionMode mode, DetectionParams params,
    const QString& scanId).

  - Implement onDetectionModeChanged() to show/hide relevant parameter
    groups.

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - Introduce a new method startTargetDetection(const QString& scanId,
    TargetDetectionBase::DetectionMode mode, const
    TargetDetectionBase::DetectionParams& params). This will be the main
    entry point for the backend.

  - It will need access to PointCloudLoadManager to fetch PointFullData
    for the specified scanId.

  - It will internally manage instances of SphereDetector and
    NaturalPointSelector.

- **SphereDetector**:

  - Its DetectionParams struct and getDefaultParameters() method will be
    used by TargetDetectionDialog.

  - Its detectAsync() method will perform the actual sphere detection.

- **NaturalPointSelector**:

  - Its DetectionParams struct and getDefaultParameters() method will be
    used.

  - For manual selection, it will interact directly with
    PointCloudViewerWidget\'s mouse events (as outlined in analyse the
    previous response and produce an MCP.(1).md).

- **PointCloudLoadManager**:

  - Needs a method getLoadedPointFullData(const QString& scanId) that
    returns std::vector\<PointFullData\> for a given scan ID,
    potentially loading it if not already in memory.

### Tests

- **Unit Tests:**

  - test_mainpresenter.cpp:

    - Add tests for handleTargetDetectionClicked() verifying correct
      TargetDetectionDialog display and startTargetDetection() call.

  - test_targetdetectiondialog.cpp (create this new test file):

    - Verify correct UI layout and initial states.

    - Test onDetectionModeChanged() shows/hides parameter groups
      correctly.

    - Test resetToDefaults() sets correct default values from
      SphereDetector and NaturalPointSelector.

    - Test startDetection() emits startDetectionRequested with correct
      parameters.

    - Test validateParameters() prevents invalid input.

  - test_spheredetector.cpp: (already exists for basic detection, but
    can be expanded to cover more DetectionParams validation and
    realistic scenarios).

  - test_naturalpointselector.cpp: (similar to SphereDetector for
    DetectionParams validation and core logic).

- **Integration Tests:**

  - Extend tests/integration/automatic_alignment_e2e_test.cpp or create
    a new tests/integration/target_based_alignment_e2e_test.cpp.

  - Simulate loading a scan.

  - Simulate clicking \"Target Detection\", configuring parameters, and
    clicking \"Start Detection\".

  - Verify that AlignmentEngine::startTargetDetection() is invoked.

### Acceptance Criteria

- A \"Target Detection\" button/tab is available in the UI.

- Clicking it opens the target detection configuration dialog/panel.

- Users can select \"Automatic Sphere Detection\" or \"Manual Natural
  Point Selection\" (or \"Both\").

- Parameters specific to the chosen detection mode are visible and
  configurable.

- Default parameters are pre-filled based on the selected algorithm.

- A \"Start Detection\" button initiates the process.

## Sub-Sprint 5.2: Backend Target Detection & Management

### Goal

Implement the core logic within the AlignmentEngine to execute target
detection algorithms and store the results in the TargetManager.

### User Stories

- **As a user,** when I start automatic target detection, I want the
  system to find targets (e.g., spheres) in the specified scan(s).

- **As a user,** I want detected targets to be automatically added to a
  list for subsequent use.

- **As a user,** I want to see progress updates as targets are being
  detected.

### UI/UX

- **TargetDetectionDialog**:

  - The QProgressBar and status label will update based on signals from
    AlignmentEngine (which relays from SphereDetector).

  - The \"Start Detection\" button will change to \"Detecting\...\" and
    be disabled.

  - A \"Cancel\" button will be enabled to stop the detection.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - In startTargetDetection():

    - Connect AlignmentEngine::targetDetectionProgress to
      TargetDetectionDialog::onDetectionProgress.

    - Connect AlignmentEngine::targetDetectionCompleted to
      TargetDetectionDialog::onDetectionCompleted.

    - Connect AlignmentEngine::targetDetectionError to
      TargetDetectionDialog::onDetectionError.

    - Call AlignmentEngine::startTargetDetection() with the necessary
      parameters and scan data.

- **TargetDetectionDialog**:

  - Implement onDetectionProgress(int percentage, const QString& stage)
    to update its internal progress bar and status label.

  - Implement onDetectionCompleted(const
    TargetDetectionBase::DetectionResult& result) to enable result
    display and transition UI.

  - Implement onDetectionError(const QString& error) to display error
    messages.

  - Implement cancelDetection() to call
    AlignmentEngine::cancelTargetDetection().

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - startTargetDetection(const QString& scanId,
    TargetDetectionBase::DetectionMode mode, const
    TargetDetectionBase::DetectionParams& params):

    - Fetch std::vector\<PointFullData\> for scanId using
      PointCloudLoadManager::getLoadedPointFullData().

    - Instantiate SphereDetector (if mode includes automatic spheres).

    - Connect SphereDetector::detectionProgress to
      AlignmentEngine::targetDetectionProgress.

    - Call SphereDetector::detectAsync() (or detect() for synchronous
      testing) with the fetched points and parameters.

    - When SphereDetector::detectionCompleted emits:

      - Iterate through result.targets from SphereDetector.

      - For each Target (e.g., SphereTarget), add it to the
        TargetManager using TargetManager::addTarget(scanId, target).

      - Emit targetDetectionCompleted from AlignmentEngine.

    - If mode includes manual natural points:

      - The TargetDetectionDialog would emit a separate signal for
        manualSelectionRequested(scanId), which MainPresenter would
        handle by preparing the PointCloudViewerWidget for interactive
        point picking (covered in Sub-Sprint 5.3).

  - Implement cancelTargetDetection() to call SphereDetector::cancel()
    if detection is ongoing.

  - **Signals**: targetDetectionProgress(int percentage, const QString&
    stage), targetDetectionCompleted(const
    TargetDetectionBase::DetectionResult& result),
    targetDetectionError(const QString& error). These will be emitted by
    AlignmentEngine to MainPresenter.

- **SphereDetector**:

  - Its detectAsync() method will perform the actual RANSAC-based sphere
    detection.

  - It will emit detectionProgress and detectionCompleted signals as it
    runs.

  - It needs to be able to set an internal m_isCancelled flag via
    cancel() to stop computation early.

- **TargetManager**:

  - Its addTarget(const QString& scanId, std::shared_ptr\<Target\>
    target) method will be used to store the detected targets. (Already
    exists from Sprint 2).

### Tests

- **Unit Tests:**

  - test_alignmentengine.cpp:

    - Test startTargetDetection() correctly calls
      SphereDetector::detectAsync() and handles its result by adding
      targets to TargetManager.

    - Verify AlignmentEngine re-emits targetDetectionProgress,
      targetDetectionCompleted, targetDetectionError signals.

    - Test cancelTargetDetection() correctly propagates the cancellation
      to SphereDetector.

  - test_spheredetector.cpp:

    - Verify detectAsync() runs in a separate thread and emits correct
      progress/completion signals.

    - Test cancel() effectively stops the detection process and
      detectionCompleted(false, \...) is emitted.

  - test_targetdetectiondialog.cpp:

    - Verify onDetectionProgress and onDetectionCompleted correctly
      update the dialog\'s UI.

    - Verify cancelDetection calls
      AlignmentEngine::cancelTargetDetection().

- **Integration Tests:**

  - Simulate loading a scan, opening the TargetDetectionDialog,
    configuring sphere detection, and starting it.

  - Verify the TargetDetectionDialog\'s progress bar and status update.

  - Verify that TargetManager receives and stores the detected
    SphereTarget objects.

  - Simulate cancelling the detection mid-way and verify it stops.

### Acceptance Criteria

- Automatic target detection runs correctly on selected scans.

- Progress of detection is displayed in the UI.

- Detected targets (e.g., SphereTarget objects) are added to the
  TargetManager.

- The detection process can be cancelled by the user.

## Sub-Sprint 5.3: Viewer Integration & Target Visualization

### Goal

Visually represent detected targets in the 3D viewer and provide a
user-facing list for managing these targets.

### User Stories

- **As a user,** I want to see the detected targets (e.g., spheres,
  selected points) rendered in the 3D viewer.

- **As a user,** I want to see a list of all detected targets for the
  current scan, including their type, ID, and position.

- **As a user,** I want to click on a target in the list and see it
  highlighted in the 3D viewer.

- **As a user,** I want to be able to manually select natural points in
  the 3D viewer and have them added to the target list.

### UI/UX

- **PointCloudViewerWidget**:

  - Extend PointCloudViewerWidget\'s rendering capabilities to draw
    various Target types (e.g., SphereTarget as a wireframe sphere,
    NaturalPointTarget as a distinct point/crosshair).

  - Implement methods like drawTargets(const QList\<Target\*\>& targets)
    that takes a list of Target objects and renders them.

  - Implement highlightTarget(const QString& targetId) to visually
    distinguish a selected target.

  - Implement interaction for Manual Natural Point Selection:

    - When manualSelectionRequested is emitted, the viewer enters a
      \"point picking\" mode.

    - mousePressEvent in the viewer will call
      NaturalPointSelector::selectPoint().

    - If a point is selected, display a temporary marker and pass
      SelectedPointResult back to MainPresenter.

- **TargetDetectionDialog**:

  - **Results Table (QTableWidget)**: Populate this table with detected
    targets (Target\* objects from TargetManager). Columns should
    include: Type, ID, Position (XYZ), Quality, and potentially
    type-specific info (Radius/Size for spheres, Description for natural
    points).

  - Connect QTableWidget::currentRowChanged signal to
    onTargetSelected(int row).

  - When Manual Natural Point Selection is active:

    - A clear instruction should be displayed (e.g., \"Click on points
      in the viewer to select targets\").

    - A \"Done Manual Selection\" button should allow the user to exit
      this mode.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Listen to AlignmentEngine::targetDetectionCompleted. When a
    TargetDetectionResult is received:

    - Call TargetDetectionDialog::updateResultsTable(result).

    - Call PointCloudViewerWidget::drawTargets(targets) (passing
      TargetManager::getAllTargetsForScan(scanId)).

  - New slot handleManualSelectionRequested(const QString& scanId)
    (connected from TargetDetectionDialog):

    - Enable point picking mode in PointCloudViewerWidget (e.g.,
      viewer-\>enablePointPicking(true)).

    - Connect PointCloudViewerWidget::pointPicked(QVector3D worldPos,
      const QString& scanId) to MainPresenter::addManualTarget().

  - New slot addManualTarget(QVector3D worldPos, const QString& scanId):

    - Call NaturalPointSelector::selectClosestPoint() to refine the
      picked point.

    - Create a NaturalPointTarget object.

    - Add it to TargetManager (targetManager-\>addTarget(scanId,
      newTarget)).

    - Notify TargetDetectionDialog to update its results table.

    - Notify PointCloudViewerWidget to draw the new target.

- **TargetDetectionDialog**:

  - Implement updateResultsTable(const
    TargetDetectionBase::DetectionResult& result) to populate the
    QTableWidget.

  - Implement onTargetSelected(int row) to get the TargetId and emit a
    signal to MainPresenter to highlightTarget().

  - Implement onAcceptTargets() and onRejectTargets() to manage target
    persistence.

- **PointCloudViewerWidget**:

  - Needs new public methods: drawTargets(const QList\<Target\*\>&
    targets) and highlightTarget(const QString& targetId).

  - Needs to emit pointPicked(QVector3D worldPos, const QString& scanId)
    on mouse click when in point picking mode.

  - Must be able to receive Target objects (or their properties) for
    rendering.

### Backend (Core Logic/Services)

- **TargetManager**:

  - The getTargetsForScan(const QString& scanId) method will be used to
    retrieve targets for visualization. (Already exists).

- **Target Hierarchy**:

  - Ensure SphereTarget, CheckerboardTarget, NaturalPointTarget (already
    defined) have necessary information (position, type, visual
    properties like radius) for rendering.

### Tests

- **Unit Tests:**

  - test_pointcloudviewerwidget_rendering_r4.cpp (or a new rendering
    test file):

    - Add tests for drawTargets() ensuring various target types are
      rendered correctly.

    - Test highlightTarget() visibly changes a target\'s appearance.

    - Test pointPicked signal is emitted on mouse click when picking is
      enabled.

  - test_targetdetectiondialog.cpp:

    - Test updateResultsTable() correctly populates the table.

    - Test onTargetSelected() emits a signal with the correct target ID.

    - Test the UI flow for Manual Natural Point Selection.

  - test_mainpresenter.cpp:

    - Test handleManualSelectionRequested() correctly enables viewer
      picking.

    - Test addManualTarget() correctly adds a NaturalPointTarget to
      TargetManager.

- **Integration Tests:**

  - Simulate loading scans.

  - Simulate running automatic sphere detection.

  - Verify that detected spheres appear in the TargetDetectionDialog\'s
    table and in the 3D viewer.

  - Simulate switching to manual natural point selection mode.

  - Simulate clicking in the viewer and verify new NaturalPointTarget
    objects appear in both the viewer and the list.

  - Simulate selecting an item in the list and verifying it\'s
    highlighted in the viewer.

### Acceptance Criteria

- Detected targets are visibly rendered in the 3D viewer with distinct
  representations for different target types.

- A list of detected targets is displayed in the UI, showing relevant
  metadata.

- Selecting a target in the list highlights it in the 3D viewer.

- Users can manually select natural points in the 3D viewer, and these
  points are added to the target list and rendered.

## Sub-Sprint 5.4: Target-Based Alignment Computation

### Goal

Enable computation of rigid alignment based on detected and selected
target correspondences between two scans.

### User Stories

- **As a user,** after detecting targets in multiple scans, I want to
  automatically compute the alignment between them using these targets.

- **As a user,** I want to see a preview of the alignment in the 3D
  viewer.

- **As a user,** I want to see the quality metrics (e.g., RMS error) for
  the target-based alignment.

### UI/UX

- **TargetDetectionDialog (or a new TargetAlignmentPanel)**:

  - Once targets are detected in *at least two scans*, a \"Compute
    Target Alignment\" button should become active.

  - This button will trigger the alignment computation.

  - Display a live preview in the PointCloudViewerWidget similar to
    ICP\'s setDynamicTransform().

  - Display final RMS error and other relevant quality metrics in a
    summary section.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Add a handleComputeTargetAlignment() slot, triggered by the UI
    button.

  - This slot will:

    - Determine the source and target scans (e.g., the two most recently
      detected scans with targets, or explicitly selected by the user in
      TargetDetectionDialog).

    - Retrieve the relevant Target objects from TargetManager for both
      scans.

    - Call AlignmentEngine::computeTargetAlignment(sourceScanId,
      targetScanId, targetCorrespondences).

    - Continuously receive transformationUpdated signals from
      AlignmentEngine and update
      PointCloudViewerWidget::setDynamicTransform().

- **TargetDetectionDialog**:

  - Add \"Compute Target Alignment\" button.

  - Implement logic to enable this button when at least two scans have
    targets detected.

  - Connect the button\'s clicked signal to
    MainPresenter::handleComputeTargetAlignment().

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - Implement computeTargetAlignment(const QString& sourceScanId, const
    QString& targetScanId).

  - Inside this method:

    - Retrieve Target\* objects for sourceScanId and targetScanId from
      TargetManager.

    - Find correspondences between these targets. This requires new
      logic: findTargetCorrespondences(const QList\<Target\*\>&
      sourceTargets, const QList\<Target\*\>& targetTargets). This
      method should use logic to match targets based on their type,
      proximity, and potentially internal features (e.g., radius for
      spheres, feature descriptors for natural points). This will return
      QList\<QPair\<QVector3D, QVector3D\>\> suitable for
      LeastSquaresAlignment.

    - Call LeastSquaresAlignment::computeTransformation() with the found
      correspondences.

    - Update internal AlignmentResult and emit alignmentResultUpdated
      and transformationUpdated signals.

- **TargetManager**:

  - Ensure getTargetsForScan(const QString& scanId) works correctly.

  - Consider adding findPotentialCorrespondences(const QString& scanId1,
    const QString& scanId2) if matching logic is complex enough to live
    here.

- **LeastSquaresAlignment**: (Already implemented and functional for
  point correspondences).

### Tests

- **Unit Tests:**

  - test_alignmentengine.cpp:

    - Add tests for findTargetCorrespondences() ensuring correct
      matching logic for various target types (e.g., matches spheres by
      radius, natural points by features).

    - Test computeTargetAlignment() correctly integrates TargetManager,
      target matching, and LeastSquaresAlignment.

    - Verify transformationUpdated and alignmentResultUpdated signals
      are emitted during computation.

  - test_target.cpp: (already exists)

    - Ensure SphereTarget and NaturalPointTarget have sufficient data
      for matching (e.g., radius for spheres, possible feature
      descriptors for natural points).

- **Integration Tests:**

  - Simulate loading two scans.

  - Simulate detecting/adding targets in both scans (e.g.,
    programmatically add SphereTarget instances in TargetManager).

  - Simulate clicking \"Compute Target Alignment\".

  - Verify the PointCloudViewerWidget shows a dynamic preview of the
    alignment.

  - Verify the final RMS error and alignment status are displayed.

### Acceptance Criteria

- A \"Compute Target Alignment\" button is available and enabled when at
  least two scans with detected targets are present.

- Clicking the button initiates the target-based alignment computation.

- The 3D viewer displays a live preview of the alignment during
  computation.

- Final quality metrics (e.g., RMS error) are displayed in the UI.

## Sub-Sprint 5.5: Finalization & Workflow Integration

### Goal

Allow users to accept or discard the target-based alignment results, and
integrate this step into the overall registration workflow
(RegistrationWorkflowWidget).

### User Stories

- **As a user,** after a target-based alignment, I want to explicitly
  accept the result to apply it permanently to the scans.

- **As a user,** I want to discard the target-based alignment if I\'m
  not satisfied, reverting the scans to their state before the
  computation.

- **As a user,** upon accepting a target-based alignment, I want the
  workflow to transition to the \"Quality Review\" step.

### UI/UX

- **TargetDetectionDialog (or AlignmentControlPanel)**:

  - Once the \"Compute Target Alignment\" completes, \"Accept\" and
    \"Discard\" buttons should become enabled (similar to ICP
    finalization).

  - A summary of the target-based alignment results (e.g., final RMS,
    number of correspondences used) should be displayed.

- **PointCloudViewerWidget**:

  - Upon \"Accept\", clearDynamicTransform() is called, and the
    underlying transformed scan is rendered.

  - Upon \"Discard\", clearDynamicTransform() is called, and the scans
    revert to their state before the computation.

- **RegistrationWorkflowWidget**:

  - After accepting a target-based alignment, the workflow should
    transition to the RegistrationStep::QualityReview.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Listens to AlignmentEngine::targetAlignmentCompleted (a new signal).

  - handleTargetAlignmentCompletion(bool success, const QMatrix4x4&
    finalTransform, float finalRMSError, int correspondenceCount) slot:

    - If success is true and finalTransform is valid:

      - Update RegistrationProject::setScanTransform() for the target
        scan.

      - Create and add a RegistrationResult (algorithm:
        \"Target-Based\") to RegistrationProject.

      - Instruct PointCloudViewerWidget::clearDynamicTransform().

      - Transition RegistrationWorkflowWidget to
        RegistrationStep::QualityReview.

    - If success is false:

      - Instruct PointCloudViewerWidget::clearDynamicTransform().

      - Update UI with failure message.

      - Keep RegistrationWorkflowWidget at the current \"Target
        Detection\" step.

  - New slots handleAcceptTargetAlignment() and
    handleDiscardTargetAlignment():

    - Triggered by UI buttons.

    - handleAcceptTargetAlignment(): Calls the logic in
      handleTargetAlignmentCompletion as if success was true.

    - handleDiscardTargetAlignment(): Calls
      PointCloudViewerWidget::clearDynamicTransform() and resets
      TargetDetectionDialog to allow new attempts.

- **TargetDetectionDialog**:

  - Its \"Accept All Targets\" and \"Reject All Targets\" buttons (from
    Sub-Sprint 5.3) will be repurposed or new buttons for \"Accept
    Alignment\" and \"Discard Alignment\" will be added.

  - These buttons should be connected to
    MainPresenter::handleAcceptTargetAlignment() and
    handleDiscardTargetAlignment().

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - computeTargetAlignment() must internally store the last computed
    transformation and relevant metrics.

  - It should emit targetAlignmentCompleted(bool success, const
    QMatrix4x4& finalTransform, float finalRMSError, int
    correspondenceCount) once the computation is done.

- **RegistrationProject**:

  - setScanTransform() and addRegistrationResult() are already in place
    and will be used.

### Tests

- **Unit Tests:**

  - test_alignmentengine.cpp:

    - Verify targetAlignmentCompleted signal is emitted correctly with
      all parameters upon target-based alignment completion.

  - test_mainpresenter.cpp:

    - Test handleTargetAlignmentCompletion() correctly applies
      transformation and stores RegistrationResult on success.

    - Verify PointCloudViewerWidget::clearDynamicTransform() is called.

    - Test handleAcceptTargetAlignment() and
      handleDiscardTargetAlignment() correctly invoke the
      finalization/discard logic.

  - test_targetdetectiondialog.cpp:

    - Verify visibility and functionality of \"Accept Alignment\" and
      \"Discard Alignment\" buttons after computation.

- **Integration Tests:**

  - Extend tests/integration/target_based_alignment_e2e_test.cpp.

  - Simulate the full workflow: load scans, detect/add targets, compute
    target alignment.

  - Simulate clicking \"Accept Alignment\" and verify transformation is
    permanent and workflow moves to QualityReview.

  - Simulate clicking \"Discard Alignment\" and verify preview is
    cleared and workflow stays at the \"Target Detection\" step.

### Acceptance Criteria

- A clear summary of the target-based alignment results is presented
  after computation.

- Users can accept or discard the computed target-based alignment.

- Accepting the alignment permanently transforms the scan(s) in the
  project and stores a RegistrationResult.

- Discarding the alignment reverts the scans to their previous state and
  clears the preview.

- The workflow transitions to the QualityReview step upon successful
  acceptance.
