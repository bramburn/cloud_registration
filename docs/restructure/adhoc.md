# Backlog: src/ File Restructuring

## Introduction

This backlog outlines the plan to restructure the C++ source and header
files currently residing directly in the src/ directory. The goal is to
move these files into their appropriate module subdirectories
(src/module_name/src/ for implementation and
src/module_name/include/module_name/ for public headers), aligning the
physical file structure with the logical modular architecture defined in
the CMake build system. This will significantly improve code
organization, maintainability, discoverability, and reduce potential
future conflicts or confusion.

## User Stories

### User Story 1: Core Module Restructuring

- **Description**: As a developer, I want to relocate core project
  management and utility files, so that the src/core/ module accurately
  reflects its purpose of housing foundational project structures and
  services, separate from application-specific logic or UI.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - src/ProjectStateService.cpp/h interacts with src/sqlitemanager.cpp/h
    and src/scanimportmanager.cpp/h (which will also be moved), and
    src/projecttreemodel.h (also moved). It relies on
    src/core/project.h.

  - src/projectmanager.cpp/h acts as a facade, delegating to
    ProjectStateService and RecentProjectsManager. It uses
    src/core/project.h.

  - src/progressmanager.cpp/h and src/sqlitemanager.cpp/h are
    fundamental utilities.

  - All these components are used by src/app/mainwindow.cpp and
    src/MainPresenter.cpp.

- **Acceptance Criteria**:

  - All files specified in this user story are successfully moved to
    src/core/src/ and src/core/include/core/ respectively.

  - The Core static library compiles without errors or warnings.

  - Any other modules or the main application that depend on these files
    link and build successfully.

  - No existing functionality is broken.

  - All relevant unit tests for Core components pass.

- **Testing Plan**:

  - **Test Case 1.1**: Build Core library after relocation.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation of the Core library.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 1.2**: Run existing Core unit tests (e.g.,
    VoxelGridFilterTests, PerformanceProfilerTests).

    - **Test Data**: N/A (uses internal test data).

    - **Expected Result**: All Core unit tests pass.

    - **Testing Tool**: GTest, CTest.

  - **Test Case 1.3**: Build and run the main CloudRegistration
    executable.

    - **Test Data**: Existing project files (if any), sample point
      clouds.

    - **Expected Result**: Application launches successfully, project
      management functions (create/open/save) work as expected.

    - **Testing Tool**: Manual verification, system tests.

### User Story 2: UI Module Restructuring

- **Description**: As a developer, I want to relocate all user
  interface-related files, so that the src/ui/ module contains all
  dialogs, widgets, and UI-specific models, promoting a clear separation
  of presentation logic from other concerns.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - src/ui/AlignmentControlPanel.cpp/h, src/ui/ExportDialog.cpp/h,
    src/ui/ICPProgressWidget.cpp/h, src/ui/UIThemeManager.cpp/h,
    src/ui/UserPreferences.cpp/h, src/ui/ViewerToolbar.cpp/h,
    src/ui/WorkflowProgressWidget.cpp/h,
    src/ui/TargetDetectionDialog.cpp/h, src/ui/confirmationdialog.h (if
    present in src/).

  - These UI components interact with other modules (e.g.,
    src/registration/AlignmentEngine.h for AlignmentControlPanel,
    src/export/IFormatWriter.h for ExportDialog).

  - src/projecttreemodel.cpp/h will be the model for SidebarWidget.

- **Acceptance Criteria**:

  - All files specified are successfully moved to src/ui/src/ and
    src/ui/include/ui/.

  - The UI static library compiles without errors or warnings.

  - The main application\'s UI is fully functional and responsive.

  - All relevant unit tests for UI components pass.

- **Testing Plan**:

  - **Test Case 2.1**: Build UI library after relocation.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation of the UI library.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 2.2**: Run existing UI unit tests (e.g.,
    RecentProjectsManagerTests, UIEnhancementTests).

    - **Test Data**: N/A.

    - **Expected Result**: All UI unit tests pass.

    - **Testing Tool**: GTest, CTest.

  - **Test Case 2.3**: Manual UI workflow.

    - **Test Data**: N/A.

    - **Expected Result**: All UI elements (dialogs, sidebars, controls)
      render correctly and interactions work as expected.

    - **Testing Tool**: Manual verification.

### User Story 3: Parsers Module Restructuring

- **Description**: As a developer, I want to relocate all file format
  parsing logic, so that the src/parsers/ module centralizes all related
  components, such as E57 and LAS parsers.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - src/E57ParserCore.cpp/h and src/e57parserlib.cpp/h rely on external
    E57Format and XercesC libraries (if available).

  - src/lasparser.cpp/h is for LAS file parsing.

  - src/e57parserlib.cpp/h acts as a Qt wrapper for E57ParserCore.

  - These parsers are used by src/pointcloudloadmanager.cpp and
    src/MainPresenter.cpp.

- **Acceptance Criteria**:

  - All files specified are successfully moved to src/parsers/src/ and
    src/parsers/include/parsers/.

  - The Parsers static library compiles without errors or warnings.

  - File loading functionality in the main application works as expected
    for E57 and LAS formats.

  - All relevant unit tests for Parsers components pass.

- **Testing Plan**:

  - **Test Case 3.1**: Build Parsers library after relocation.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation of the Parsers
      library.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 3.2**: Run existing Parsers unit tests (e.g.,
    E57LinkageTest, LasParserTests, E57ParserTests, E57ParserCoreTests).

    - **Test Data**: Sample E57 and LAS files.

    - **Expected Result**: All Parsers unit tests pass.

    - **Testing Tool**: GTest, CTest.

  - **Test Case 3.3**: Manual file loading.

    - **Test Data**: Sample E57 and LAS files.

    - **Expected Result**: Application successfully loads and displays
      E57 and LAS files.

    - **Testing Tool**: Manual verification.

### User Story 4: Application and Load Management Module Restructuring

