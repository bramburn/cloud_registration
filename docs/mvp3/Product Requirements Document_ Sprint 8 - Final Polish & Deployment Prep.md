# **Product Requirements Document: Sprint 8 - Final Polish & Deployment Prep**

This PRD details the requirements for **Sprint 8: Final Polish &
Deployment Prep**, the concluding sprint in Phase 3 (Production
Hardening) of our project plan. The overarching goal of this sprint is
to finalize the application\'s usability, ensure stability, prepare for
simplified deployment, and provide essential user documentation.

## Sprint 8 Overview

- **Sprint Goal:** Deliver a highly polished, stable, and easily
  deployable application with comprehensive user documentation,
  incorporating critical user experience features like Undo/Redo
  functionality.

- **Context:** This sprint builds upon all previous development,
  including core registration workflows (Sprints 1-5), quality
  assessment (Sprint 6), and global optimization/performance profiling
  (Sprint 7). It addresses final usability enhancements and prepares the
  product for release.

## Sub-Sprint 8.1: Robust Undo/Redo Stack for Registration Actions

### Goal

Implement a reliable Undo/Redo mechanism specifically for
registration-related actions, allowing users to revert or reapply
changes made to scan poses.

### User Stories

- **As a user,** I want to undo my last registration action (e.g.,
  manual alignment, ICP, target-based alignment) if I make a mistake or
  want to try a different approach.

- **As a user,** I want to redo a previously undone registration action
  so I can reapply changes without redoing the work manually.

- **As a user,** I want clear visual feedback (e.g., enabled/disabled
  buttons) indicating whether Undo or Redo operations are available.

### UI/UX

- **Menu Bar**: Add \"Undo\" (Ctrl+Z) and \"Redo\" (Ctrl+Y) actions to
  the \"Edit\" menu.

