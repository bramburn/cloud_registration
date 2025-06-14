# Product Requirements Document: Point Cloud Application

Version: 1.3

Date: June 14, 2025

Author: Gemini

Status: Proposed

## 1. Introduction & Vision

### 1.1. Executive Summary

This document outlines the requirements for the Minimum Viable Product
(MVP) of a new, high-performance desktop application for point cloud
visualization, manipulation, and registration. The application will be
built from the ground up using C++, Qt6, and CMake, with a primary focus
on creating a robust, user-friendly, and extensible platform that serves
as a powerful open-source alternative to commercial software like FARO
SCENE, CloudCompare, and Leica Cyclone.

#### 1.1.1. Project Context

This project represents a major refactoring and rebuild of a previous
codebase. The existing code will be copied into a new branch named
restructure, which will serve as the baseline for this development
effort. The prior implementation will serve as a reference for proven
concepts, but this new version will address foundational issues and
strictly adhere to Test-Driven Development (TDD) principles from the
outset.

- **Reference Repository:** C:\\dev\\old_cloud_repo

### 1.2. Product Vision

To create an intuitive, professional-grade 3D point cloud application
that enables users to efficiently load, visualize, manipulate, and
register large datasets with industry-leading performance and a superior
user experience.

### 1.3. Target Audience & User Personas

#### **Primary Persona: Sarah, The Survey Technician**

- **Background:** 5+ years of experience with terrestrial laser
  scanning, proficient with FARO Scene and CloudCompare.

- **Goals:** Efficiently process daily scan data, perform registrations,
  and create accurate deliverables for clients.

- **Pain Points:** Cumbersome software interfaces, slow processing of
  large files, frustrating file format incompatibilities, and a steep
  learning curve for new tools.

- **Needs:** A reliable, fast, and intuitive tool that simplifies her
  core workflow.

#### **Secondary Persona: Michael, The Engineering Manager**

- **Background:** Oversees survey and construction projects, with
  occasional hands-on software use for review and approval.

- **Goals:** Quickly monitor project progress, review scan data quality,
  and approve deliverables without needing deep technical expertise.

- **Pain Points:** Software that is too complex for quick visualization
  tasks.

- **Needs:** An application with a simple interface for loading a
  project and visually inspecting the results.

## 2. Market Research & Competitive Analysis

  ----------------------------------------------------------------------------
  **Application**    **Strengths**       **UI/UX Patterns**  **Key Takeaways
                                                             for Our App**
  ------------------ ------------------- ------------------- -----------------
  **FARO SCENE**     Excellent automated Left sidebar        Adopt the
                     registration,       (project tree),     standard
                     professional        central 3D          three-panel
                     workflows, robust   viewport, bottom    layout. Focus on
                     hardware            properties panel.   a clear,
                     integration.                            professional
                                                             workflow.

  **CloudCompare**   Open-source, highly Modular docking     Provide powerful
                     flexible, extensive panels, highly      tools but
                     set of analysis and customizable but    organize them
                     measurement tools.  can be              logically. Avoid
                                         overwhelming.       UI clutter.

  **Leica Cyclone**  Strong data         Project-centric     A strong project
                     management for      dashboard,          file structure is
                     large survey        structured data     critical for user
                     projects,           management          confidence and
                     database-centric.   interface.          organization.

  **Autodesk ReCap** Seamless            Modern ribbon       Right-click
                     integration with    interface,          context menus are
                     BIM and cloud       context-sensitive   essential for an
                     workflows.          toolbars.           efficient
                                                             workflow.
  ----------------------------------------------------------------------------

**Key UI/UX Insights:** A successful application in this space must have
a project-centric workflow, utilize a standard three-panel layout,
provide powerful context menus, and give clear visual feedback (e.g.,
loading bars, status indicators). A UCS view cube is industry standard
for navigation.

## 3. UI/UX Guidelines

### 3.1. Visual Design Principles

- **Professional Aesthetic:** A dark theme is preferred to reduce eye
  strain during long sessions.

- **Information Hierarchy:** The most important information and tools
  must be visually prominent.

- **Consistency:** Interaction patterns must be uniform and predictable
  throughout the application.