- **Description**: As a developer, I want to relocate general
  application components and point cloud load management logic, so that
  the src/app/ module contains the main executable and its direct
  dependencies for overall application flow.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - src/mainwindow.cpp/h is the main window, connecting to MainPresenter
    and various UI elements.

  - src/MainPresenter.cpp/h coordinates the application, interacting
    with IMainView, IE57Parser, IPointCloudViewer, ProjectManager,
    PointCloudLoadManager, and ScanImportManager.

  - src/MainPresenterExtensions.cpp contains additional slots for
    MainPresenter.

  - src/MainViewAdapter.cpp/h bridges MainWindow to IMainView.

  - src/pointcloudloadmanager.cpp/h and src/scanimportmanager.cpp/h
    handle file loading and import workflows, interacting with parsers
    and project managers.

- **Acceptance Criteria**:

  - All files specified are successfully moved to src/app/src/ and
    src/app/include/app/.

  - The CloudRegistration executable compiles and links successfully.

  - The application launches and all core functionalities (project
    creation, file opening, scan import, UI navigation) work correctly.

  - Any existing integration tests for App components pass.

- **Testing Plan**:

  - **Test Case 4.1**: Build main CloudRegistration executable.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation and linking of the
      executable.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 4.2**: Run existing integration tests (e.g.,
    EndToEndIntegrationTests, MainPresenterTests).

    - **Test Data**: Mocked or real dependencies.

    - **Expected Result**: All integration tests pass.

    - **Testing Tool**: GTest, CTest.

  - **Test Case 4.3**: Manual application launch and basic workflow.

    - **Test Data**: N/A.

    - **Expected Result**: Application launches, main window is
      displayed, menu items are functional, and basic file/project
      operations can be initiated.

    - **Testing Tool**: Manual verification.

### User Story 5: Interfaces Module Consolidation

- **Description**: As a developer, I want to consolidate all interface
  definitions, so that the src/interfaces/ module serves as a single,
  central source of truth for all abstract contracts, reinforcing
  dependency inversion.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - IMainView.h is implemented by MainViewAdapter and used by
    MainPresenter.

  - IPointCloudViewer.h is implemented by PointCloudViewerWidget and
    used by MainPresenter.

  - IE57Parser.h is implemented by e57parserlib.h and used by
    MainPresenter.

  - IE57Parser.cpp is the accompanying implementation for the Q_OBJECT
    macro in IE57Parser.h.

- **Acceptance Criteria**:

  - All interface header files are successfully moved to
    src/interfaces/include/interfaces/.

  - The accompanying .cpp files for interfaces (like IE57Parser.cpp) are
    moved to src/interfaces/src/.

  - The Interfaces static library compiles without errors or warnings.

  - All modules that implement or use these interfaces build and link
    successfully.

  - No existing unit tests or application functionality relying on these
    interfaces are broken.

- **Testing Plan**:

  - **Test Case 5.1**: Build Interfaces library after relocation.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation of the Interfaces
      library.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 5.2**: Build all dependent modules (e.g., App, UI,
    Parsers, Rendering).

    - **Test Data**: N/A.

    - **Expected Result**: All dependent modules compile and link
      successfully without new errors related to interface usage.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 5.3**: Run relevant unit tests that rely on interface
    mocks.

    - **Test Data**: Mock objects for interfaces.

    - **Expected Result**: Tests involving mocks of IMainView,
      IPointCloudViewer, IE57Parser pass.

    - **Testing Tool**: GTest, CTest.

### User Story 6: Rendering Module Restructuring

- **Description**: As a developer, I want to relocate the core 3D
  visualization widget, so that the src/rendering/ module contains all
  graphics-related components.

- **Actions to Undertake**: See detailed breakdown below.

- **References between Files**:

  - src/pointcloudviewerwidget.cpp/h relies on OpenGL, Qt GUI, and other
    rendering utilities. It implements IPointCloudViewer.

  - Used by src/app/mainwindow.cpp and src/MainPresenter.cpp.

- **Acceptance Criteria**:

  - Files are successfully moved to src/rendering/src/ and
    src/rendering/include/rendering/.

  - The Rendering static library compiles without errors or warnings.

  - The 3D viewer in the main application is fully functional,
    displaying point clouds correctly, and camera controls work.

  - All relevant unit tests for Rendering components pass.

- **Testing Plan**:

  - **Test Case 6.1**: Build Rendering library after relocation.

    - **Test Data**: Relocated source/header files.

    - **Expected Result**: Successful compilation of the Rendering
      library.

    - **Testing Tool**: CMake, compiler output.

  - **Test Case 6.2**: Run existing Rendering unit tests (e.g.,
    PointCloudViewerRenderingR4Tests).

    - **Test Data**: N/A.

    - **Expected Result**: All Rendering unit tests pass.

    - **Testing Tool**: GTest, CTest.

  - **Test Case 6.3**: Manual 3D viewer verification.

    - **Test Data**: Sample point cloud files.

    - **Expected Result**: Load a point cloud file and interact with the
      3D viewer (rotate, pan, zoom). Verify correct rendering and
      responsiveness.

    - **Testing Tool**: Manual verification.

## Actions to Undertake (Detailed Breakdown)

For each file, the following steps will be performed:

1.  **Move Files**: Physically move the .cpp file to
    src/\<module_name\>/src/ and the .h file to
    src/\<module_name\>/include/\<module_name\>/.

2.  **Update CMakeLists.txt**:

    - Remove the file from the APP_SOURCES/APP_HEADERS in
      src/app/CMakeLists.txt (or any other CMakeLists.txt where it was
      temporarily included from src/).

    - Add the file to the \_SOURCES and \_HEADERS lists in the
      \<module_name\>/CMakeLists.txt.

    - Adjust target_include_directories in the module\'s CMakeLists.txt
      if necessary to reflect the new internal paths.

3.  **Update #include Directives**: Modify any #include directives
    within the moved files to use the correct modular include paths
    (e.g., src/core/project.h becomes #include \"core/project.h\" if
    src/core/include is in the public include path).

