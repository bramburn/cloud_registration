# Product Requirements Document: Sprint 1 - Point Selection UI & State Management

This PRD details the requirements for **Sprint 1: Point Selection UI &
State Management**, which is the first sprint in Phase 1 (MVP Delivery)
of our project plan. The overarching goal of this sprint is to enable
users to intuitively select and view corresponding points between two
point cloud scans within the application.

## Sprint 1 Overview

- **Sprint Goal:** Enable users to select and view corresponding points
  between two scans to prepare for manual alignment.

- **Context:** This sprint lays the foundational UI and state management
  for the core manual alignment workflow. It builds upon the existing
  PointCloudViewerWidget, NaturalPointSelector, and TargetManager
  components.

## Sub-Sprint 1.1: UI Mode Activation & Initial Viewer Integration

### Goal

Implement a clear \"Start Manual Alignment\" mode within the
application\'s UI and ensure the viewer is ready to accept point
selections in this mode.

### User Stories

- **As a user,** I want to click a button to initiate the manual
  alignment workflow so that I can begin selecting points.

- **As a user,** I want the application to clearly indicate when I am in
  manual alignment mode.

### UI/UX

- A prominent button or menu item, labeled \"Manual Alignment\" or
  \"Start Manual Alignment\", will be added to the
  RegistrationWorkflowWidget (or MainPresenter\'s toolbar/menu).

- Upon activation, the UI should visually change (e.g., button state
  changes, a status bar message appears) to confirm the mode is active.

- The PointCloudViewerWidget should be visually prepared for point
  selection, possibly by highlighting the active scan for point picking
  or displaying crosshairs.

### Frontend (Component/View Layer)

- **RegistrationWorkflowWidget**:

  - Add a new QPushButton (or similar UI element) for \"Start Manual
    Alignment\".

  - Connect its clicked signal to a slot that triggers the MainPresenter
    to activate the manual alignment mode.

- **MainPresenter**:

  - Implement a method (e.g., startManualAlignmentMode()) that sets an
    internal state variable (e.g., currentWorkflowMode =
    ManualAlignment).

  - This method should also instruct the PointCloudViewerWidget to enter
    a point selection-ready state.

- **PointCloudViewerWidget**:

  - Implement a method (e.g., setSelectionMode(mode: Enum)) that, when
    ManualAlignment mode is set, enables its internal
    NaturalPointSelector and prepares the cursor/visual feedback.

### Backend (Core Logic/Services)

- **AlignmentEngine**:

  - Might need a method to initialize or reset the manual alignment
    state, clearing any previously selected points.

- **TargetManager**:

  - Ensure TargetManager is accessible and ready to store
    TargetCorrespondence objects, although actual storage will happen in
    later sub-sprints.

  - Potentially add a clearTargets() method for mode resets.

### Tests

- **Unit Tests:**

  - test_registrationworkflowwidget.cpp: Verify the \"Start Manual
    Alignment\" button exists and emits the correct signal.

  - test_mainpresenter.cpp: Verify MainPresenter correctly updates its
    internal state and calls the PointCloudViewerWidget to change its
    mode.

  - test_pointcloudviewerwidget.cpp: Verify setSelectionMode() correctly
    enables NaturalPointSelector.

- **Integration Tests:**

  - Test the flow from clicking the \"Start Manual Alignment\" button to
    the PointCloudViewerWidget being in the correct selection mode.

### Acceptance Criteria

- The \"Start Manual Alignment\" button is present and clickable.

- Clicking the button puts the application into a clearly indicated
  \"Manual Alignment\" mode.

- The PointCloudViewerWidget is ready to receive point selection inputs.

## Sub-Sprint 1.2: Point Selection Logic & Visual Feedback

### Goal

Enable users to click on points in the PointCloudViewerWidget and for
the application to register these clicks as selected points, providing
immediate visual feedback.

### User Stories

- **As a user,** I want to click on a point in the first scan (e.g.,
  Scan A) to select it.

- **As a user,** I want to click on a corresponding point in the second
  scan (e.g., Scan B) to select it.

- **As a user,** I want to see visual markers (e.g., spheres,
  crosshairs) on selected points to confirm my selection.

- **As a user,** I want the application to guide me on which scan to
  pick next (e.g., \"Select point in Scan B\").

### UI/UX

- When a point is selected in Scan A, a small, distinct visual marker
  (e.g., a colored sphere) appears at the selected location.

- A clear message appears, guiding the user to select the corresponding
  point in Scan B.

- When the corresponding point is selected in Scan B, a similar distinct
  marker appears, perhaps with a different color or linked visually to
  the first point\'s marker (e.g., a dashed line connecting them in 3D
  space, or matching IDs).

- The cursor might change to indicate point selection mode.

### Frontend (Component/View Layer)

- **PointCloudViewerWidget**:

  - Implement mousePressEvent handling (if not already present for
    selection) to capture screen coordinates.

  - Translate screen coordinates to 3D point cloud coordinates using
    OpenGLRenderer\'s projection logic.

  - Pass the 3D point and the ID of the scan it belongs to (ScanA or
    ScanB) to the MainPresenter.

  - OpenGLRenderer:

    - Implement functionality to draw temporary visual markers (e.g.,
      drawSphere(position, color) or drawCrosshairs(position, color)).

    - Manage the rendering of these markers based on data provided by
      PointCloudViewerWidget or MainPresenter.

- **MainPresenter**:

  - Receive selected points from PointCloudViewerWidget.

  - Manage the internal state of which scan is currently active for
    selection (e.g., expectingPointInScanA, expectingPointInScanB).

  - Upon receiving two points (one from Scan A, one from Scan B), create
    a TargetCorrespondence object.

  - Pass the TargetCorrespondence object to the TargetManager (or
    AlignmentEngine if it\'s the primary state holder for
    correspondences).

  - Update status messages in the UI (e.g., a QLabel in the
    AlignmentControlPanel) to guide the user (\"Click point in Scan B\",
    \"Pair 1 selected\").

### Backend (Core Logic/Services)

- **NaturalPointSelector**:

  - Ensure its selectPoint() method correctly identifies the 3D point on
    the point cloud given screen coordinates and view parameters.

- **TargetManager**:

  - Implement addCorrespondence(correspondence: TargetCorrespondence) to
    store the selected point pairs.

- **TargetCorrespondence (Data Structure)**:

  - Confirm its structure: struct TargetCorrespondence { Point3D
    pointOnScanA; Point3D pointOnScanB; int pairId; }

### Tests

- **Unit Tests:**

  - test_pointcloudviewerwidget.cpp: Test mousePressEvent to ensure it
    correctly identifies the clicked 3D point and passes it up.

  - test_openglrenderer.cpp: Test the rendering of temporary visual
    markers.

  - test_mainpresenter.cpp: Test its state management for
    expectingPointInScanA/expectingPointInScanB and TargetCorrespondence
    creation.

  - test_targetmanager.cpp: Test addCorrespondence to ensure it stores
    points correctly.

- **Integration Tests:**

  - Simulate a user clicking on points in both scans and verify that
    visual markers appear and a TargetCorrespondence object is formed
    and stored.

### Acceptance Criteria

- Users can click to select points on both Scan A and Scan B.

- Each selected point (both in Scan A and Scan B) receives a clear
  visual marker in the 3D viewer.

- The UI provides real-time guidance on which scan the user should
  select a point from next.

- The application correctly captures and internally stores the 3D
  coordinates of the selected points.

## Sub-Sprint 1.3: Point Pair Management & Display

### Goal

Provide a user-facing list or table that displays the manually selected
point pairs, allowing for review and potential deletion.

### User Stories

- **As a user,** I want to see a list of all selected point pairs.

- **As a user,** I want to clearly identify which point belongs to which
  scan within the list.

- **As a user,** I want the list to update automatically as I select new
  point pairs.

- **As a user,** I want to be able to delete a selected point pair if I
  make a mistake.

### UI/UX

- A dedicated panel, likely within the AlignmentControlPanel, will house
  a QTableView or QListWidget to display the selected point pairs.

- Each row in the table will represent a TargetCorrespondence, showing:

  - Pair ID (e.g., \"Pair 1\", \"Pair 2\")

  - Coordinates of the point in Scan A (e.g., (X.XX, Y.YY, Z.ZZ))

  - Coordinates of the point in Scan B (e.g., (X.XX, Y.YY, Z.ZZ))

- Add a \"Delete Selected Pair\" button next to the list, or allow row
  selection and deletion via Del key.

- Visual markers in the 3D viewer should correspond to the selected pair
  in the list (e.g., highlighting or changing color when a pair is
  selected in the list).

### Frontend (Component/View Layer)

- **AlignmentControlPanel**:

  - Add a QTableView or QListWidget component to display
    TargetCorrespondence data.

  - Implement a custom QAbstractTableModel (or similar) to provide data
    for the QTableView from the TargetManager. This model should emit
    dataChanged signals when the underlying data updates.

  - Add a \"Delete Selected Pair\" button.

  - Connect the button\'s clicked signal to a slot that retrieves the
    selected row from the table and sends a request to the MainPresenter
    to delete that correspondence.

- **MainPresenter**:

  - Acts as a mediator: receives updates from TargetManager (via
    signals) and updates the AlignmentControlPanel\'s model.

  - Implements a method (e.g., deleteCorrespondence(pairId: int)) that
    calls the TargetManager to remove the correspondence.

  - Updates the PointCloudViewerWidget to remove the visual markers for
    the deleted pair.

- **PointCloudViewerWidget**:

  - Implement removeMarkersForPair(pairId: int) to clear the visual
    feedback for deleted pairs.

- **OpenGLRenderer**:

  - Needs to be able to clear specific rendered markers.

### Backend (Core Logic/Services)

- **TargetManager**:

  - Implement removeCorrespondence(pairId: int) method to remove a
    TargetCorrespondence object from its internal storage.

  - Emit a signal (e.g., correspondenceRemoved(pairId)) when a pair is
    deleted, so MainPresenter can update the UI.

  - Consider a signal (e.g., correspondenceAdded(pairId)) for when a new
    pair is added, so the list updates dynamically.

### Tests

- **Unit Tests:**

  - test_alignmentcontrolpanel.cpp: Test the table model, its ability to
    display data, and button functionality.

  - test_mainpresenter.cpp: Test its mediation logic between
    TargetManager and AlignmentControlPanel.

  - test_targetmanager.cpp: Test removeCorrespondence and signal
    emission.

- **Integration Tests:**

  - Simulate adding multiple point pairs, verifying they appear in the
    list.

  - Simulate deleting a point pair from the list and verifying it
    disappears from both the list and the 3D viewer.

- **End-to-End Test (Basic):**

  - Automate adding a few pairs and deleting one, verifying the state of
    the UI and internal data.

### Acceptance Criteria

- A list/table displaying selected point pairs is visible in the UI.

- The list accurately shows the coordinates of the selected points for
  each pair.

- The list updates in real-time as point pairs are added or deleted.

- Users can select a point pair in the list and delete it via a
  dedicated button or key press.

- Deleting a pair removes its corresponding visual markers from the 3D
  viewer.