### 3.2. Layout Standard (1920x1080)

+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\
\| Menu Bar \|\
+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\
\| \| \| \|\
\| \| \| \|\
\| Project/Scan \| \| Properties / \|\
\| Tree \| 3D Viewport \| Tools \|\
\| (Sidebar) \| \| (Dock Panel) \|\
\| \| \| \|\
\| \| \| \|\
\| \| \| \|\
+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+\
\| Status Bar \|\
+\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\-\--+

### 3.3. Color Scheme (Dark Theme)

- **Primary Background:** #2B2B2B

- **Secondary Background:** #3C3C3C

- **Primary Text:** #FFFFFF

- **Accent/Selection:** #0078D4 (Microsoft Blue)

- **Border:** #5A5A5A

## 4. Core Features & Functionality (MVP)

### 4.1. Project Management

- **FR-4.1.1: Project Creation & Loading:** Create projects in dedicated
  folders. The project consists of a primary project.json file for
  configuration and a project.sqlite file for transactional history.

- **FR-4.1.2: Scan Import:** At any time, users can import scans into
  the project. The import process adds a reference to the scan in the
  project structure and gives the user the choice to **Copy** or
  **Move** the source file into the project\'s /scans directory.
  Importing a scan does not load it into memory by default.

- **FR-4.1.3: Hierarchical Scan Organization:** A QTreeView sidebar
  shall be the primary interface for project organization.

  - **User Story:** *As a survey technician, I want to create and manage
    clusters so that I can organize scans into logical groups (e.g., by
    room or floor), making it easier to work with large projects.*

  - Users can create, rename, and delete folders (clusters).

  - Users can organize scans and clusters by dragging and dropping them
    within the tree.

- **FR-4.1.4: Cluster Locking for Registration Integrity:**

  - **User Story:** *As a survey technician, I want to lock a cluster so
    that all its internal scan positions are fixed relative to each
    other, ensuring registration integrity while still allowing the
    entire group to be moved.*

  - Users must be able to lock/unlock clusters via a context menu or
    icon.

  - When a cluster is locked, individual scans within it cannot be
    transformed.

  - A locked cluster can still be transformed as a single, rigid group.

  - The lock state must be visually indicated in the project tree (e.g.,
    with a lock icon).

### 4.2. 3D Visualization

- **FR-4.2.1: High-Performance Rendering:** OpenGL renderer with an
  Octree-based Level of Detail (LOD) system for handling large datasets.

- **FR-4.2.2: Camera Controls:** Intuitive orbit (MMB), pan (Shift+MMB),
  and zoom (scroll wheel).

- **FR-4.2.3: User Coordinate System (UCS) View Box:** A CAD-like view
  cube for snapping to standard views.

- **FR-4.2.4: Visibility Toggles:** Toggle visibility for individual
  scans and folders in the sidebar.

### 4.3. Point Cloud Manipulation & Interaction

- **FR-4.3.1: Point Cloud Transformation:** Move/rotate selected scans
  with a 3D gizmo or property panel.

- **FR-4.3.2: Point Cloud Loading States:** Visually distinguish and
  manage Loaded/Unloaded states for scans to control memory usage.

- **FR-4.3.3: Interactive Clipping Box:** A draggable 3D box to isolate
  regions of the point cloud.

- **FR-4.3.4: History / Undo:** Implement an undo system for
  fine-grained changes and a checkpoint system for reverting to major
  saved states.

### 4.4. Rendering Modes & Options

- **FR-4.4.1: Point Cloud Coloring:** Support **RGB**, **Intensity**,
  **Elevation**, and **Single Color** modes.

- **FR-4.4.2: X-Ray View:** Implement a transparency mode to visualize
  overlapping structures.

### 4.5. Supported File Formats

- **FR-4.5.1: Import:** Priority is **E57** and **PLY**. Add support for
  LAS/LAZ and PTS post-MVP.

- **FR-4.5.2: Export:** Export selected scans or the entire project to
  **E57** or **PLY** with all transformations applied.

## 5. Technology Stack & Architecture

- **Language:** C++ (C++17 or newer)

- **UI Framework:** Qt6

- **Database:** SQLite for transactional history.