4.  **Update Call Sites**: Update #include directives in any other files
    that use the moved components.

**Files to be Restructured:**

- **File**: src/ProjectStateService.cpp

  - **New Location**: src/core/src/ProjectStateService.cpp

  - **CMake Changes**:

    - Remove from APP_SOURCES in src/app/CMakeLists.txt.

    - Add to CORE_SOURCES in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files (e.g.,
    src/projectmanager.cpp).

- **File**: src/ProjectStateService.h

  - **New Location**: src/core/include/core/ProjectStateService.h

  - **CMake Changes**:

    - Remove from APP_HEADERS in src/app/CMakeLists.txt.

    - Add to CORE_HEADERS in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/projectmanager.cpp

  - **New Location**: src/core/src/projectmanager.cpp

  - **CMake Changes**:

    - Remove from APP_SOURCES in src/app/CMakeLists.txt.

    - Add to CORE_SOURCES in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/projectmanager.h

  - **New Location**: src/core/include/core/projectmanager.h

  - **CMake Changes**:

    - Remove from APP_HEADERS in src/app/CMakeLists.txt.

    - Add to CORE_HEADERS in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/progressmanager.cpp

  - **New Location**: src/core/src/progressmanager.cpp

  - **CMake Changes**:

    - Remove from APP_SOURCES in src/app/CMakeLists.txt.

    - Add to CORE_SOURCES in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/progressmanager.h

  - **New Location**: src/core/include/core/progressmanager.h

  - **CMake Changes**:

    - Remove from APP_HEADERS in src/app/CMakeLists.txt.

    - Add to CORE_HEADERS in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/sqlitemanager.cpp

  - **New Location**: src/core/src/sqlitemanager.cpp

  - **CMake Changes**:

    - Remove from UI_SOURCES in src/ui/CMakeLists.txt.

    - Add to CORE_SOURCES in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files (e.g.,
    src/ProjectStateService.cpp).

- **File**: src/sqlitemanager.h

  - **New Location**: src/core/include/core/sqlitemanager.h

  - **CMake Changes**:

    - Remove from UI_HEADERS in src/ui/CMakeLists.txt.

    - Add to CORE_HEADERS in src/core/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/createprojectdialog.cpp

  - **New Location**: src/ui/src/createprojectdialog.cpp

  - **CMake Changes**:

    - Modify UI_SOURCES in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in src/projecthubwidget.cpp.

- **File**: src/createprojectdialog.h

  - **New Location**: src/ui/include/ui/createprojectdialog.h

  - **CMake Changes**:

    - Modify UI_HEADERS in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in src/projecthubwidget.h.

- **File**: src/scanimportdialog.cpp

  - **New Location**: src/ui/src/scanimportdialog.cpp

  - **CMake Changes**:

    - Modify UI_SOURCES in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/scanimportdialog.h

  - **New Location**: src/ui/include/ui/scanimportdialog.h

  - **CMake Changes**:

    - Modify UI_HEADERS in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/sidebarwidget.cpp

  - **New Location**: src/ui/src/sidebarwidget.cpp

  - **CMake Changes**:

    - Modify UI_SOURCES in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/sidebarwidget.h

  - **New Location**: src/ui/include/ui/sidebarwidget.h

  - **CMake Changes**:

    - Modify UI_HEADERS in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/projecttreemodel.cpp

  - **New Location**: src/ui/src/projecttreemodel.cpp

  - **CMake Changes**:

    - Modify UI_SOURCES in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/projecttreemodel.h

  - **New Location**: src/ui/include/ui/projecttreemodel.h

  - **CMake Changes**:

    - Modify UI_HEADERS in src/ui/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/lasparser.cpp

  - **New Location**: src/parsers/src/lasparser.cpp

  - **CMake Changes**:

    - Modify PARSERS_SOURCES in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/lasparser.h

  - **New Location**: src/parsers/include/parsers/lasparser.h

  - **CMake Changes**:

    - Modify PARSERS_HEADERS in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/E57ParserCore.cpp

  - **New Location**: src/parsers/src/E57ParserCore.cpp

  - **CMake Changes**:

    - Modify PARSERS_SOURCES in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/E57ParserCore.h

  - **New Location**: src/parsers/include/parsers/E57ParserCore.h

  - **CMake Changes**:

    - Modify PARSERS_HEADERS in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/e57parserlib.cpp

  - **New Location**: src/parsers/src/e57parserlib.cpp

  - **CMake Changes**:

    - Modify PARSERS_SOURCES in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/e57parserlib.h

  - **New Location**: src/parsers/include/parsers/e57parserlib.h

  - **CMake Changes**:

    - Modify PARSERS_HEADERS in src/parsers/CMakeLists.txt to reflect
      the new path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/mainwindow.cpp

  - **New Location**: src/app/src/mainwindow.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/mainwindow.h

  - **New Location**: src/app/include/app/mainwindow.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/mainwindow_simple.cpp

  - **New Location**: src/app/src/mainwindow_simple.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

  - **Note**: Evaluate if this file is still needed. If not, delete it.

- **File**: src/mainwindow_simple.h

  - **New Location**: src/app/include/app/mainwindow_simple.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

  - **Note**: Evaluate if this file is still needed. If not, delete it.

- **File**: src/MainPresenter.cpp

  - **New Location**: src/app/src/MainPresenter.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/MainPresenter.h

  - **New Location**: src/app/include/app/MainPresenter.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/MainPresenterExtensions.cpp

  - **New Location**: src/app/src/MainPresenterExtensions.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/MainViewAdapter.cpp

  - **New Location**: src/app/src/MainViewAdapter.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/MainViewAdapter.h

  - **New Location**: src/app/include/app/MainViewAdapter.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.Lists to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/pointcloudloadmanager.cpp

  - **New Location**: src/app/src/pointcloudloadmanager.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/pointcloudloadmanager.h

  - **New Location**: src/app/include/app/pointcloudloadmanager.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/scanimportmanager.cpp

  - **New Location**: src/app/src/scanimportmanager.cpp

  - **CMake Changes**:

    - Modify APP_SOURCES in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/scanimportmanager.h

  - **New Location**: src/app/include/app/scanimportmanager.h

  - **CMake Changes**:

    - Modify APP_HEADERS in src/app/CMakeLists.txt to reflect the new
      path.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/IMainView.h

  - **New Location**: src/interfaces/include/interfaces/IMainView.h

  - **CMake Changes**:

    - Remove from UI_HEADERS in src/ui/CMakeLists.txt.

    - Add to INTERFACES_HEADERS in src/interfaces/CMakeLists.txt.

  - **Include Changes**: Update #includes in src/app/MainPresenter.h,
    src/app/MainViewAdapter.h, and src/app/mainwindow.h.

