**Project Requirements Document: Parallelized Project Restructure**

Author: Gemini

Version: 1.1

Date: June 11, 2025

**1. Introduction & Goal**

The objective of this project is to refactor the Cloud Registration
application from a monolithic structure into a modular, package-based
architecture. The current system, with all source files compiled into a
single executable, has led to significant development bottlenecks,
including long recompilation times for minor changes, difficulty in
isolating build errors, and an inability for teams to work independently
without causing integration conflicts. A change in a minor UI component,
for example, can trigger a full recompilation of unrelated data-parsing
or algorithmic code.

This refactoring will enable multiple development teams to work in
parallel on different components, significantly speeding up the
development process and improving the overall quality and
maintainability of the codebase. This document outlines a phased sprint
plan where foundational work is completed first, followed by a series of
parallel sprints for each module, and concluding with integration and
finalization sprints.

**2. Project Phases & Parallelism**

- **Phase 0: Scaffolding (1 Sprint, Sequential)**

  - This initial phase is performed by a single team (or the tech lead)
    and is a prerequisite for all subsequent work. It creates the
    complete directory structure and all necessary CMakeLists.txt files,
    establishing a common foundation and a clear contract for all teams.

<!-- -->

- **Phase 1: Parallel Module Development (Multiple Sprints, Parallel)**

  - This is the core of the parallel work. **Sprints 2 through 6 can be
    executed simultaneously by different teams.** Each sprint is
    self-contained and focuses on migrating the code for one specific
    module (e.g., core, algorithms, rendering) into its own static
    library. The defined public header files (include/) for each module
    will serve as the stable interface that other teams can code
    against.

<!-- -->

- **Phase 2: Integration & Finalization (Multiple Sprints, Sequential)**

  - Once all modules from Phase 1 are complete, individually compiled,
    and unit tested, this phase integrates them into the main
    application and the final test suite. These sprints are sequential
    and bring all the parallel work together into a cohesive, functional
    application.

**Phase 0: Scaffolding**

**Sprint 1: Project Skeleton and Build System Foundation**

- **User Story**: As the Tech Lead, I want to create the complete new
  directory structure and all necessary CMakeLists.txt files for the
  modular project, so that multiple teams can immediately start working
  in parallel on their respective modules within a consistent and
  functional build framework.