- **Build System:** CMake

- **Dependency Management:** vcpkg

- **3D Graphics:** OpenGL 4.5+ Core Profile

- **Architecture:** Modular (Model-View-Presenter or MVVM) with clear
  separation of concerns.

- **Testing:** Google Test (gtest)

## 6. Phased Development Plan (MVP Epics)

**Phase 1: Foundation & Core Viewer (Sprints 1-4)**

- **Epic 1: Application Bootstrap (Sprint 1)**

  - **Goal:** Establish the core project structure, build system, and a
    runnable, empty application window.

  - **Tasks:**

    - Initialize Git repository; Set up CMakeLists.txt and vcpkg.

    - Define the initial modular directory structure (/src, /app, /core,
      /ui).

    - Create the main QMainWindow class with a menu bar, status bar, and
      placeholder layout.

    - Implement basic \"Exit\" and \"About\" dialogs.

  - **Acceptance Criteria:** The project compiles into a runnable GUI
    application that shows an empty main window which can be closed.

- **Epic 2: Project Management (Sprint 2)**

  - **Goal:** Implement the project file structure and the UI for
    managing projects.

  - **Tasks:**

    - Define the project.json (for static config) and project.sqlite
      (for history) specifications.

    - Implement logic for creating a new project (folder, project.json,
      and empty .sqlite database).

    - Implement \"Save Checkpoint\" and \"Load\" project functionality.

    - Create the sidebar QTreeView to display the project\'s folder
      structure.

  - **Acceptance Criteria:** Users can create, save, and load projects.
    The project folder contains both a .json and .sqlite file.

- **Epic 3: 3D Visualization Engine (Sprint 3)**

  - **Goal:** Render a point cloud in the 3D viewport.

  - **Tasks:**

    - Integrate an OpenGL widget into the main window\'s central area.

    - Implement basic file parsers for PLY and E57.

    - Implement the initial Octree-based data structure for LOD.

    - Write basic GLSL shaders for rendering points.

    - Add a \"Load Scan\" action that loads a file and displays it.

  - **Acceptance Criteria:** A user can load a PLY or E57 file from the
    menu, and it will be rendered in the 3D view.

- **Epic 4: Navigation & Controls (Sprint 4)**

  - **Goal:** Enable user interaction with the 3D scene.

  - **Tasks:**

    - Implement a camera class with orbit, pan, and zoom logic.

    - Connect mouse events from the OpenGL widget to the camera
      controls.

    - Create the UCS view cube widget and overlay it on the viewport.

    - Implement logic to snap the camera to standard views when the cube
      is clicked.

  - **Acceptance Criteria:** The user can freely navigate around the
    loaded point cloud using the mouse and snap to standard views using
    the UCS cube.

**Phase 2: Interaction & Tools (Sprints 5-8)**

- **Epic 5: Point Cloud State & Transformation (Sprint 5)**

  - **Goal:** Allow users to manage scan visibility and positioning in a
    controlled, transactional manner.

  - **Tasks:**

    - **Transactional Actions:** Implement a system where user actions
      (transform, move between folders, rename, lock/unlock) are
      recorded as individual transactions in the project.sqlite
      database.

    - **Locking & Visibility:** Implement \"Lock\" state/icon for scans
      and clusters in the project tree (via a context menu). Locked
      items cannot be selected for transformation. If a cluster is
      locked, its child scans cannot be transformed individually.

    - **Transform Mode:** Add a \"Transform Mode\" toggle button to the
      main toolbar.

    - **Gizmo Logic:** Implement a 3D transformation gizmo that only
      appears when an item is selected and \"Transform Mode\" is active.
      The gizmo must be disabled if the selected item (or its parent
      cluster) is locked.

    - **Checkpoint Save:** Implement a \"Save Checkpoint\" button. This
      action commits all pending transactions since the last checkpoint
      to the database as a single, major save state.

  - **Acceptance Criteria:** All specified user actions are recorded in
    the SQLite database. A user can create a \"checkpoint\" to save the
    project state.