- **Toolbar (Optional)**: Add corresponding \"Undo\" and \"Redo\"
  buttons to a relevant toolbar (e.g., ViewerToolbar or
  RegistrationWorkflowWidget\'s control area).

- **Enabled State**: \"Undo\" should be enabled only if there\'s a
  previous state to revert to. \"Redo\" should be enabled only if
  there\'s an undone state to reapply.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleUndo() and handleRedo() slots, connected to the UI
    actions.

  - This presenter will manage the overall Command history.

  - It will listen to signals from RegistrationProject (or
    AlignmentEngine) when a significant registration action occurs
    (RegistrationResult added, ScanInfo::transform updated).

  - When an action is performed, MainPresenter will create a Command
    object (e.g., ApplyTransformationCommand) and push it onto a history
    stack (e.g., RegistrationHistoryManager).

  - When handleUndo() is called:

    - Retrieve the last Command from the RegistrationHistoryManager.

    - Execute the undo() method of that Command.

    - Update RegistrationProject and PointCloudViewerWidget to reflect
      the undone state.

    - Manage the enabled/disabled state of Undo/Redo UI elements.

  - When handleRedo() is called:

    - Retrieve the next Command from the RegistrationHistoryManager\'s
      redo stack.

    - Execute the redo() method of that Command.

    - Update RegistrationProject and PointCloudViewerWidget.

    - Manage the enabled/disabled state of Undo/Redo UI elements.

- **MainWindow**:

  - Add \"Undo\" and \"Redo\" QActions.

  - Manage their enabled states based on signals from MainPresenter
    (e.g., undoAvailabilityChanged(bool),
    redoAvailabilityChanged(bool)).

- **PointCloudViewerWidget**:

  - Ensure it can efficiently re-render point clouds after
    transformations are changed by Undo/Redo operations.

### Backend (Core Logic/Services)

- **RegistrationHistoryManager.h/.cpp (New Component in
  src/registration/)**:

  - Implement a generic Command interface or abstract base class (e.g.,
    ICommand with execute(), undo(), redo()).

  - Define concrete Command classes for registration actions:

    - ApplyTransformationCommand: Stores previous ScanInfo::transform,
      new ScanInfo::transform, and the affected scan ID. execute()
      applies new, undo() applies previous, redo() applies new.

    - AddRegistrationResultCommand: Stores the RegistrationResult and
      affects the RegistrationProject. execute() adds, undo() removes.

  - Implement a RegistrationHistoryManager class (using QStack or
    std::vector for Undo and Redo stacks) to manage the sequence of
    ICommand objects.

  - Provide methods like pushCommand(ICommand\* command), undo(),
    redo(), canUndo(), canRedo().

- **RegistrationProject**:

  - Existing methods like setScanTransform() and
    removeRegistrationResult() will be used by the Command objects
    during Undo/Redo.

  - It may need new signals like scanTransformChanged(const QString&
    scanId, const QMatrix4x4& newTransform) to notify the MainPresenter
    about changes.

- **AlignmentEngine**:

  - After an alignment is accepted (manual, ICP, target-based), it
    should explicitly trigger the creation and pushing of the
    appropriate Command object to RegistrationHistoryManager.

### Tests

- **Unit Tests:**

  - tests/registration/test_registration_history.cpp (new test file):

    - Test ICommand implementations: execute(), undo(), redo() correctly
      manipulate dummy data.

    - Test RegistrationHistoryManager: pushCommand() adds to history,
      undo()/redo() change stack state, canUndo()/canRedo() reflect
      stack contents.

    - Test correct application of multiple sequential and interleaved
      Undo/Redo operations.

  - test_mainpresenter.cpp:

    - Test handleUndo() and handleRedo() correctly invoke
      RegistrationHistoryManager and trigger UI updates.

    - Verify Undo/Redo UI action states are managed correctly.

- **Integration Tests:**

  - Extend tests/integration/global_optimization_e2e_test.cpp.

  - Simulate multiple registration actions (e.g., a manual alignment,
    then an ICP refinement).

  - Simulate clicking \"Undo\" and verify the viewer reverts to the
    previous state.

  - Simulate clicking \"Redo\" and verify the viewer reapplies the
    change.

  - Verify Undo/Redo buttons\' enabled/disabled states change
    dynamically.

### Acceptance Criteria

- \"Undo\" and \"Redo\" actions/buttons are available in the UI.

- The Undo/Redo functionality works for accepted manual, ICP, and
  target-based registrations.

- \"Undo\" and \"Redo\" actions are correctly enabled/disabled based on
  history availability.

- The 3D viewer accurately reflects the state after Undo/Redo
  operations.

## Sub-Sprint 8.2: UI/UX Refinement & User Feedback Integration

### Goal

Refine the overall User Interface and User Experience based on
anticipated user feedback, ensuring a polished, intuitive, and
consistent application.

### User Stories

- **As a user,** I want the application to look and feel professional
  and modern, with consistent styling and clear visual hierarchies.

- **As a user,** I want intuitive navigation and interaction patterns
  that make the application easy to learn and use.

- **As a user,** I want clear and concise feedback messages for all
  operations.

### UI/UX

- **Global Styling**: Leverage UIThemeManager (Sprint 7) to apply a
  consistent, modern visual theme across all widgets (buttons, text
  inputs, labels, group boxes, tables, trees, progress bars, etc.).

- **Layout & Spacing**: Review all layouts for optimal spacing,
  alignment, and readability, ensuring a clean and uncluttered look.

- **Iconography**: Use consistent, high-quality icons for all actions
  and elements (e.g., using Font Awesome or custom SVG icons where
  appropriate).

- **Feedback & Messaging**:

  - Standardize all user-facing messages (information, warning, error,
    success).

  - Ensure non-blocking notifications where possible (e.g., brief status
    bar messages for non-critical info).

- **Accessibility**: Basic accessibility improvements (e.g., keyboard
  navigation for common controls, sufficient color contrast).

- **Responsiveness**: Ensure the UI scales gracefully on different
  screen resolutions and window sizes.

### Frontend (Component/View Layer)

- **UIThemeManager**:

  - Ensure all standard component styles (buttons, line edits, combo
    boxes, labels, etc.) are fully implemented in
    generateAllComponentStyles().

  - Apply the generated stylesheet globally to the QApplication.

- **Existing UI Widgets**:

  - Review and update mainwindow.cpp, AlignmentControlPanel.cpp,
    ExportDialog.cpp, ICPProgressWidget.cpp, TargetDetectionDialog.cpp,
    createprojectdialog.cpp, loadingsettingsdialog.cpp,
    projecthubwidget.cpp, scanimportdialog.cpp, sidebarwidget.cpp,
    WorkflowProgressWidget.cpp.

  - Replace hardcoded styles with UIThemeManager calls (e.g.,
    UIThemeManager::instance().getColorHex(UIThemeManager::ColorRole::Primary)).

  - Adjust layouts (e.g., using QFormLayout, QGridLayout more
    effectively) and add appropriate stretch factors and margins.

  - Improve tooltips and status messages for clarity.

- **PointCloudViewerWidget**:

  - Ensure smooth camera controls and rendering performance.

  - Improve overlay text (e.g., loading, error messages) for better
    readability and visual integration.

### Backend (Core Logic/Services)

- **No direct backend logic changes.** This sub-sprint is primarily
  focused on the presentation layer.

### Tests

- **Manual QA / Usability Testing**: This is the primary testing method
  for UI/UX refinement. Conduct internal testing sessions.

- **Visual Regression Tests (Optional)**: If a framework is in place
  (e.g., screenshot comparison), implement tests to catch unintended
  visual changes.

- **Unit Tests**:

  - tests/ui/test_ui_enhancement.cpp: Expand existing tests to verify
    that UI components correctly apply styles from UIThemeManager.

  - Verify consistency of messages and layouts across different UI
    components through mocked interactions.

- **Integration Tests**:

  - Existing end-to-end tests should ensure the application remains
    functional after UI changes (no breakage).

### Acceptance Criteria

- The application adheres to a consistent, modern visual theme
  throughout all UI components.

- All major UI elements (buttons, inputs, labels, containers) have
  professional and consistent styling.

- User-facing messages are clear, concise, and follow a standard format.

- The application layout is clean, well-spaced, and responsive to window
  resizing.

## Sub-Sprint 8.3: Deployment Finalization (docker-compose.yml)

### Goal

Finalize the Docker deployment setup to allow for easy and consistent
deployment of the application, including handling of GUI applications
within Docker.

### User Stories

- **As a system administrator,** I want to deploy the application easily
  using docker-compose on a Linux system with X11 forwarding for the
  GUI.

- **As a system administrator,** I want the Docker setup to include
  basic data persistence and log management.

### UI/UX

- **No direct UI/UX changes** in the application itself. This sub-sprint
  focuses on the deployment environment.

### Frontend (Component/View Layer)

- **No direct frontend code changes.**

### Backend (Core Logic/Services)

- **deployment/docker/Dockerfile**:

  - Ensure it correctly builds the CloudRegistration application with
    all necessary dependencies.

  - Verify it includes Qt libraries and any other runtime dependencies.

  - Set up a non-root user for security best practices.

- **deployment/docker/docker-compose.yml**:

  - Review and finalize docker-compose.yml (already provided in
    deployment/docker/docker-compose.yml) for production deployment.

  - **X11 Forwarding**: Confirm the DISPLAY environment variable and
    /tmp/.X11-unix volume mounts are correctly configured for GUI
    forwarding.

  - **Data Persistence**: Verify named volumes (cloudregistration-data,
    cloudregistration-logs) are correctly configured for bind mounts to
    the host system.

  - **Resource Limits**: Ensure deploy section with cpus and memory
    limits is set appropriately for production.

  - **Health Check**: Confirm the healthcheck command (pgrep -f
    CloudRegistration) is reliable.

  - **Logging (Optional)**: If logrotate is included, ensure it\'s
    correctly configured to manage application logs.

  - **Monitoring (Optional)**: If prom/node-exporter is included, ensure
    it\'s correctly configured.

  - **Build Context**: Verify the build.context and dockerfile paths are
    correct relative to the docker-compose.yml file.

### Tests

- **Manual Testing (Deployment)**:

  - Build the Docker image: docker-compose build.

  - Run the application: docker-compose up -d.

  - Verify the GUI launches successfully on the host system via X11
    forwarding.

  - Test basic application functionality within the Docker container.

  - Verify that data and logs directories are created on the host and
    persistence works (e.g., save a project, exit container, restart,
    verify project is still there).

  - Verify log rotation (if implemented) is working.

  - Test stopping and restarting the container.

- **Automated Tests (if possible)**:

  - Basic Docker build validation (e.g., running docker build . in
    CI/CD).

  - Basic container startup and process check.

### Acceptance Criteria

- The docker-compose.yml and Dockerfile are finalized and correctly
  configured for GUI application deployment.

- The application can be successfully built and run as a Docker
  container.

- Data and logs persist across container restarts.

- The GUI is accessible from the host machine via X11 forwarding.

## Sub-Sprint 8.4: User Documentation

### Goal

Create comprehensive, clear, and user-friendly documentation covering
installation, basic usage, key features, and troubleshooting.

### User Stories

- **As a new user,** I want clear step-by-step instructions to install
  the application.

- **As a user,** I want to understand how to perform basic tasks like
  creating a project, importing scans, and performing manual/automatic
  registration.

- **As a user,** I want to find solutions to common problems or error
  messages.

### UI/UX

- **No direct UI/UX changes.** This is a non-code deliverable.

- **Help Menu (Optional future work)**: A \"Help\" menu item could link
  to the online documentation.

### Frontend (Component/View Layer)

- **No direct frontend code changes.**

### Backend (Core Logic/Services)

- **No direct backend code changes.**

### Deliverables

- **Installation Guide**: Detailed steps for installing the application
  (e.g., Docker deployment, local build setup for developers).

- **Getting Started Guide**: Walkthrough of core workflows:

  - Creating a new project.

  - Importing scans (E57, LAS).

  - Performing manual alignment (point selection, accept/cancel).

  - Performing automatic ICP alignment.

  - Performing target-based alignment.

  - Exporting results.

  - Viewing quality maps.

- **Feature Reference**: Explanations of advanced features (e.g.,
  BundleAdjustment, PerformanceProfiler, Coordinate Systems).

- **Troubleshooting Guide**: Common issues and their solutions (e.g.,
  \"File failed to load\", \"ICP not converging\", \"Memory warnings\").

- **API Documentation (for developers)**: High-level overview of the
  modular architecture and key interfaces (referencing the internal
  documentation generated in Sprint 5).

### Tests

- **Documentation Review**: Manual review by multiple team members for
  clarity, accuracy, completeness, and grammar.

- **User Acceptance Testing (UAT) with Documentation**: Provide early
  users with the documentation and observe if they can successfully use
  the application by following the guides.

### Acceptance Criteria

- Comprehensive user documentation is created and accessible.

- The documentation covers installation, core workflows, and
  troubleshooting.

- The documentation is clear, accurate, and easy to understand for the
  target audience.