- **Actions to Undertake**:

  1.  Create a new feature/modular-restructure branch in Git to isolate
      this work.

  2.  Create the full directory structure as defined in the
      \"Recommended Modular C++ Project Structure\" document (e.g.,
      src/core, src/algorithms, etc., including include/\<module_name\>
      and src subdirectories for each).

  3.  Create a new root CMakeLists.txt file. This file will set up the
      project, define global properties, find globally required packages
      (like Qt6 and GTest), and use add_subdirectory() to include all
      module and test directories.

  4.  Create a placeholder CMakeLists.txt file inside *each* module\'s
      directory (e.g., src/core/CMakeLists.txt). Initially, these will
      contain a basic add_library(\<module_name\> STATIC \"\") command
      with no source files and a message(STATUS \"Configuring
      \<module_name\> library\...\") to confirm it\'s being processed.

  5.  Create placeholder CMakeLists.txt files in src/app/ and tests/
      that will later define the main executable and the test runners.

<!-- -->

- **List of Files being Created**:

  - /CMakeLists.txt (Root)

  - src/core/CMakeLists.txt

  - src/algorithms/CMakeLists.txt

  - src/parsers/CMakeLists.txt

  - src/rendering/CMakeLists.txt

  - src/ui/CMakeLists.txt

  - src/app/CMakeLists.txt

  - tests/CMakeLists.txt

<!-- -->

- **Acceptance Criteria**:

  - The project skeleton can be configured using cmake -B build from the
    root directory without any warnings or errors.

  - The build process, initiated by cmake \--build build, will complete
    successfully, producing empty static library files (e.g., core.a or
    core.lib) for each module.

  - The generated directory structure and CMake files perfectly match
    the project\'s architectural plan.

<!-- -->

- **Testing Plan**:

  - **Test 1**: From a clean state, run cmake -B build. The output
    should show the status messages from each module\'s CMakeLists.txt,
    confirming they are all included.

  - **Test 2**: Run cmake \--build build. The build output should
    confirm that each library target (e.g., core, algorithms) is built,
    even though they contain no source files yet.

**Phase 1: Parallel Module Development**

**The following sprints (2-6) can be executed simultaneously by
different teams.**

**Sprint 2: Implement the core Library**

- **User Story**: As a developer on the core team, I want to migrate all
  foundational code (project management, data structures, and managers)
  into a self-contained core static library, so that all other modules
  can link against it and use its functionality.

- **Actions to Undertake**:

  1.  Move relevant public headers (project.h, projectmanager.h,
      pointdata.h, sqlitemanager.h) to the src/core/include/core/
      directory.

  2.  Move the corresponding implementation files (.cpp) to the
      src/core/src/ directory.

  3.  Update src/core/CMakeLists.txt to include all source files for the
      core library.

  4.  Use target_include_directories to expose the public headers to
      other modules.

  5.  Link the core library against its direct dependencies, such as
      Qt6::Core and Qt6::Sql.

<!-- -->

- **Acceptance Criteria**:

  - The core library compiles successfully as a standalone static
    library by running cmake \--build build \--target core.

  - All internal #include paths within the module are updated to reflect
    the new structure.

**Sprint 3: Implement the algorithms Library**

- **User Story**: As a developer on the algorithms team, I want to
  migrate all registration algorithms (ICP, Least Squares) into a
  self-contained algorithms static library to decouple the mathematical
  logic from the rest of the application.

- **Actions to Undertake**:

  1.  Move algorithm headers (ICPRegistration.h,
      LeastSquaresAlignment.h) to src/algorithms/include/algorithms/.

  2.  Move the corresponding .cpp files to src/algorithms/src/.

  3.  Update src/algorithms/CMakeLists.txt to define the algorithms
      library and list its source files.

  4.  Link against necessary dependencies. This will include the core
      library (for data structures like Point3D) and Eigen3::Eigen.

<!-- -->

- **Acceptance Criteria**:

  - The algorithms library compiles successfully as a standalone static
    library (cmake \--build build \--target algorithms). This command
    will automatically build the core library first due to the
    dependency.

**Sprint 4: Implement the parsers Library**

- **User Story**: As a developer on the data team, I want to migrate all
  file parsers (E57, LAS) into a self-contained parsers static library
  to isolate all file I/O and format-specific logic.

- **Actions to Undertake**:

  1.  Move parser interface and implementation headers (e57parserlib.h,
      lasparser.h, E57ParserCore.h) to src/parsers/include/parsers/.

  2.  Move the corresponding .cpp files to src/parsers/src/.

  3.  Update src/parsers/CMakeLists.txt to define the parsers library.

  4.  Link against external dependencies such as E57Format and
      XercesC::XercesC, and internal ones like Qt6::Core.

<!-- -->

- **Acceptance Criteria**:

  - The parsers library compiles successfully as a standalone static
    library (cmake \--build build \--target parsers).

**Sprint 5: Implement the rendering Library**

- **User Story**: As a developer on the graphics team, I want to migrate
  all OpenGL, camera control, and viewer logic into a self-contained
  rendering static library to abstract all graphics operations.

- **Actions to Undertake**:

  1.  Move rendering-related headers (OpenGLRenderer.h,
      CameraController.h, GpuCuller.h) to
      src/rendering/include/rendering/.

  2.  Move the corresponding .cpp files to src/rendering/src/.

  3.  Update src/rendering/CMakeLists.txt to define the rendering
      library.

  4.  Link against Qt\'s OpenGL modules and any other graphics-related
      dependencies.

<!-- -->

- **Acceptance Criteria**:

  - The rendering library compiles successfully as a standalone static
    library (cmake \--build build \--target rendering).

**Sprint 6: Implement the ui Library**

- **User Story**: As a developer on the UI team, I want to migrate all
  reusable UI widgets, such as custom dialogs and control panels, into a
  self-contained ui static library.

- **Actions to Undertake**:

  1.  Move reusable widget headers (AlignmentControlPanel.h,
      CreateProjectDialog.h, ExportDialog.h) to src/ui/include/ui/.

  2.  Move the corresponding .cpp files and .ui files to src/ui/src/.

  3.  Update src/ui/CMakeLists.txt to define the ui library.

  4.  Link against Qt6::Widgets and any other module libraries it
      depends on (e.g., core).

<!-- -->

- **Acceptance Criteria**:

  - The ui library compiles successfully as a standalone static library
    (cmake \--build build \--target ui).

**Phase 2: Integration & Finalization**

**Sprint 7: Application Integration**

- **User Story**: As the integration lead, I want to assemble the main
  application executable by linking all the newly created static
  libraries together, ensuring all components function correctly as a
  whole.

- **Actions to Undertake**:

  1.  Move application-specific files like main.cpp and
      mainwindow.h/.cpp to the src/app/ directory.

  2.  Update src/app/CMakeLists.txt to define the CloudRegistration
      executable using these files.

  3.  Update the #include statements in mainwindow.cpp to use the new
      modular paths (e.g., #include \"core/project.h\").

  4.  Use target_link_libraries to link the executable against all the
      modular libraries: core, algorithms, parsers, rendering, and ui.

<!-- -->

- **Acceptance Criteria**:

  - The full CloudRegistration application compiles and links without
    any errors.

  - The application launches and core functionalities, such as opening a
    file and viewing the point cloud, are operational.

**Sprint 8: Test Suite Integration**

- **User Story**: As the QA lead, I want to refactor the entire test
  suite so that each test case compiles and runs against the appropriate
  new modular library, ensuring our quality standards are maintained.

- **Actions to Undertake**:

  1.  Move existing test files into a structured tests/ subdirectory
      (e.g., tests/core/, tests/algorithms/).

  2.  Update the tests/CMakeLists.txt file to define individual test
      executables for each module.

  3.  For each test executable, link it *only* against the specific
      library it is testing and GTest. For example, test_projectmanager
      will link against core and GTest::gtest_main.

  4.  Ensure all tests pass by running ctest from the build directory.

<!-- -->

- **Acceptance Criteria**:

  - All test executables for all modules compile and link successfully.

  - Running ctest \--output-on-failure shows all tests passing.
