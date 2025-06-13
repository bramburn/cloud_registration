# **Product Requirements Document: Sprint 4 - Cloud-to-Cloud Registration Workflow**

This PRD details the requirements for **Sprint 4: Cloud-to-Cloud
Registration Workflow**, the next sprint in our project plan, moving
towards the Minimum Competitive Product (MCP). The overarching goal of
this sprint is to integrate the existing Iterative Closest Point (ICP)
algorithms into a user-friendly \"Automatic Alignment\" workflow,
allowing users to perform automated registration based on point cloud
overlap.

## Sprint 4 Overview

- **Sprint Goal:** Enable users to perform automatic cloud-to-cloud
  registration using ICP, display real-time progress, and provide clear
  results.

- **Context:** This sprint builds upon the foundational manual alignment
  MVP (Sprints 1-3). It leverages the existing PointToPlaneICP and
  LeastSquaresAlignment algorithms, along with the AlignmentEngine, to
  create an automated workflow. The ICPProgressWidget will be integrated
  for real-time feedback.

## Sub-Sprint 4.1: ICP Mode Activation & Parameter Selection

### Goal

Enable users to activate the automatic ICP alignment mode and configure
its initial parameters through a dedicated UI.

### User Stories

- **As a user,** I want to click a button to initiate the automatic ICP
  alignment workflow so that the system can attempt to register my scans
  without manual point picking.

- **As a user,** I want to be able to set parameters like maximum
  iterations and convergence threshold before running ICP to control the
  alignment process.

- **As a user,** I want the system to suggest reasonable default
  parameters for ICP so that I can quickly start an alignment.

### UI/UX

- **RegistrationWorkflowWidget (or AlignmentControlPanel)**:

  - A new prominent button or tab labeled \"Automatic Alignment (ICP)\"
    should be added. This button should be enabled when at least two
    scans are selected in the project (one source, one target/reference,
    or simply two scans in the viewer).

  - Clicking this button should transition the UI to an \"ICP
    Configuration\" view or display a modal dialog (ICPParameterDialog)
    that allows users to adjust ICP parameters.

  - The dialog/panel should include input fields for:

    - Max Iterations (e.g., QSpinBox, default 50-100)

    - Convergence Threshold (e.g., QDoubleSpinBox, default 1e-5 to 1e-6
      meters)

    - Max Correspondence Distance (e.g., QDoubleSpinBox, default 0.1-1.0
      meters, based on scan overlap)

    - Checkbox for Outlier Rejection (default checked)

    - Numeric input for Outlier Threshold (e.g., QDoubleSpinBox, default
      2.0-3.0 standard deviations)

    - Initial Coarse Alignment: A dropdown or checkbox to use \"Scanner
      Origin\" or \"Bounding Box Center\" as initial guess.

  - \"Run ICP\" and \"Cancel\" buttons within the configuration
    dialog/panel.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleAutomaticAlignmentClicked() slot, connected to the
    \"Automatic Alignment (ICP)\" UI button.

  - This slot will determine the source and target scans from the
    current project context (e.g., currently viewed scans, or selected
    scans from the SidebarWidget).

  - If necessary, instantiate and show the ICPParameterDialog.

  - Connect ICPParameterDialog::runICPRequested signal to a new
    MainPresenter::startICPComputation() slot, passing the configured
    ICPParams.

- **RegistrationWorkflowWidget**:

  - Add the \"Automatic Alignment (ICP)\" button.

  - Connect its clicked signal to
    MainPresenter::handleAutomaticAlignmentClicked().

  - May need to manage which sub-panel is active (ManualAlignment or
    ICPConfiguration).

- **New/Modified UI Component (ICPParameterDialog.h/.cpp or extend
  AlignmentControlPanel)**:

  - Create a dedicated dialog or section for ICP parameter input using
    QSpinBox, QDoubleSpinBox, QCheckBox.

  - Expose a method to retrieve configured ICPParams.

  - Emit a runICPRequested(ICPParams params) signal when the \"Run ICP\"
    button is clicked.

  - Implement logic to retrieve recommended default parameters from
    ICPRegistration (via AlignmentEngine).

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - Add startAutomaticAlignment(const QString& sourceScanId, const
    QString& targetScanId, const ICPParams& params) method. This method
    will fetch point clouds for the specified scans, prepare the
    ICPRegistration instance, and initiate the compute() call.

  - It will need access to PointCloudLoadManager to retrieve the
    PointCloud data for the specified scans.

  - It should also have a method to get recommended ICP parameters based
    on input scan sizes/characteristics.

