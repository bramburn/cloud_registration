# **Product Requirements Document: Sprint 2 - Alignment Computation & Live Preview**

This PRD details the requirements for **Sprint 2: Alignment Computation
& Live Preview**, which is the second sprint in Phase 1 (MVP Delivery)
of our project plan. The overarching goal of this sprint is to enable
users to compute and visualize the alignment of two point cloud scans
based on their previously selected corresponding points, providing
immediate visual feedback and quality metrics.

## Sprint 2 Overview

- **Sprint Goal:** Enable users to compute and visualize the alignment
  based on their selected points, and display the quality metrics.

- **Context:** This sprint builds directly on Sprint 1\'s ability to
  select and manage point correspondences. It introduces the core
  computation and visual feedback loop for manual alignment.

## Sub-Sprint 2.1: Triggering Alignment Computation

### Goal

Implement a UI button to initiate the alignment computation and pass the
selected point pairs to the backend.

### User Stories

- **As a user,** after selecting a sufficient number of point pairs, I
  want to click a button to see a preview of the alignment.

- **As a user,** I want to know that the alignment computation has
  started.

### UI/UX

- A new button, labeled \"Preview Alignment\" or \"Compute Alignment\",
  will be added to the AlignmentControlPanel, positioned logically near
  the list of point pairs.

- The button should be enabled only when a minimum number of point pairs
  (e.g., 3 non-collinear pairs) have been selected (this minimum will be
  enforced by backend validation, but the UI should reflect it).

- Upon clicking, the button should visually indicate it\'s processing
  (e.g., disable itself, show a loading spinner next to it, or update a
  status bar message).

### Frontend (Component/View Layer)

- **AlignmentControlPanel**:

  - Add a new QPushButton for \"Preview Alignment\".

  - Implement logic to enable/disable this button based on the number of
    TargetCorrespondence objects currently stored in TargetManager
    (which MainPresenter will expose).

  - Connect the button\'s clicked signal to a slot that triggers the
    MainPresenter to initiate the alignment computation.

  - Display a simple \"Calculating\...\" or \"Applying
    Transformation\...\" message in a status area.

- **MainPresenter**:

  - Implement a method (e.g., triggerAlignmentPreview()) that is called
    when the \"Preview Alignment\" button is clicked.

  - This method will retrieve the TargetCorrespondence objects from the
    TargetManager.

  - It will then call a method on the AlignmentEngine to perform the
    alignment.

  - It will manage the UI state (e.g., enabling/disabling the button,
    updating status messages) during the computation.

### Backend (Core Logic/Services)

- **TargetManager**:

  - Add a method (e.g., getAllCorrespondences()) to return a std::vector
    or similar collection of all stored TargetCorrespondence objects.

- **AlignmentEngine**:

  - Add a method (e.g., computeLeastSquaresAlignment(const
    std::vector\<TargetCorrespondence\>& correspondences)) that takes
    the selected point pairs.

  - This method will internally call the LeastSquaresAlignment
    algorithm.

  - It should return the computed TransformationMatrix and the
    calculated RMS error.

  - It should handle cases where insufficient or invalid point pairs are
    provided (e.g., fewer than 3, collinear points), returning an error
    or invalid transformation.

- **LeastSquaresAlignment**:

  - Ensure its public interface allows it to be called with a collection
    of TargetCorrespondence objects and returns a TransformationMatrix
    and RMS error.

### Tests

- **Unit Tests:**

  - test_alignmentcontrolpanel.cpp: Verify the \"Preview Alignment\"
    button\'s state (enabled/disabled) changes correctly based on point
    pair count. Verify it emits the correct signal when clicked.

  - test_mainpresenter.cpp: Verify triggerAlignmentPreview() correctly
    retrieves correspondences from TargetManager and calls
    AlignmentEngine.

  - test_targetmanager.cpp: Verify getAllCorrespondences() returns the
    correct data.

- **Integration Tests:**

  - Simulate selecting enough point pairs, then clicking the \"Preview
    Alignment\" button, and verify that the AlignmentEngine\'s
    computeLeastSquaresAlignment method is invoked with the correct
    data.

### Acceptance Criteria

- The \"Preview Alignment\" button is visible in the
  AlignmentControlPanel.

- The \"Preview Alignment\" button is disabled until at least 3 point
  pairs are selected.

- Clicking the enabled \"Preview Alignment\" button triggers the
  alignment computation in the backend.

- The UI provides feedback that the computation is in progress.

## Sub-Sprint 2.2: Backend Alignment Execution & Result Retrieval

### Goal

Successfully execute the LeastSquaresAlignment algorithm in the backend
and retrieve its results (transformation matrix and RMS error).

### User Stories

- **As a developer,** I want the AlignmentEngine to robustly compute the
  alignment using LeastSquaresAlignment given the selected points.

- **As a developer,** I want to reliably receive the computed
  transformation and RMS error from the backend.

### UI/UX

