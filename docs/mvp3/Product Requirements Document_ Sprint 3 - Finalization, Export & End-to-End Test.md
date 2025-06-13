# **Product Requirements Document: Sprint 3 - Finalization, Export & End-to-End Test**

This PRD details the requirements for **Sprint 3: Finalization, Export &
End-to-End Test**, which is the third and final sprint in Phase 1 (MVP
Delivery) of our project plan. The overarching goal of this sprint is to
allow users to commit or discard their manual alignment, export the
registered point clouds, and ensure the entire MVP workflow is
thoroughly tested.

## Sprint 3 Overview

- **Sprint Goal:** Enable users to finalize manual alignments, export
  results, and establish a comprehensive end-to-end test for the MVP
  workflow.

- **Context:** This sprint builds directly on Sprint 1\'s point
  selection and Sprint 2\'s alignment computation/preview. It completes
  the core user story: \"As a user, I want to load two scans, manually
  align them, and export the result.\"

## Sub-Sprint 3.1: Finalization (Accept/Cancel Alignment)

### Goal

Enable users to explicitly accept the computed alignment, applying it
permanently to the project\'s scan poses, or to cancel it, reverting to
the previous state.

### User Stories

- **As a user,** I want to click an \"Accept\" button to commit the
  previewed alignment so that the scans are permanently registered in my
  project session.

- **As a user,** I want to click a \"Cancel\" button to discard the
  previewed alignment and return to the point selection step without
  applying any changes.

- **As a user,** I want clear visual feedback confirming whether the
  alignment was accepted or cancelled.

### UI/UX

- **RegistrationWorkflowWidget (or AlignmentControlPanel)**:

  - Add \"Accept\" and \"Cancel\" buttons, logically positioned near the
    \"Compute Alignment\" button or at the bottom of the alignment
    panel.

  - The \"Accept\" button should only be enabled when a valid alignment
    has been computed and is being previewed (i.e.,
    AlignmentEngine::AlignmentState::Valid).

  - The \"Cancel\" button should always be available once the \"Manual
    Alignment\" mode is active.

- **PointCloudViewerWidget**:

  - Upon \"Accept,\" the dynamicTransform should be cleared, and the
    underlying scan data (or its stored transformation in the
    RegistrationProject) should be updated. The viewer should then
    render the permanently transformed scan.

  - Upon \"Cancel,\" the dynamicTransform should be cleared, and the
    viewer should revert to displaying the scans in their pre-alignment
    state.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleAcceptAlignment() and handleCancelAlignment() slots,
    triggered by UI button clicks.

  - handleAcceptAlignment():

    - Retrieve the TransformationMatrix from
      AlignmentEngine::getCurrentResult().

    - Call AlignmentEngine::clearCorrespondences() to reset the current
      alignment session.

    - Call RegistrationProject::setScanTransform() to update the target
      scan\'s pose with the new transformation. This will also update
      the ScanInfo in RegistrationProject.

    - Call RegistrationProject::addRegistrationResult() to store a new
      RegistrationResult indicating the successful manual alignment.

    - Instruct PointCloudViewerWidget to clear its dynamicTransform.

    - Transition the RegistrationWorkflowWidget to the next logical step
      (e.g., QualityReview).

  - handleCancelAlignment():

    - Call AlignmentEngine::clearCorrespondences().

    - Instruct PointCloudViewerWidget to clear its dynamicTransform.

    - Transition the RegistrationWorkflowWidget back to the
      PointSelection step.

- **AlignmentControlPanel**:

  - Add QPushButton instances for \"Accept\" and \"Cancel\".

  - Connect clicked signals to MainPresenter slots.

  - Manage button enabled states based on
    AlignmentEngine::AlignmentState updates.

- **RegistrationWorkflowWidget**:

  - Ensure its state machine (WorkflowStateMachine) correctly handles
    transitions based on \"Accept\" (to QualityReview) and \"Cancel\"
    (to PointSelection).

  - Listen to MainPresenter for completion/cancellation signals to
    update its own state and potentially navigate tabs.

- **PointCloudViewerWidget**:

  - Expose a clearDynamicTransform() method (already present, used in
    Sprint 2) to reset the preview transformation.

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - Ensure clearCorrespondences() correctly resets its internal state
    and m_currentResult.

  - Its current m_currentResult.transformation holds the most recent
    previewed transformation.