- **ICPRegistration**:

  - Ensure its ICPParams struct supports all required parameters.

  - Add a getRecommendedParameters(const PointCloud& source, const
    PointCloud& target) static method that returns ICPParams based on
    scan properties (e.g., density, bounding box size).

- **PointCloudLoadManager**:

  - Needs a method getLoadedPointCloud(const QString& scanId) that
    returns a PointCloud object for a given scan ID, potentially loading
    it if not already in memory.

### Tests

- **Unit Tests:**

  - test_mainpresenter.cpp: Add tests for
    handleAutomaticAlignmentClicked():

    - Verify correct display of ICPParameterDialog.

    - Verify startICPComputation() is called with correct parameters.

  - test_icpprogresswidget.cpp: (already exists for basic progress, but
    can be expanded)

    - Test parameter dialog default values and user input handling.

    - Test ICPParameterDialog::runICPRequested signal emission.

  - test_icpregistration.cpp:

    - Test getRecommendedParameters() logic to ensure it provides
      sensible defaults.

    - Test that ICPRegistration::compute() is called with the expected
      PointCloud data and initial guess.

- **Integration Tests:**

  - Extend tests/integration/manual_alignment_workflow_e2e_test.cpp or
    create a new tests/integration/automatic_alignment_e2e_test.cpp.

  - Simulate loading two scans.

  - Simulate clicking \"Automatic Alignment (ICP)\" button and
    interacting with the ICPParameterDialog to configure and run ICP.

  - Verify that AlignmentEngine::startAutomaticAlignment() is invoked.

### Acceptance Criteria

- An \"Automatic Alignment (ICP)\" button is available in the UI.

- Clicking the button opens an ICP parameter configuration interface.

- Users can set max iterations, convergence threshold, max
  correspondence distance, and toggle outlier rejection with its
  threshold.

- Recommended default parameters are provided in the UI.

- A \"Run ICP\" button initiates the ICP computation.

## Sub-Sprint 4.2: ICP Execution & Live Progress

### Goal

Execute the PointToPlaneICP algorithm using the configured parameters
and display its real-time progress to the user through a dedicated
progress widget.

### User Stories

- **As a user,** when I start ICP, I want to see a live progress bar
  indicating the current iteration and overall completion percentage.

- **As a user,** I want to see the current Root Mean Square (RMS) error
  and how it changes with each iteration to understand convergence.

- **As a user,** I want to be able to cancel the ongoing ICP computation
  if it\'s taking too long or not converging as expected.

### UI/UX