- **Epic 6: Advanced Rendering (Sprint 6)**

  - **Goal:** Provide multiple visualization modes to aid in data
    inspection.

  - **Tasks:**

    - Add a toolbar dropdown to select the active rendering mode.

    - Modify the GLSL shaders to accept a uniform variable for the
      render mode.

    - Implement shader logic to switch between coloring methods: RGB,
      Intensity, Elevation.

    - Create a customizable color ramp for the Elevation mode.

    - Implement an X-Ray/Transparency mode using alpha blending.

  - **Acceptance Criteria:** The user can select any of the specified
    rendering modes from the UI, and the point cloud display updates in
    real-time.

- **Epic 7: Spatial Tools (Sprint 7)**

  - **Goal:** Provide both temporary and persistent spatial tools for
    isolating regions of interest.

  - **Tasks:**

    - **Trimming Box (Ad-hoc):**

      - Implement a temporary ClippingBox class. This tool is activated
        via a toolbar button, is not saved with the project, and is used
        for quick, disposable viewing tasks.

      - Render the box as an interactable wireframe cube with handles on
        each face.

      - Pass its boundaries to the point cloud shader to discard points
        outside the volume.

    - **Slicing Box (Persistent):**

      - Implement a persistent SlicingBox entity. For the MVP, only one
        instance is allowed per project.

      - Add a \"Create Slicing Box\" action to the context menu for the
        project root and top-level folders.

      - When created, add a \"Slicer\" item to the QTreeView under its
        parent.

      - This tree item must have a checkbox for toggling visibility and
        a context menu option for deletion.

      - The state of the Slicing Box (position, size, visibility,
        parent) must be recorded as a transaction in the project.sqlite
        database and saved with checkpoints.

  - **Acceptance Criteria:** A user can enable a temporary trimming box
    to quickly isolate a view. A user can right-click a folder/project
    to create a persistent Slicing Box, which then appears in the
    project tree. The Slicing Box\'s visibility can be toggled from the
    tree, and its state is saved with the project.

- **Epic 8: Finalizing MVP (Sprint 8)**

  - **Goal:** Complete the core MVP feature set with robust history
    management and data export capabilities.

  - **Tasks:**

    - **Undo/Redo:** Implement a standard undo/redo stack that pops and
      reapplies transactions from the project.sqlite database.

    - **History Window:** Create a new dockable window that displays a
      log of all major \"Checkpoints\". This window should allow a user
      to revert the entire project to a selected checkpoint.

    - **Export:** Create an \"Export\" dialog. This should allow format
      selection (E57, PLY) and let the user define the scope: selected
      scan(s), a specific folder/cluster, or the entire project. The
      export will write the data *at its current state*, applying all
      transformations.

  - **Acceptance Criteria:** A user can undo/redo individual changes. A
    user can open a history window and revert the project to a previous
    major checkpoint. A user can export the scene.

## 7. Risk Assessment & Mitigation

  -----------------------------------------------------------------------
  **Risk Category**       **Risk Description**    **Mitigation Strategy**
  ----------------------- ----------------------- -----------------------
  **Technical**           Performance issues with Implement streaming,
                          very large (\>100M      progressive loading,
                          points) files.          and aggressive Octree
                                                  LOD from the start.

  **Technical**           Cross-platform          Conduct continuous
                          compatibility issues    integration testing on
                          (e.g., OpenGL drivers). Windows and Linux. Have
                                                  fallback shader
                                                  options.

  **User Experience**     The application is      Adhere to standard UI
                          perceived as too        patterns. Create
                          complex for new users.  comprehensive tutorials
                                                  and contextual
                                                  tooltips.

  **Project**             Delays in core feature  Focus strictly on MVP
                          development impact      features. Defer
                          timeline.               non-essential
                                                  enhancements to
                                                  post-MVP.
  -----------------------------------------------------------------------

## 8. Future Enhancements (Post-MVP)

- **Advanced Registration:** Implement manual (3-point) and automated
  (ICP-based) registration algorithms with quality reporting.

- **Measurement Tools:** Add tools for measuring distance, area, and
  coordinates.

- **Collaboration:** Explore cloud project sharing and multi-user
  viewing capabilities.

- **Integrations:** Develop plugins for CAD software (e.g., Revit,
  AutoCAD) and direct integration with survey equipment APIs.