- **RegistrationProject**:

  - Modify setScanTransform(const QString& scanId, const QMatrix4x4&
    transform) to update the transform field of the ScanInfo struct
    within its internal scans\_ map. This is crucial for persisting the
    new pose.

  - Implement addRegistrationResult(const RegistrationResult& result) to
    store the finalized alignment. This RegistrationResult struct should
    include: sourceScanId, targetScanId, transformation, rmsError,
    correspondenceCount, isValid, algorithm (e.g., \"Manual\"), and
    timestamp.

- **ProjectTreeModel**:

  - When RegistrationProject updates a scan\'s transform, the
    ProjectTreeModel will need to be notified to reflect these changes
    if it displays scan transforms.

### Tests

- **Unit Tests:**

  - test_alignmentengine.cpp: Add tests for clearCorrespondences()
    ensuring complete state reset.

  - test_registrationproject.cpp:

    - Test setScanTransform() to verify ScanInfo::transform is updated.

    - Test addRegistrationResult() to ensure RegistrationResult objects
      are correctly stored and retrieved.

- **Integration Tests:**

  - Simulate loading scans, picking points, computing alignment, and
    then clicking \"Accept\". Verify:

    - PointCloudViewerWidget::clearDynamicTransform() is called.

    - RegistrationProject::setScanTransform() is called with the correct
      parameters.

    - RegistrationProject::addRegistrationResult() is called with a
      valid RegistrationResult.

    - The UI transitions to the QualityReview step.

  - Simulate loading scans, picking points, computing alignment, and
    then clicking \"Cancel\". Verify:

    - PointCloudViewerWidget::clearDynamicTransform() is called.

    - The UI transitions back to the PointSelection step.

### Acceptance Criteria

- The UI provides \"Accept\" and \"Cancel\" buttons for the manual
  alignment.

- The \"Accept\" button is enabled only when a valid alignment preview
  is active.

- Clicking \"Accept\" applies the transformation permanently to the
  target scan within the project data.

- A RegistrationResult record is created and stored in the
  RegistrationProject.

- Clicking \"Cancel\" discards the preview and reverts to the point
  selection state.

- In both cases (\"Accept\" and \"Cancel\"), the live preview
  (dynamicTransform) is cleared.

- The UI transitions to the appropriate next step (QualityReview for
  Accept, PointSelection for Cancel).

## Sub-Sprint 3.2: Export Functionality Integration

### Goal

Integrate the existing PointCloudExporter into the MVP workflow,
allowing users to export the current registered point cloud.

### User Stories

- **As a user,** I want to click an \"Export\" button or menu item to
  save the currently loaded and registered point cloud to a file.

- **As a user,** I want to select the output format (e.g., E57, LAS,
  XYZ) and configure basic export options (e.g., include
  color/intensity).

- **As a user,** I want to see a progress indicator during the export
  process.

### UI/UX

- **Menu/Toolbar**: Add an \"Export Point Cloud\...\" action to the
  \"File\" menu (already outlined in mainwindow.h as
  m_exportPointCloudAction). This action should be enabled when a point
  cloud is loaded and registered.

- **ExportDialog**: The ExportDialog (already existing in Sprint 6)
  should be launched when the export action is triggered. It will guide
  the user through format and options selection.

- **Progress**: The application\'s main progress dialog/status bar
  should display export progress.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement a handleExportPointCloud() slot, triggered by the \"Export
    Point Cloud\...\" menu action.

  - Inside handleExportPointCloud():

    - Obtain the current point cloud data from
      IPointCloudViewer::getCurrentPointCloudData(). This method already
      exists in PointCloudViewerWidget and returns std::vector\<Point\>.

    - Instantiate and configure the ExportDialog.

    - Pass the std::vector\<Point\> data to the ExportDialog.

    - Connect ExportDialog::exportRequested signal to
      PointCloudExporter::exportPointCloudAsync().

    - Show the ExportDialog modally.

- **ExportDialog**:

  - Ensure it can accept std::vector\<Point\> and correctly populate its
    options (e.g., estimated size).

  - Connect its internal \"Export\" button to emit exportRequested
    signal with configured ExportOptions.

- **MainWindow (as IMainView)**:

  - Ensure m_exportPointCloudAction is enabled/disabled appropriately
    based on m_viewer-\>hasPointCloudData() and if a registration has
    been accepted.

  - Its onExportCompleted slot (already defined) will be called by
    PointCloudExporter to show final messages.

### Backend (Core Logic/Services)

- **IPointCloudViewer**:

  - Ensure getCurrentPointCloudData() accurately provides the
    *transformed* and *current* point cloud data (i.e., applies the last
    accepted transformation if one exists). This means
    PointCloudViewerWidget\'s internal data representation needs to
    reflect the current project state (the ScanInfo::transform values).

- **PointCloudExporter**:

  - The exportPointCloudAsync method and its associated logic (writing
    to various formats, handling progress) are already present from
    Sprint 6. No new backend logic is strictly required for its core
    functionality, but it must be correctly invoked by the frontend.

### Tests

- **Unit Tests:**

  - No new core unit tests for PointCloudExporter itself (they are
    already in tests/Sprint6Test.cpp).

  - test_mainpresenter.cpp: Add a test for handleExportPointCloud() that
    mocks IPointCloudViewer::getCurrentPointCloudData() and ExportDialog
    interactions, verifying that the export process is correctly
    initiated.

- **Integration Tests:**

  - Simulate a full manual alignment workflow (load, pick, compute,
    accept).

  - Then, click the \"Export\" menu item, interact with the ExportDialog
    (mocking user input), and verify that
    PointCloudExporter::exportPointCloudAsync() is called with
    appropriate data and options.

### Acceptance Criteria

- An \"Export Point Cloud\" menu item is available and correctly
  enabled/disabled.

- Clicking \"Export Point Cloud\" opens the ExportDialog.

- The ExportDialog displays relevant information about the point cloud
  to be exported.

- Export options selected in the dialog are correctly passed to
  PointCloudExporter.

- The PointCloudExporter successfully exports the currently transformed
  point cloud data to the selected format.

- Progress feedback is provided during export.

- Success/failure messages are displayed upon export completion.

## Sub-Sprint 3.3: Project Tree Model Update & Scan Grouping

### Goal

Visually represent registered scans in the project tree view, showing
them as a grouped \"bundle\" or \"scan group\" rather than individual,
unrelated scans.

### User Stories

- **As a user,** when two scans are registered, I want to see them
  grouped together in the project tree to easily understand their
  relationship.

- **As a user,** I want to see an indicator (e.g., an icon or label)
  that shows which scans are the \"reference\" and \"target\" in a
  registration.

### UI/UX

- **SidebarWidget (QTreeView)**:

  - When RegistrationProject::addRegistrationResult() is called, the
    ProjectTreeModel (used by SidebarWidget) should update its
    hierarchy.

  - Instead of two independent scans, a new \"Scan Group\" or \"Bundle\"
    node could be created, containing the registered scans as children.

  - Alternatively, the existing scan nodes could be visually linked or
    re-parented under a common, implicitly created group if they are
    part of a registration.

  - Icons: Add distinct icons for \"reference\" scans and \"target\"
    scans (or a \"registered\" icon).

  - Display registration result properties (e.g., RMS error) as metadata
    on the group node or linked scan nodes.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - When RegistrationProject::addRegistrationResult() is called (after
    \"Accept\" in Sub-Sprint 3.1), MainPresenter should notify the
    SidebarWidget to update its model. This might involve calling
    m_view-\>getSidebar()-\>refreshFromDatabase() or a more targeted
    update method on the ProjectTreeModel.

- **RegistrationProject**:

  - When addRegistrationResult is invoked, it should emit a signal
    (e.g., registrationResultAdded(sourceScanId, targetScanId)) that the
    MainPresenter can listen to.

- **ProjectTreeModel**:

  - Modify ProjectTreeModel::refreshFromDatabase() (or create
    updateScanGroupings()) to query RegistrationProject for active
    RegistrationResult objects.

  - Based on these results, dynamically create and manage \"group\"
    nodes or re-parent individual scan nodes to visually reflect the
    registration.

  - Implement data() method overrides for roles like Qt::DecorationRole
    (for icons) or Qt::ToolTipRole (for RMS error tooltip) for scan
    group/registered scan items.

- **SidebarWidget**:

  - Ensure it correctly displays the hierarchy from ProjectTreeModel.

### Backend (Core Logic/Services)

- **RegistrationProject**:

  - Its getRegistrationResults() method (already exists) will be crucial
    for the ProjectTreeModel to determine groupings.

  - Ensure RegistrationResult stores enough information to identify the
    source and target scans and their roles.

- **SQLiteManager**:

  - The ProjectTreeModel might need new queries to efficiently retrieve
    RegistrationResult data and associated ScanInfos from the database.
    (If RegistrationResult is stored in the database).

### Tests

- **Unit Tests:**

  - test_registrationproject.cpp: Verify addRegistrationResult()
    correctly stores the data and getRegistrationResults() retrieves it.

  - test_projecttreemodel.cpp: Add tests for:

    - populateFromData() or refreshScans() correctly creating
      parent/child nodes based on RegistrationResult (simulated).

    - data() method returning correct display text, icons, and tooltips
      for registered scans/groups.

- **Integration Tests:**

  - Extend the end-to-end test (Sub-Sprint 3.4) to include a visual
    inspection (or mocked UI verification) of the SidebarWidget\'s tree
    structure after an alignment is accepted.

  - Verify that new \"group\" nodes appear or existing scan nodes update
    to show their registered status.

### Acceptance Criteria

- Registered scan pairs appear logically grouped or linked in the
  SidebarWidget\'s project tree.

- The project tree displays key information about the registration
  (e.g., RMS error on the group).

- Visual cues (icons, labels) indicate the relationship and status of
  registered scans.

- The ProjectTreeModel accurately reflects the project\'s registration
  state.

## Sub-Sprint 3.4: End-to-End Integration Test for MVP

### Goal

Create a single, comprehensive automated test that simulates the entire
manual alignment MVP workflow, from loading scans to exporting the
result, verifying all intermediate steps and final outcomes.

### User Stories (Internal/Developer)

- **As a developer,** I want an automated end-to-end test for the manual
  alignment MVP so that I can quickly verify the core functionality is
  intact.

- **As a developer,** I want this test to cover UI interactions, backend
  logic, and file I/O (simulated) to ensure full system integration.

### Test Scope

This test will use mock UI and external services (parser, exporter)
where direct interaction is not feasible, but will directly call and
verify the presenter and core backend logic.

### Steps

1.  **Initialize Components:** Instantiate MainPresenter, MockMainView,
    MockE57Parser, MockPointCloudViewer, MockPointCloudExporter,
    MockProjectManager, and MockPointCloudLoadManager.

2.  **Mock Setup:** Configure mocks to simulate realistic user
    interactions and backend responses (e.g., successful file dialogs,
    successful parsing, successful export).

3.  **Simulate Project Creation:** Call
    MainPresenter::handleNewProject().

4.  **Simulate Scan Loading (Scan A):**

    - Call MainPresenter::handleImportScans() (which calls
      handleOpenFile()).

    - Mock MockE57Parser to return point data for Scan A.

    - Verify MockPointCloudViewer::loadPointCloud() is called.

5.  **Simulate Scan Loading (Scan B):** Repeat step 4 for Scan B.

6.  **Simulate Manual Alignment (Point Selection):**

    - Simulate user selecting points in PointCloudViewerWidget (e.g.,
      directly call AlignmentEngine::addCorrespondence()).

    - Add at least 3 valid correspondences.

7.  **Simulate Alignment Computation & Preview:**

    - Call AlignmentEngine::recomputeAlignment().

    - Verify MockPointCloudViewer::setDynamicTransform() is called.

    - Verify RMS error display in MockMainView.

8.  **Simulate Alignment Acceptance:**

    - Call MainPresenter::handleAcceptAlignment().

    - Verify MockPointCloudViewer::clearDynamicTransform() is called.

    - Verify MockProjectManager::setScanTransform() is called.

    - Verify MockProjectManager::addRegistrationResult() is called.

9.  **Simulate Export:**

    - Call MainPresenter::handleExportPointCloud().

    - Mock MockMainView::askForSaveFilePath() to return a valid export
      path.

    - Mock MockPointCloudExporter::exportPointCloudAsync() to simulate
      success.

    - Verify MockPointCloudExporter::exportPointCloudAsync() is called
      with correct data (transformed point cloud) and options.

10. **Verify Final State:** Assert that the final state of the
    application is consistent with a successful workflow (e.g., project
    open, scans registered, export initiated).

### Tests

- **New File:** tests/integration/manual_alignment_workflow_e2e_test.cpp
  (or similar).

- **Dependencies:** All components from src/, and mocks from
  tests/mocks/.

- This is an **integration test**, not a unit test. It ties together the
  work of Sprints 1, 2, and 3.

### Acceptance Criteria

- A new, dedicated end-to-end integration test file exists.

- The test runs automatically and passes successfully.

- The test covers the full manual alignment MVP workflow: load scans,
  select points, compute alignment, accept alignment, export result.

- All critical interactions between MainPresenter, IMainView,
  IE57Parser, IPointCloudViewer, AlignmentEngine, RegistrationProject,
  and PointCloudExporter are verified.