- No direct UI changes for this sub-sprint, as it\'s primarily backend
  logic. However, the status message from Sub-Sprint 2.1 should reflect
  completion or error.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Wait for the AlignmentEngine to complete its computation
    (potentially asynchronously).

  - Handle the success or failure result from AlignmentEngine.

  - If successful, store the TransformationMatrix and RMS error for
    later use by PointCloudViewerWidget and AlignmentControlPanel.

  - If failed, update the status message to indicate an error (e.g.,
    \"Alignment Failed: Insufficient points\").

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - The computeLeastSquaresAlignment method will:

    - Validate the input correspondences (e.g., minimum count, check for
      degenerate configurations).

    - Instantiate and call
      LeastSquaresAlignment::computeTransformation(pointOnScanA,
      pointOnScanB) with the extracted point data.

    - Retrieve the resulting TransformationMatrix and RMS_Error from
      LeastSquaresAlignment.

    - Return these results (or an error indicator) to the caller
      (MainPresenter).

- **LeastSquaresAlignment**:

  - Ensure its computeTransformation method is robust and handles
    various point configurations (collinear, co-planar, insufficient
    points) gracefully, either by throwing exceptions or returning
    specific error codes/status.

### Tests

- **Unit Tests:**

  - test_alignmentengine.cpp: Thoroughly test
    computeLeastSquaresAlignment() with:

    - Valid sets of correspondences (different numbers of pairs).

    - Edge cases: fewer than 3 pairs, collinear points, identical
      points.

    - Verify correct TransformationMatrix and RMS error are returned.

  - test_least_squares_alignment.cpp: Revisit and enhance existing tests
    to ensure robustness and correct output of both transformation and
    RMS error.

- **Integration Tests:**

  - Simulate the full backend pipeline: TargetManager -\> MainPresenter
    -\> AlignmentEngine -\> LeastSquaresAlignment, verifying that a
    valid transformation and RMS error are returned to MainPresenter
    when valid points are provided.

  - Test error handling path for invalid inputs.

### Acceptance Criteria

- The AlignmentEngine successfully computes a TransformationMatrix and
  RMS error when provided with valid point correspondences.

- The MainPresenter receives and stores the computed transformation and
  RMS error.

- The backend handles invalid point sets gracefully, providing
  appropriate feedback to the MainPresenter.

## Sub-Sprint 2.3: Live Preview & RMS Display

### Goal

Visually apply the computed transformation to the moving scan in the
viewer and display the calculated RMS error to the user.

### User Stories

- **As a user,** I want to immediately see the effect of the computed
  alignment on the point clouds in the 3D viewer.

- **As a user,** I want to see the Root Mean Square (RMS) error of the
  alignment prominently displayed.

### UI/UX

- The PointCloudViewerWidget will update to show the *moving scan*
  (e.g., Scan B) transformed by the computed TransformationMatrix. The
  fixed scan (Scan A) remains stationary.

- The transformed scan should appear to snap into alignment with the
  fixed scan. This is a \"preview\" state, not a permanent change yet.

- A dedicated QLabel or text area in the AlignmentControlPanel (or a
  persistent status bar) will display the calculated RMS error with
  appropriate units (e.g., \"RMS Error: 0.015 m\").

### Frontend (Component/View Layer)

- **PointCloudViewerWidget**:

  - Implement a method (e.g., applyPreviewTransformation(scanId: ScanID,
    transform: TransformationMatrix)) that takes the ID of the scan to
    transform (the moving one) and the TransformationMatrix.

  - Internally, PointCloudViewerWidget will instruct its OpenGLRenderer
    to render the specified scan using this temporary transformation *in
    addition to* its current loaded position. This must be a preview,
    not a modification of the underlying scan data.

- **OpenGLRenderer**:

  - Needs to be able to render a point cloud using an arbitrary
    transformation matrix for preview purposes, without altering the
    base geometry of the point cloud data. This might involve updating
    the model matrix used for that specific scan\'s drawing.

- **AlignmentControlPanel**:

  - Add a QLabel or similar widget to display the RMS error.

  - MainPresenter will update this label with the RMS error value
    received from AlignmentEngine.

- **MainPresenter**:

  - Upon successful completion of the alignment computation, call
    PointCloudViewerWidget::applyPreviewTransformation() with the moving
    scan\'s ID and the computed matrix.

  - Update the RMS error QLabel in the AlignmentControlPanel with the
    value from AlignmentEngine.

### Backend (Core Logic/Services)

- **ErrorAnalysis**:

  - Confirm its calculateRMSError() method (or similar) is robust and
    provides a clear, single RMS value from the differences between
    aligned points. (This is likely called internally by
    LeastSquaresAlignment or AlignmentEngine).

### Tests

- **Unit Tests:**

  - test_pointcloudviewerwidget.cpp: Test applyPreviewTransformation()
    to ensure it correctly instructs the OpenGLRenderer to apply the
    transformation.

  - test_openglrenderer.cpp: Test the rendering logic for applying
    temporary transformations to specific models.

  - test_alignmentcontrolpanel.cpp: Test that the RMS error label can be
    updated correctly.

  - test_mainpresenter.cpp: Test that MainPresenter correctly calls
    PointCloudViewerWidget and AlignmentControlPanel to display the
    results.

- **Integration Tests:**

  - Perform a full workflow: load scans, select points, click \"Preview
    Alignment\", and verify that the transformed scan appears correctly
    in the viewer and the RMS error is displayed. This will require
    visual inspection or programmatic pixel comparison for the viewer
    part.

### Acceptance Criteria

- After clicking \"Preview Alignment\", the 3D viewer immediately
  updates to show the moving scan transformed according to the computed
  alignment.

- The alignment preview does not permanently alter the original scan
  data.

- The calculated RMS error is displayed clearly and accurately in the
  AlignmentControlPanel.

- The UI provides feedback on the status of the alignment (e.g.,
  \"Alignment Preview Ready\" or \"RMS: 0.015 m\").