- **ICPProgressWidget**:

  - This widget (already existing in
    src/ui/include/ui/ICPProgressWidget.h) should become visible when
    ICP computation starts.

  - It must display:

    - A title \"ICP Registration in Progress\".

    - Current iteration number (e.g., \"Iteration: 15 / 50\").

    - Current RMS error (e.g., \"RMS Error: 0.005 m\").

    - A progress bar reflecting overall iteration completion.

    - An \"Elapsed Time\" counter.

    - A \"Cancel\" button.

- **PointCloudViewerWidget**:

  - During ICP iterations, the PointCloudViewerWidget should
    continuously update the transformed source scan based on the
    intermediate transformation provided by ICPRegistration. This means
    setDynamicTransform() will be called multiple times.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - In startICPComputation() (triggered from Sub-Sprint 4.1):

    - Instantiate ICPProgressWidget and make it visible.

    - Connect ICPRegistration::progressUpdated (emitted by
      AlignmentEngine) to ICPProgressWidget::updateProgress.

    - Connect ICPProgressWidget::cancelRequested to
      AlignmentEngine::cancelAutomaticAlignment().

    - Pass the source and target PointCloud data to
      AlignmentEngine::startAutomaticAlignment().

- **ICPProgressWidget**:

  - Ensure its updateProgress(int iteration, float rmsError, const
    QMatrix4x4& transformation) slot correctly updates its labels and
    progress bar.

  - Ensure its onCancelClicked() slot emits cancelRequested().

- **PointCloudViewerWidget**:

  - The setDynamicTransform() method (already existing from Sprint 4:
    analyse the previous response and produce an MCP.(1).md) will be
    called repeatedly by MainPresenter as ICPRegistration emits
    intermediate transformations.

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - startAutomaticAlignment() will internally create and manage an
    instance of PointToPlaneICP.

  - It will connect its own signals (progressUpdated,
    computationFinished) directly from the internal PointToPlaneICP
    instance.

  - Implement cancelAutomaticAlignment() which will call
    m_icpAlgorithm-\>cancel().

- **PointToPlaneICP (inherits ICPRegistration)**:

  - Its compute() method must emit progressUpdated(iteration, rmsError,
    currentTransformation) in each iteration.

  - Its cancel() method must set an internal flag (m_isCancelled) that
    causes the computation loop to break.

  - It will need to transform the source point cloud with the
    currentTransformation in each iteration before finding
    correspondences to ensure proper iterative refinement.

### Tests

- **Unit Tests:**

  - test_icpregistration.cpp:

    - Add tests to verify progressUpdated signal is emitted correctly
      during iteration, including iteration, rmsError, and
      transformation parameters.

    - Add tests for cancel() functionality, ensuring the compute()
      method stops prematurely and computationFinished(false, \...) is
      emitted.

  - test_alignmentengine.cpp:

    - Verify startAutomaticAlignment() correctly initializes and starts
      PointToPlaneICP.

    - Verify cancelAutomaticAlignment() correctly calls the ICP
      instance\'s cancel method.

  - test_icpprogresswidget.cpp:

    - Verify updateProgress() correctly updates all UI elements (labels,
      progress bar).

    - Test that onCancelClicked() emits cancelRequested().

- **Integration Tests:**

  - Simulate triggering ICP.

  - Verify ICPProgressWidget becomes visible.

  - Verify ICPProgressWidget receives and displays progress updates (at
    least a few iterations).

  - Simulate clicking \"Cancel\" in ICPProgressWidget and verify that
    ICPRegistration::cancel() is called and the process terminates
    gracefully.

### Acceptance Criteria

- The ICPProgressWidget appears and displays real-time progress during
  ICP computation.

- The progress display includes current iteration, RMS error, and
  elapsed time.

- The 3D viewer dynamically updates to show the intermediate alignment
  during ICP iterations.

- Users can successfully cancel an ongoing ICP process.

## Sub-Sprint 4.3: ICP Result Application & Workflow Progression

### Goal

Apply the final ICP transformation to the project\'s scan poses, display
comprehensive quality metrics for the automatic alignment, and enable
the user to accept or discard the result, moving the workflow forward.

### User Stories

- **As a user,** after ICP completes, I want to see a clear summary of
  the alignment quality (e.g., final RMS error, convergence status).

- **As a user,** I want to be able to accept the ICP-computed alignment,
  making it permanent in the project.

- **As a user,** I want to be able to discard the ICP result if I\'m not
  satisfied, returning to the previous state without applying changes.

- **As a user,** upon accepting the ICP result, I want the workflow to
  transition to the \"Quality Review\" step.

### UI/UX

- **ICPProgressWidget**:

  - Upon computationFinished, the widget should transition to a
    \"Completed\" state or close itself, showing a summary of the result
    (e.g., \"ICP Completed Successfully\", \"Final RMS: X.XXX m\").

  - If it remains open, it should transform into a summary dialog.

- **AlignmentControlPanel**:

  - The \"Quality Metrics\" section should update to reflect the final
    ICP result (RMS error, number of iterations, convergence message).

  - Buttons to \"Accept\" and \"Discard\" the ICP result should
    appear/become enabled. These might be new dedicated ICP-specific
    buttons or the existing \"Accept/Cancel\" buttons from manual
    alignment are re-purposed.

- **PointCloudViewerWidget**:

  - Upon acceptance, clearDynamicTransform() is called, and the
    underlying transformed scan is rendered.

  - Upon discard, clearDynamicTransform() is called, and the scans
    revert to their state before the ICP run.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Listens to ICPRegistration::computationFinished (via
    AlignmentEngine).

  - handleICPCompletion(bool success, const QMatrix4x4& finalTransform,
    float finalRMSError, int iterations) slot:

    - If success is true and finalTransform is valid:

      - Update RegistrationProject::setScanTransform() using
        finalTransform.

      - Create a RegistrationResult object (source/target scans,
        finalTransform, finalRMSError, \"ICP\" as algorithm, iterations,
        timestamp).

      - Call RegistrationProject::addRegistrationResult().

      - Instruct PointCloudViewerWidget::clearDynamicTransform().

      - Transition RegistrationWorkflowWidget to
        RegistrationStep::QualityReview.

    - If success is false (e.g., cancelled or failed to converge):

      - Instruct PointCloudViewerWidget::clearDynamicTransform().

      - Update AlignmentControlPanel with failure message.

      - Keep RegistrationWorkflowWidget at the current step
        (ICPRegistration).

  - New slots handleAcceptICPResult() and handleDiscardICPResult():

    - handleAcceptICPResult(): Triggers the logic for applying the last
      computed ICP result to RegistrationProject.

    - handleDiscardICPResult(): Clears the dynamic transform and resets
      the UI state, allowing the user to try again or return to a
      previous workflow step.

- **ICPProgressWidget**:

  - Its onComputationFinished() slot should handle closing the widget or
    transitioning to a summary view. It should emit computationCompleted
    to MainPresenter.

- **AlignmentControlPanel**:

  - Needs to update its display based on the final
    AlignmentEngine::AlignmentResult (which now holds ICP results).

  - Add \"Accept\" and \"Discard\" buttons for ICP results. These
    buttons should connect to MainPresenter::handleAcceptICPResult() and
    handleDiscardICPResult() respectively.

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - The computeAutomaticAlignment() method (or similar) will encapsulate
    the PointToPlaneICP execution and its results. It needs to store the
    last successful ICPRegistration::Result internally so that
    MainPresenter can retrieve it when \"Accept\" is clicked.

  - It should also provide a method to retrieve the last computed ICP
    result regardless of whether it was successful.

- **RegistrationProject**:

  - setScanTransform() and addRegistrationResult() (from Sprint 3.1) are
    ready for use.

### Tests

- **Unit Tests:**

  - test_icpregistration.cpp:

    - Verify computationFinished signal carries success status,
      finalTransformation, finalRMSError, and iterations.

  - test_mainpresenter.cpp:

    - Test handleICPCompletion():

      - Verify RegistrationProject::setScanTransform() and
        addRegistrationResult() are called on success.

      - Verify PointCloudViewerWidget::clearDynamicTransform() is called
        in both success and failure cases.

      - Verify RegistrationWorkflowWidget transitions correctly.

    - Test handleAcceptICPResult() and handleDiscardICPResult() to
      ensure they correctly trigger the finalization or discard logic.

  - test_alignmentcontrolpanel.cpp:

    - Verify the display of final ICP quality metrics.

    - Test the enabled state and clicked signals of the \"Accept\" and
      \"Discard\" buttons.

- **Integration Tests:**

  - Extend tests/integration/automatic_alignment_e2e_test.cpp.

  - Simulate triggering ICP and allowing it to complete.

  - Verify the final result is displayed correctly in the UI.

  - Simulate clicking \"Accept\" and verify the workflow progresses to
    \"Quality Review\" and the transformation is stored.

  - Simulate clicking \"Discard\" and verify the workflow remains in the
    ICP step and the preview is cleared.

### Acceptance Criteria

- Upon ICP completion, a clear summary of the alignment quality is
  presented to the user.

- Users can accept the ICP result, making it permanent in the project
  and progressing the workflow.

- Users can discard the ICP result, removing the previewed
  transformation.

- The 3D viewer reflects the accepted transformation permanently, or
  reverts to the previous state if discarded.

- The workflow transitions to the QualityReview step after successful
  acceptance.