- **File**: src/IPointCloudViewer.h

  - **New Location**:
    src/interfaces/include/interfaces/IPointCloudViewer.h

  - **CMake Changes**:

    - Remove from UI_HEADERS in src/ui/CMakeLists.txt.

    - Add to INTERFACES_HEADERS in src/interfaces/CMakeLists.txt.

  - **Include Changes**: Update #includes in src/app/MainPresenter.h,
    src/app/mainwindow.h, and src/rendering/pointcloudviewerwidget.h.

- **File**: src/IE57Parser.cpp

  - **New Location**: src/interfaces/src/IE57Parser.cpp

  - **CMake Changes**:

    - Modify PARSERS_SOURCES in src/parsers/CMakeLists.txt to remove the
      old path.

    - Add to INTERFACES_SOURCES in src/interfaces/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/IE57Parser.h

  - **New Location**: src/interfaces/include/interfaces/IE57Parser.h

  - **CMake Changes**:

    - Modify PARSERS_HEADERS in src/parsers/CMakeLists.txt to remove the
      old path.

    - Add to INTERFACES_HEADERS in src/interfaces/CMakeLists.txt.

  - **Include Changes**: Update #includes in src/parsers/e57parserlib.h.

- **File**: src/IE57Writer.h

  - **New Location**: src/interfaces/include/interfaces/IE57Writer.h

  - **CMake Changes**:

    - Remove from PARSERS_HEADERS in src/parsers/CMakeLists.txt. (It was
      incorrectly placed under parsers, it\'s an interface for writing).

    - Add to INTERFACES_HEADERS in src/interfaces/CMakeLists.txt.

  - **Include Changes**: Update #includes in src/app/MainPresenter.h.

- **File**: src/pointcloudviewerwidget.cpp

  - **New Location**: src/rendering/src/pointcloudviewerwidget.cpp

  - **CMake Changes**:

    - Remove from APP_SOURCES in src/app/CMakeLists.txt.

    - Add to RENDERING_SOURCES in src/rendering/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

- **File**: src/pointcloudviewerwidget.h

  - **New Location**:
    src/rendering/include/rendering/pointcloudviewerwidget.h

  - **CMake Changes**:

    - Remove from APP_HEADERS in src/app/CMakeLists.txt.

    - Add to RENDERING_HEADERS in src/rendering/CMakeLists.txt.

  - **Include Changes**: Update #includes in affected files.

## References between Files

  -----------------------------------------------------------------------------------------------------------------------
  **Current Path**                   **Proposed New Path (Example)**                            **Key Relationships**
  ---------------------------------- ---------------------------------------------------------- -------------------------
  src/ProjectStateService.cpp/h      src/core/src/ProjectStateService.cpp                       Used by ProjectManager.
                                     src/core/include/core/ProjectStateService.h                Manages SQLiteManager,
                                                                                                ScanImportManager,
                                                                                                ProjectTreeModel. Relies
                                                                                                on src/core/project.h.

  src/projectmanager.cpp/h           src/core/src/projectmanager.cpp                            Facade for
                                     src/core/include/core/projectmanager.h                     project-related
                                                                                                operations. Uses
                                                                                                ProjectStateService,
                                                                                                RecentProjectsManager.
                                                                                                Used by MainPresenter.

  src/progressmanager.cpp/h          src/core/src/progressmanager.cpp                           Global progress tracking.
                                     src/core/include/core/progressmanager.h                    Used by MainPresenter,
                                                                                                PointCloudLoadManager,
                                                                                                MainWindow.

  src/sqlitemanager.cpp/h            src/core/src/sqlitemanager.cpp                             Database operations. Used
                                     src/core/include/core/sqlitemanager.h                      by ProjectStateService.

  src/createprojectdialog.cpp/h      src/ui/src/createprojectdialog.cpp                         UI dialog. Used by
                                     src/ui/include/ui/createprojectdialog.h                    ProjectHubWidget.

  src/scanimportdialog.cpp/h         src/ui/src/scanimportdialog.cpp                            UI dialog. Used by
                                     src/ui/include/ui/scanimportdialog.h                       MainWindow.

  src/sidebarwidget.cpp/h            src/ui/src/sidebarwidget.cpp                               UI widget. Uses
                                     src/ui/include/ui/sidebarwidget.h                          ProjectTreeModel, emits
                                                                                                signals to MainPresenter.

  src/projecttreemodel.cpp/h         src/ui/src/projecttreemodel.cpp                            UI model for
                                     src/ui/include/ui/projecttreemodel.h                       SidebarWidget. Uses
                                                                                                SQLiteManager.

  src/lasparser.cpp/h                src/parsers/src/lasparser.cpp                              LAS file parsing. Used by
                                     src/parsers/include/parsers/lasparser.h                    PointCloudLoadManager.

  src/E57ParserCore.cpp/h            src/parsers/src/E57ParserCore.cpp                          Core E57 parsing logic.
                                     src/parsers/include/parsers/E57ParserCore.h                Used by e57parserlib.

  src/e57parserlib.cpp/h             src/parsers/src/e57parserlib.cpp                           Qt wrapper for
                                     src/parsers/include/parsers/e57parserlib.h                 E57ParserCore. Implements
                                                                                                IE57Parser. Used by
                                                                                                MainPresenter.

  src/mainwindow.cpp/h               src/app/src/mainwindow.cpp                                 Main application window.
                                     src/app/include/app/mainwindow.h                           Implements IMainView.
                                                                                                Contains
                                                                                                PointCloudViewerWidget.

  src/mainwindow_simple.cpp/h        src/app/src/mainwindow_simple.cpp                          Simple main window
                                     src/app/include/app/mainwindow_simple.h                    (likely deprecated).

  src/MainPresenter.cpp/h            src/app/src/MainPresenter.cpp                              Main application logic.
                                     src/app/include/app/MainPresenter.h                        Uses IMainView,
                                                                                                IE57Parser,
                                                                                                IPointCloudViewer,
                                                                                                ProjectManager,
                                                                                                PointCloudLoadManager.

  src/MainPresenterExtensions.cpp    src/app/src/MainPresenterExtensions.cpp                    Extensions for
                                                                                                MainPresenter.

  src/MainViewAdapter.cpp/h          src/app/src/MainViewAdapter.cpp                            Adapter for MainWindow to
                                     src/app/include/app/MainViewAdapter.h                      IMainView.

  src/pointcloudloadmanager.cpp/h    src/app/src/pointcloudloadmanager.cpp                      Point cloud loading
                                     src/app/include/app/pointcloudloadmanager.h                manager. Uses parsers.
                                                                                                Used by MainPresenter,
                                                                                                MainWindow.

  src/scanimportmanager.cpp/h        src/app/src/scanimportmanager.cpp                          Scan import manager. Used
                                     src/app/include/app/scanimportmanager.h                    by MainPresenter,
                                                                                                MainWindow.

  src/IMainView.h                    src/interfaces/include/interfaces/IMainView.h              Interface. Implemented by
                                                                                                MainViewAdapter. Used by
                                                                                                MainPresenter.

  src/IPointCloudViewer.h            src/interfaces/include/interfaces/IPointCloudViewer.h      Interface. Implemented by
                                                                                                PointCloudViewerWidget.
                                                                                                Used by MainPresenter.

  src/IE57Parser.cpp/h               src/interfaces/src/IE57Parser.cpp                          Interface. Implemented by
                                     src/interfaces/include/interfaces/IE57Parser.h             e57parserlib.h. Used by
                                                                                                MainPresenter.

  src/IE57Writer.h                   src/interfaces/include/interfaces/IE57Writer.h             Interface. Used by
                                                                                                MainPresenter,
                                                                                                PointCloudExporter.

  src/pointcloudviewerwidget.cpp/h   src/rendering/src/pointcloudviewerwidget.cpp               3D viewer implementation.
                                     src/rendering/include/rendering/pointcloudviewerwidget.h   Implements
                                                                                                IPointCloudViewer. Used
                                                                                                by MainWindow.
  -----------------------------------------------------------------------------------------------------------------------

## List of Files being Created (Moved)

This section lists the files that will be moved and their new locations.
No entirely new functionality-related files are being created, only
reorganized.

- **File 1**: src/core/src/ProjectStateService.cpp

  - **Purpose**: Implementation of the ProjectStateService to manage
    active project state.

  - **Contents**: C++ source code for project loading, saving, and state
    management logic.

  - **Relationships**: Depends on
    src/core/include/core/ProjectStateService.h,
    src/core/include/core/sqlitemanager.h,
    src/app/include/app/scanimportmanager.h,
    src/ui/include/ui/projecttreemodel.h.

- **File 2**: src/core/include/core/ProjectStateService.h

  - **Purpose**: Header for the ProjectStateService, defining its public
    interface.

  - **Contents**: C++ header with class definition for
    ProjectStateService.

  - **Relationships**: Included by
    src/core/include/core/projectmanager.h.

- **File 3**: src/core/src/projectmanager.cpp

  - **Purpose**: Implementation of the ProjectManager facade for
    project-related operations.

  - **Contents**: C++ source code for creating, loading, and managing
    projects, delegating to ProjectStateService and
    RecentProjectsManager.

  - **Relationships**: Depends on
    src/core/include/core/projectmanager.h,
    src/core/include/core/ProjectStateService.h,
    src/ui/include/ui/recentprojectsmanager.h.

- **File 4**: src/core/include/core/projectmanager.h

  - **Purpose**: Header for the ProjectManager facade.

  - **Contents**: C++ header with class definition for ProjectManager.

  - **Relationships**: Included by src/app/include/app/MainPresenter.h.

- **File 5**: src/core/src/progressmanager.cpp

  - **Purpose**: Implementation of the ProgressManager for tracking
    long-running operations.

  - **Contents**: C++ source code for starting, updating, and finishing
    progress indications.

  - **Relationships**: Depends on
    src/core/include/core/progressmanager.h.

- **File 6**: src/core/include/core/progressmanager.h

  - **Purpose**: Header for the ProgressManager.

  - **Contents**: C++ header with class definition for ProgressManager.

  - **Relationships**: Used by src/app/include/app/mainwindow.h.

- **File 7**: src/core/src/sqlitemanager.cpp

  - **Purpose**: Implementation of the SQLiteManager for database
    interactions.

  - **Contents**: C++ source code for SQLite database operations.

  - **Relationships**: Depends on src/core/include/core/sqlitemanager.h.

- **File 8**: src/core/include/core/sqlitemanager.h

  - **Purpose**: Header for the SQLiteManager.

  - **Contents**: C++ header with class definition for SQLiteManager.

  - **Relationships**: Included by
    src/core/include/core/ProjectStateService.h,
    src/ui/include/ui/projecttreemodel.h.

- **File 9**: src/ui/src/createprojectdialog.cpp

  - **Purpose**: Implementation of the dialog for creating new projects.

  - **Contents**: C++ source code for the UI and logic of the
    CreateProjectDialog.

  - **Relationships**: Depends on
    src/ui/include/ui/createprojectdialog.h.

- **File 10**: src/ui/include/ui/createprojectdialog.h

  - **Purpose**: Header for the CreateProjectDialog.

  - **Contents**: C++ header with class definition for
    CreateProjectDialog.

  - **Relationships**: Included by src/ui/src/projecthubwidget.cpp.

- **File 11**: src/ui/src/scanimportdialog.cpp

  - **Purpose**: Implementation of the dialog for importing scan files.

  - **Contents**: C++ source code for the UI and logic of the
    ScanImportDialog.

  - **Relationships**: Depends on src/ui/include/ui/scanimportdialog.h.

- **File 12**: src/ui/include/ui/scanimportdialog.h

  - **Purpose**: Header for the ScanImportDialog.

  - **Contents**: C++ header with class definition for ScanImportDialog.

  - **Relationships**: Included by src/app/include/app/mainwindow.h.

- **File 13**: src/ui/src/sidebarwidget.cpp

  - **Purpose**: Implementation of the project sidebar tree view.

  - **Contents**: C++ source code for the UI and logic of the
    SidebarWidget.

  - **Relationships**: Depends on src/ui/include/ui/sidebarwidget.h,
    src/ui/include/ui/projecttreemodel.h,
    src/core/include/core/sqlitemanager.h,
    src/core/include/core/projectmanager.h.

- **File 14**: src/ui/include/ui/sidebarwidget.h

  - **Purpose**: Header for the SidebarWidget.

  - **Contents**: C++ header with class definition for SidebarWidget.

  - **Relationships**: Included by src/app/include/app/mainwindow.h.

- **File 15**: src/ui/src/projecttreemodel.cpp

  - **Purpose**: Implementation of the data model for the project tree
    view.

  - **Contents**: C++ source code for the ProjectTreeModel
    (QAbstractItemModel).

  - **Relationships**: Depends on src/ui/include/ui/projecttreemodel.h.

- **File 16**: src/ui/include/ui/projecttreemodel.h

  - **Purpose**: Header for the ProjectTreeModel.

  - **Contents**: C++ header with class definition for ProjectTreeModel.

  - **Relationships**: Included by
    src/core/include/core/ProjectStateService.h,
    src/ui/include/ui/sidebarwidget.h.

- **File 17**: src/parsers/src/lasparser.cpp

  - **Purpose**: Implementation of the LAS file parser.

  - **Contents**: C++ source code for reading and parsing LAS format
    files.

  - **Relationships**: Depends on
    src/parsers/include/parsers/lasparser.h.

- **File 18**: src/parsers/include/parsers/lasparser.h

  - **Purpose**: Header for the LAS file parser.

  - **Contents**: C++ header with class definition for LasParser.

  - **Relationships**: Included by src/app/include/app/mainwindow.h.

- **File 19**: src/parsers/src/E57ParserCore.cpp

  - **Purpose**: Core implementation of the E57 file parser
    (Qt-independent).

  - **Contents**: C++ source code for low-level E57 file parsing using
    libE57Format.

  - **Relationships**: Depends on
    src/parsers/include/parsers/E57ParserCore.h.

- **File 20**: src/parsers/include/parsers/E57ParserCore.h

  - **Purpose**: Header for the core E57 parser.

  - **Contents**: C++ header with class definition for E57ParserCore.

  - **Relationships**: Included by
    src/parsers/include/parsers/e57parserlib.h.

- **File 21**: src/parsers/src/e57parserlib.cpp

  - **Purpose**: Qt adapter and wrapper for E57ParserCore.

  - **Contents**: C++ source code for E57ParserLib, providing Qt
    signals/slots and delegating to E57ParserCore.

  - **Relationships**: Depends on
    src/parsers/include/parsers/e57parserlib.h,
    src/parsers/include/parsers/E57ParserCore.h,
    src/interfaces/include/interfaces/IE57Parser.h.

- **File 22**: src/parsers/include/parsers/e57parserlib.h

  - **Purpose**: Header for the Qt E57 parser wrapper.

  - **Contents**: C++ header with class definition for E57ParserLib.

  - **Relationships**: Implements
    src/interfaces/include/interfaces/IE57Parser.h.

- **File 23**: src/app/src/mainwindow.cpp

  - **Purpose**: Main application window implementation.

  - **Contents**: C++ source code for the QMainWindow setup, UI
    connections, and high-level event handling.

  - **Relationships**: Depends on src/app/include/app/mainwindow.h,
    src/app/include/app/MainPresenter.h,
    src/interfaces/include/interfaces/IMainView.h,
    src/interfaces/include/interfaces/IPointCloudViewer.h.

- **File 24**: src/app/include/app/mainwindow.h

  - **Purpose**: Header for the main application window.

  - **Contents**: C++ header with class definition for MainWindow.

  - **Relationships**: Implements
    src/interfaces/include/interfaces/IMainView.h.

- **File 25**: src/app/src/mainwindow_simple.cpp

  - **Purpose**: (To be evaluated for removal) Simplified QMainWindow
    for initial testing.

  - **Contents**: C++ source code for basic MainWindow setup.

  - **Relationships**: Depends on
    src/app/include/app/mainwindow_simple.h.

- **File 26**: src/app/include/app/mainwindow_simple.h

  - **Purpose**: (To be evaluated for removal) Header for the simple
    MainWindow.

  - **Contents**: C++ header with class definition.

  - **Relationships**: N/A.

- **File 27**: src/app/src/MainPresenter.cpp

  - **Purpose**: Implementation of the MainPresenter (application
    logic).

  - **Contents**: C++ source code for handling user actions,
    coordinating between view and model, and managing application state.

  - **Relationships**: Depends on src/app/include/app/MainPresenter.h,
    src/interfaces/include/interfaces/IMainView.h,
    src/interfaces/include/interfaces/IE57Parser.h,
    src/interfaces/include/interfaces/IPointCloudViewer.h,
    src/core/include/core/projectmanager.h,
    src/app/include/app/pointcloudloadmanager.h.

- **File 28**: src/app/include/app/MainPresenter.h

  - **Purpose**: Header for the MainPresenter.

  - **Contents**: C++ header with class definition for MainPresenter.

  - **Relationships**: Used by src/app/include/app/mainwindow.h.

- **File 29**: src/app/src/MainPresenterExtensions.cpp

  - **Purpose**: Additional functionality/slots for MainPresenter.

  - **Contents**: C++ source code extending MainPresenter.

  - **Relationships**: Depends on src/app/include/app/MainPresenter.h.

- **File 30**: src/app/src/MainViewAdapter.cpp

  - **Purpose**: Adapter to bridge MainWindow to IMainView.

  - **Contents**: C++ source code forwarding IMainView calls to
    MainWindow.

  - **Relationships**: Depends on src/app/include/app/MainViewAdapter.h,
    src/app/include/app/mainwindow.h,
    src/interfaces/include/interfaces/IMainView.h.

- **File 31**: src/app/include/app/MainViewAdapter.h

  - **Purpose**: Header for the MainViewAdapter.

  - **Contents**: C++ header with class definition for MainViewAdapter.

  - **Relationships**: Implements
    src/interfaces/include/interfaces/IMainView.h.

- **File 32**: src/app/src/pointcloudloadmanager.cpp

  - **Purpose**: Implementation of the PointCloudLoadManager for
    handling point cloud file loading.

  - **Contents**: C++ source code for loading point cloud data and
    emitting progress/completion signals.

  - **Relationships**: Depends on
    src/app/include/app/pointcloudloadmanager.h,
    src/parsers/include/parsers/e57parserlib.h,
    src/parsers/include/parsers/lasparser.h,
    src/interfaces/include/interfaces/IPointCloudViewer.h.

- **File 33**: src/app/include/app/pointcloudloadmanager.h

  - **Purpose**: Header for the PointCloudLoadManager.

  - **Contents**: C++ header with class definition for
    PointCloudLoadManager.

  - **Relationships**: Used by src/app/include/app/MainPresenter.h,
    src/app/include/app/mainwindow.h.

- **File 34**: src/app/src/scanimportmanager.cpp

  - **Purpose**: Implementation of the ScanImportManager for managing
    scan import operations.

  - **Contents**: C++ source code for importing scan files.

  - **Relationships**: Depends on
    src/app/include/app/scanimportmanager.h.

- **File 35**: src/app/include/app/scanimportmanager.h

  - **Purpose**: Header for the ScanImportManager.

  - **Contents**: C++ header with class definition for
    ScanImportManager.

  - **Relationships**: Included by
    src/core/include/core/ProjectStateService.h,
    src/app/include/app/MainPresenter.h.

- **File 36**: src/interfaces/src/IE57Parser.cpp

  - **Purpose**: Companion implementation for IE57Parser.h (due to
    Q_OBJECT macro).

  - **Contents**: Minimal C++ source file.

  - **Relationships**: Depends on
    src/interfaces/include/interfaces/IE57Parser.h.

- **File 37**: src/interfaces/include/interfaces/IE57Parser.h

  - **Purpose**: Abstract interface for E57 file parsing.

  - **Contents**: C++ header defining the IE57Parser interface.

  - **Relationships**: Implemented by
    src/parsers/include/parsers/e57parserlib.h. Used by
    src/app/include/app/MainPresenter.h.

- **File 38**: src/interfaces/include/interfaces/IMainView.h

  - **Purpose**: Abstract interface for the main application window.

  - **Contents**: C++ header defining the IMainView interface.

  - **Relationships**: Implemented by
    src/app/include/app/MainViewAdapter.h. Used by
    src/app/include/app/MainPresenter.h.

- **File 39**: src/interfaces/include/interfaces/IPointCloudViewer.h

  - **Purpose**: Abstract interface for 3D point cloud rendering.

  - **Contents**: C++ header defining the IPointCloudViewer interface.

  - **Relationships**: Implemented by
    src/rendering/include/rendering/pointcloudviewerwidget.h. Used by
    src/app/include/app/MainPresenter.h.

- **File 40**: src/interfaces/include/interfaces/IE57Writer.h

  - **Purpose**: Abstract interface for E57 file writing.

  - **Contents**: C++ header defining the IE57Writer interface.

  - **Relationships**: Used by src/app/include/app/MainPresenter.h,
    src/export/PointCloudExporter.cpp.

- **File 41**: src/rendering/src/pointcloudviewerwidget.cpp

  - **Purpose**: Implementation of the 3D point cloud viewer widget.

  - **Contents**: C++ source code for OpenGL rendering, camera control,
    and viewer state management.

  - **Relationships**: Depends on
    src/rendering/include/rendering/pointcloudviewerwidget.h,
    src/interfaces/include/interfaces/IPointCloudViewer.h.

- **File 42**: src/rendering/include/rendering/pointcloudviewerwidget.h

  - **Purpose**: Header for the 3D point cloud viewer.

  - **Contents**: C++ header with class definition for
    PointCloudViewerWidget.

  - **Relationships**: Implements
    src/interfaces/include/interfaces/IPointCloudViewer.h. Included by
    src/app/include/app/mainwindow.h.

## Acceptance Criteria

1.  **Build Success**: The entire project, including all libraries and
    the main executable, compiles and links without any errors or
    warnings related to file paths, missing includes, or incorrect
    dependencies.

2.  **Test Suite Pass**: All existing unit tests and integration tests
    (including those in tests/sprint4/, tests/integration/,
    tests/test_e57parser_sprint4_comprehensive.cpp, etc.) pass
    successfully after the restructuring.

3.  **Application Launch & Functionality**:

    - The CloudRegistration application launches without errors.

    - The project hub and project view are displayed correctly.

    - Creating a new project successfully creates the directory
      structure and metadata.

    - Opening an existing project successfully loads the project and its
      scans (if any).

    - Importing E57 and LAS scan files works as before.

    - Point clouds are loaded and displayed correctly in the 3D viewer.

    - Camera controls (orbit, pan, zoom, presets) function as expected.

    - UI elements related to project management (sidebar, buttons) are
      enabled/disabled correctly based on application state.

    - No regression in existing features (e.g., performance, memory
      usage, error handling).

4.  **Modular Structure Adherence**:

    - No C++ source (.cpp) or header (.h) files (excluding main.cpp)
      remain directly in the src/ directory.

    - All relocated files are in their respective
      src/\<module_name\>/src/ and
      src/\<module_name\>/include/\<module_name\>/ directories.

    - CMakeLists.txt files for each module accurately list their
      contained source and header files from their new locations.

    - #include directives across the codebase use the new modular paths
      (e.g., #include \"core/project.h\" instead of #include
      \"../project.h\" or #include \"project.h\" from a different src/
      subdirectory).

5.  **No Duplicate Files**: No unnecessary duplicate files are created
    or left behind.

## Testing Plan

The testing plan focuses on a multi-layered approach to ensure that the
restructuring does not introduce regressions and that the new modular
structure is correctly implemented.

1.  **Build Verification Testing (BVT)**

    - **Test Case 1**: Clean build of the entire project.

      - **Test Data**: All source files after relocation.

      - **Expected Result**: cmake \--build . completes successfully
        with zero compilation or linking errors/warnings.

      - **Testing Tool**: CMake, Compiler (MSVC/GCC/Clang).

    - **Test Case 2**: Verify correct include paths.

      - **Test Data**: Randomly sample a few .cpp files from different
        modules.

      - **Expected Result**: All #include statements in these files
        correctly resolve to the new modular paths.

      - **Testing Tool**: Manual code inspection, IDE\'s include path
        resolution.

2.  **Automated Unit and Integration Testing**

    - **Test Case 2.1**: Run all existing unit tests.

      - **Test Data**: Pre-existing unit test data.

      - **Expected Result**: All tests (e.g., from tests/algorithms/,
        tests/core/, tests/parsers/, tests/ui/, etc.) pass with 100%
        success rate.

      - **Testing Tool**: CTest (ctest \--output-on-failure), Google
        Test.

    - **Test Case 2.2**: Run all integration tests.

      - **Test Data**: Pre-existing integration test data (e.g., sample
        E57/LAS files, mock objects).

      - **Expected Result**: All tests (e.g., EndToEndIntegrationTests,
        MainPresenterTests, E57Sprint4ComprehensiveTest) pass with 100%
        success rate.

      - **Testing Tool**: CTest, Google Test, Google Mock.

3.  **Manual Functional Testing**

    - **Test Case 3.1**: Application launch and basic UI interaction.

      - **Steps**: Launch CloudRegistration.exe. Navigate through menus.
        Click various buttons (e.g., \"New Project\", \"Open Project\").

      - **Expected Result**: Application launches without crashes. All
        UI elements respond correctly.

      - **Testing Tool**: Manual execution.

    - **Test Case 3.2**: Project management workflow.

      - **Steps**: Create a new project. Import an E57 file. Observe
        sidebar populated with scans. Close the project. Open the newly
        created project.

      - **Expected Result**: Project creation is successful. Scans are
        imported and visible. Project state is maintained across
        sessions.

      - **Test Data**: A sample E57 file.

      - **Testing Tool**: Manual execution.

    - **Test Case 3.3**: Point cloud loading and 3D viewer interaction.

      - **Steps**: Open a large E57 or LAS file. Interact with the 3D
        viewer (orbit, pan, zoom). Toggle rendering options (if exposed
        in UI).

      - **Expected Result**: Point cloud loads and renders correctly.
        Camera controls are smooth. No rendering artifacts or crashes.

      - **Test Data**: A large sample E57 file (e.g., bunnyDouble.e57)
        and a LAS file.

      - **Testing Tool**: Manual execution.

    - **Test Case 3.4**: Error handling verification.

      - **Steps**: Attempt to open a non-existent file. Attempt to
        import an invalid file type.

      - **Expected Result**: Appropriate error messages are displayed to
        the user. The application remains stable and does not crash.

      - **Testing Tool**: Manual execution.

## Assumptions and Dependencies

- **Assumptions**:

  - The existing unit and integration tests are comprehensive enough to
    cover the functionality of the moved components.

  - The current build system (CMake and vcpkg) is correctly configured
    and functional.

  - The development environment supports C++17.

  - No implicit dependencies exist between the moved files that are not
    covered by explicit #include statements or CMake target
    dependencies.

  - The content of the files themselves (logic, algorithms) will not be
    changed, only their location and include paths.

  - The mainwindow_simple.cpp/h files are a legacy/simple version of the
    main window and can either be removed or treated as app-specific
    components. For this backlog, they will be moved to src/app/.

- **Dependencies**:

  - **Qt 6.9.0**: Essential for UI and core framework functionality.

  - **Eigen3**: For linear algebra operations in src/algorithms/.

  - **libE57Format**: For E57 file parsing.

  - **Xerces-C**: XML parser dependency for libE57Format.

  - **GTest/GMock**: Testing frameworks.

  - **CMake**: Build system.

  - **vcpkg**: Package manager for non-Qt dependencies.

## Non-Functional Requirements

- **Performance**: The restructuring should not introduce any measurable
  performance degradation in file loading, processing, or 3D rendering.
  Existing performance benchmarks should be maintained or improved.

- **Maintainability**: The primary goal of this restructuring is to
  significantly improve the maintainability and readability of the
  codebase by enforcing a clear modular structure.

- **Readability**: Code should be easier to navigate and understand,
  with clear separation of concerns between modules.

- **Scalability**: The new structure should facilitate future
  development and scaling of the application by making it easier to add
  new features or modify existing ones within well-defined modules.

- **Testability**: The clear interfaces and modularity will further
  enhance the testability of individual components.

- **Portability**: The changes should not negatively impact the
  cross-platform portability of the application.

## Conclusion

This detailed backlog provides a clear roadmap for reorganizing the core
src/ directory into a well-defined modular structure. By systematically
relocating files, updating build configurations, and verifying
functionality through a comprehensive testing plan, we aim to
significantly enhance the project\'s long-term maintainability,
scalability, and developer experience without introducing any
regressions.
