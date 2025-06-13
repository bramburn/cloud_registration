# **Detailed Backlog: Sprint 1, Sub-Sprint 1.1 - UI Mode Activation & Initial Viewer Integration**

## Introduction

This document provides a detailed backlog for Sub-Sprint 1.1: UI Mode
Activation & Initial Viewer Integration, which is the foundational step
for enabling manual alignment within the Cloud Registration application.
This sub-sprint focuses on preparing the user interface and the 3D
viewer to receive user input for point selection.

## User Stories

- **User Story 1**: As a user, I want to click a button to initiate the
  manual alignment workflow.

  - **Description**: This user story enables the user to explicitly
    enter a mode where they can begin the process of manually aligning
    two point clouds. It signifies the start of a guided workflow.

  - **Actions to Undertake**:

    1.  **Modify RegistrationWorkflowWidget**: Add a new QPushButton
        element within the RegistrationWorkflowWidget\'s UI. Label this
        button clearly, for example, \"Start Manual Alignment\".

    2.  **Connect Signal to Slot**: Establish a connection so that when
        this \"Start Manual Alignment\" button is clicked, it emits a
        signal that the MainPresenter can receive.

    3.  **Implement MainPresenter::startManualAlignmentMode()**: Create
        this method in MainPresenter. This method will be responsible
        for updating the internal application state to reflect that
        manual alignment mode is active.

    4.  **Instruct PointCloudViewerWidget**: From within
        MainPresenter::startManualAlignmentMode(), call a method on the
        PointCloudViewerWidget (e.g., setSelectionMode(ManualAlignment))
        to visually prepare the 3D viewer for point selection.

    5.  **Implement PointCloudViewerWidget::setSelectionMode()**:
        Implement this method to configure the viewer for point
        selection. This includes potentially changing the cursor,
        displaying crosshairs, or enabling an internal
        NaturalPointSelector for input capture.

    6.  **Update UI Feedback**: Implement logic to visually change the
        UI to indicate the active mode (e.g., disable the \"Start Manual
        Alignment\" button, update a status bar message).

  - **References between Files**:

    - RegistrationWorkflowWidget (modifies UI, emits signal) -\>
      MainPresenter (receives signal, updates state, calls viewer)

    - MainPresenter -\> PointCloudViewerWidget (calls method to set
      selection mode)

    - PointCloudViewerWidget -\> NaturalPointSelector (enables internal
      component for point picking)

    - AlignmentEngine: May require initialization or state reset via
      MainPresenter.

    - TargetManager: Needs to be accessible for future point storage,
      but no direct interaction in this sub-sprint.

  - **Acceptance Criteria**:

    - A clearly labeled \"Start Manual Alignment\" button is visible in
      the RegistrationWorkflowWidget.

    - Clicking the \"Start Manual Alignment\" button changes the UI to
      indicate that manual alignment mode is active.

    - The PointCloudViewerWidget visually prepares itself for point
      selection (e.g., cursor change, visible guides).

  - **Testing Plan**:

    - **Test Case 1**: Verify button presence and signal emission.

      - **Test Data**: N/A

      - **Expected Result**: The \"Start Manual Alignment\" button is
        visible and, when clicked, emits the expected signal (mocked).

      - **Testing Tool**: Unit test
        (test_registrationworkflowwidget.cpp)

    - **Test Case 2**: Verify MainPresenter state update and viewer
      instruction.

      - **Test Data**: N/A

      - **Expected Result**: MainPresenter\'s internal mode variable is
        set to ManualAlignment, and
        PointCloudViewerWidget::setSelectionMode() is called (mocked).

      - **Testing Tool**: Unit test (test_mainpresenter.cpp)

    - **Test Case 3**: Verify PointCloudViewerWidget selection mode
      activation.

      - **Test Data**: N/A

      - **Expected Result**: When setSelectionMode(ManualAlignment) is
        called (mocked), PointCloudViewerWidget configures its internal
        NaturalPointSelector for readiness.

      - **Testing Tool**: Unit test (test_pointcloudviewerwidget.cpp)

    - **Test Case 4**: End-to-End UI mode activation.

      - **Test Data**: Application launched with loaded scans.

      - **Expected Result**: Clicking \"Start Manual Alignment\" button
        visually activates the mode in the UI and prepares the viewer
        for point picking.

      - **Testing Tool**: Integration test (manual observation or UI
        automation script if available)

## Actions to Undertake

1.  **Frontend (UI Components)**:

    - **RegistrationWorkflowWidget**:

      - Add a new QPushButton for \"Start Manual Alignment\".

      - Connect its clicked signal to a method in MainPresenter.

      - Implement visual feedback mechanisms (e.g., changing button
        appearance, updating a status label) when the mode is active.

    - **MainPresenter**:

      - Implement startManualAlignmentMode() method to update internal
        currentWorkflowMode and call
        PointCloudViewerWidget::setSelectionMode().

      - Implement handleStartManualAlignmentClicked() slot to connect to
        the UI button.

    - **PointCloudViewerWidget**:

      - Implement setSelectionMode(mode: Enum): This method will
        encapsulate the logic to prepare the viewer for point selection.
        It should enable an internal NaturalPointSelector (if it
        controls one) and potentially change the cursor or render
        temporary guides.

2.  **Backend (Core Logic/Services)**:

    - **AlignmentEngine**:

      - Review existing code to determine if AlignmentEngine needs a
        method to reset/initialize its state for a new manual alignment
        session. If so, add void resetAlignmentState();.

    - **TargetManager**:

      - Verify TargetManager is properly initialized and accessible to
        AlignmentEngine or MainPresenter for future use in storing
        correspondences. No direct modifications are expected in this
        sub-sprint, but its readiness is a dependency.

## References between Files

- **RegistrationWorkflowWidget.h / RegistrationWorkflowWidget.cpp**:

  - **Modifies**: Adds a new QPushButton.

  - **Calls**: Connects to
    MainPresenter::handleStartManualAlignmentClicked().

- **MainPresenter.h / MainPresenter.cpp**:

  - **Modifies**: Adds startManualAlignmentMode() method and
    handleStartManualAlignmentClicked() slot.

  - **Calls**: Calls PointCloudViewerWidget::setSelectionMode().

- **PointCloudViewerWidget.h / PointCloudViewerWidget.cpp**:

  - **Modifies**: Adds setSelectionMode(mode: Enum) method. This method
    will likely interact with internal rendering logic to draw selection
    aids (e.g., crosshairs).

  - **Uses**: May use NaturalPointSelector internally.

- **AlignmentEngine.h / AlignmentEngine.cpp**:

  - **Modifies (Potentially)**: Add void resetAlignmentState(); if
    needed for a clean start to manual alignment.

- **TargetManager.h / TargetManager.cpp**:

  - **Dependency**: Will be used in subsequent sub-sprints to store
    TargetCorrespondence objects.

## List of Files being Created

- No new files are expected for this sub-sprint. Existing files will be
  modified.

## Acceptance Criteria

- The \"Start Manual Alignment\" button is present in the
  RegistrationWorkflowWidget and is clickable.

- When clicked, the application enters a \"Manual Alignment\" mode,
  clearly indicated by UI changes (e.g., button state, status bar
  message).

- The PointCloudViewerWidget is visually prepared to accept point
  selections (e.g., changed cursor, rendering of point-picking guides).

- Internal application state (e.g., MainPresenter\'s
  currentWorkflowMode) correctly reflects the active manual alignment
  mode.

## Testing Plan

- **Unit Test**: test_registrationworkflowwidget.cpp

  - **Test Case 1**: Verify \"Start Manual Alignment\" button existence
    and signal emission.

    - **Test Data**: N/A

    - **Expected Result**: Button is present. Clicking it emits the
      clicked signal.

    - **Testing Tool**: Google Test / Qt Test

  - **Test Case 2**: Verify MainPresenter\'s handling of \"Start Manual
    Alignment\" request.

    - **Test Data**: Mocked IMainView and IPointCloudViewer to simulate
      UI interaction and verify method calls.

    - **Expected Result**: MainPresenter::startManualAlignmentMode() is
      called, which in turn calls
      IPointCloudViewer::setSelectionMode(ManualAlignment).

    - **Testing Tool**: Google Mock / Qt Test

- **Unit Test**: test_pointcloudviewerwidget.cpp

  - **Test Case 1**: Verify setSelectionMode(ManualAlignment) prepares
    the viewer.

    - **Test Data**: N/A (unit test on PointCloudViewerWidget directly).

    - **Expected Result**: Internal flags or visual states related to
      point selection (e.g., m_selectionModeEnabled flag,
      m_showCrosshairs flag) are set correctly.

    - **Testing Tool**: Google Test / Qt Test

- **Integration Test**: Manual Alignment UI Flow

  - **Test Case 1**: Full UI Mode Activation Flow.

    - **Test Data**: A loaded point cloud project with at least two
      scans.

    - **Expected Result**: Launch the application. Click \"Start Manual
      Alignment\". Observe button state change, status bar message, and
      visual cues (e.g., crosshairs, cursor change) in the 3D viewer.

    - **Testing Tool**: Manual QA / UI automation script.

## Assumptions and Dependencies

- **Existing Components**: RegistrationWorkflowWidget, MainPresenter,
  PointCloudViewerWidget, NaturalPointSelector, AlignmentEngine, and
  TargetManager classes are already implemented (even if as stubs) and
  correctly linked in the build system.

- **Qt Environment**: A functional Qt 6 development environment is
  assumed for UI development and testing.

- **Viewer Ready**: The PointCloudViewerWidget is capable of rendering
  basic point clouds and responding to mouse events (even if the actual
  point picking logic is rudimentary in this sub-sprint).

- **Build System**: CMake setup correctly integrates all modules and
  handles Qt\'s MOC.

## Non-Functional Requirements

- **Usability**: The \"Start Manual Alignment\" button should be easily
  discoverable and intuitive.

- **Responsiveness**: The UI should remain responsive while switching to
  the manual alignment mode; no noticeable freezing or delays.

- **Consistency**: The visual feedback for mode activation should be
  consistent with other application state changes.
