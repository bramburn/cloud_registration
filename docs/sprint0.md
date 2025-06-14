# Product Backlog: Sprint 0 - Project Foundation & Setup

### **Introduction**

This document outlines the detailed product backlog for Sprint 0. The
primary goal of this foundational sprint is to establish a clean,
robust, and reproducible development environment on the restructure
branch. This initial phase is crucial as it addresses the architectural
shortcomings of the previous codebase and sets a new standard for
quality. This sprint does not deliver end-user features but is the most
critical step for ensuring long-term project health, maintainability,
and the successful adoption of a strict Test-Driven Development (TDD)
methodology from the very first line of new code.

### **User Stories**

- **User Story 1**: Establish Core Project Structure

  - **Description**: As a developer, I want to set up the complete
    project structure, including a modern build system, automated
    dependency management, and a logical source code layout. This is
    essential so that we have a consistent, scalable, and modular
    foundation for the application rebuild, which will improve
    development velocity and reduce technical debt.

  - **Epic**: Foundation & Application Bootstrap

### **Actions to Undertake**

1.  **Branch and Code Setup**:

    - Create a new, clean branch named restructure from the current main
      branch to isolate the rebuild effort.

    - Copy the existing codebase from C:\\dev\\old_cloud_repo into a
      /reference_code subfolder within the new branch. This code is to
      be used strictly for reference purposes and will not be compiled
      as part of the new solution. It will be gradually and entirely
      replaced.

2.  **CMake Configuration**:

    - Create a root CMakeLists.txt file. This file will define the
      project name (PointCloudApplication), set the required C++
      standard to C++17, and orchestrate the build by including the /src
      and /tests subdirectories.

    - Create a CMakePresets.json file in the root directory. Populate it
      using the provided JSON configuration to define a repeatable build
      environment for Visual Studio 2022. This preset ensures that all
      developers use the same toolchain file, architecture, and paths
      for critical dependencies like Qt and vcpkg.

3.  **Dependency Management (vcpkg)**:

    - Create a vcpkg.json manifest file in the project root. This file
      formally declares all external project dependencies.

    - Declare the core dependencies: qtbase, qtsvg for UI elements;
      qttools for development utilities like Qt Designer; and gtest for
      the unit testing framework. This setup enables vcpkg to
      automatically handle the acquisition, compilation, and linkage of
      these libraries.

4.  **Directory and Module Scaffolding**:

    - Create the initial directory structure as defined in the PRD. This
      clean separation of concerns is fundamental to a maintainable
      architecture:

      - /src: The parent directory for all application source code.

      - /src/app: Contains the application entry point and main window
        logic, orchestrating the overall application lifecycle.

      - /src/core: Reserved for platform-agnostic business logic, data
        models, and algorithms.

      - /src/ui: Intended for all custom Qt widgets and UI-related logic
        that will be reused across the application.

      - /tests: A parallel source tree for all unit and integration
        tests, mirroring the /src structure for clarity.

    - Create placeholder CMakeLists.txt files within each subdirectory
      (/src, /src/app, /tests) to handle the compilation of their
      respective modules and link them as needed.

5.  **Basic Application Stub**:

    - In /src/app, create a main.cpp file to serve as the application\'s
      executable entry point.

    - This main.cpp will initialize the QApplication object and
      instantiate the MainWindow.

    - Create MainWindow.h and MainWindow.cpp. This class will represent
      a simple, empty QMainWindow with a title set to \"Point Cloud
      Application\". Its successful launch is a key validation point for
      the entire toolchain and library integration.

6.  **Testing Framework Setup**:

    - In /tests, create a main_test.cpp file. Its sole purpose is to
      initialize and run the Google Test framework, acting as the entry
      point for the test suite executable.

    - Create a sample_test.cpp with a single, trivial passing test
      (e.g., EXPECT_EQ(1, 1);). This verifies that the testing framework
      is correctly configured, linked against the test executable, and
      can be run successfully.

### **List of Files being Created**

- **File 1**: CMakeLists.txt (Root)

  - **Purpose**: The main entry point for the CMake build system,
    defining the overall project and its primary components.

  - **Contents**: cmake_minimum_required, project definition,
    find_package calls for Qt6 and GTest, and add_subdirectory commands
    for the src and tests directories.

- **File 2**: CMakePresets.json

  - **Purpose**: To define a consistent and sharable build environment,
    simplifying project setup for all developers.

  - **Contents**: The JSON object provided, specifying the generator
    (Visual Studio 17 2022), architecture, and crucial cache variables
    for the vcpkg-qt preset, including the toolchain file and Qt path.

- **File 3**: vcpkg.json

  - **Purpose**: A declarative manifest for vcpkg to manage all
    third-party project dependencies automatically.

  - **Contents**: A JSON object defining the project name and a
    dependencies array explicitly listing qtbase, qtsvg, qttools, and
    gtest.

- **File 4**: src/app/main.cpp

  - **Purpose**: The executable entry point that initializes the Qt
    application and launches the main window.

  - **Contents**: A standard main function that creates a QApplication,
    instantiates MainWindow, shows it, and starts the Qt event loop via
    app.exec().

- **File 5 & 6**: src/app/MainWindow.h & src/app/MainWindow.cpp

  - **Purpose**: A basic, empty QMainWindow stub used to verify that Qt
    linkage, resource compilation, and application startup are all
    functioning correctly.

  - **Contents**: A standard class definition inheriting from
    QMainWindow, with a constructor that sets the window title and
    initial size.

- **File 7**: tests/main_test.cpp

  - **Purpose**: The executable entry point for the test suite,
    responsible for running all defined tests.

  - **Contents**: A main function that calls
    ::testing::InitGoogleTest(&argc, argv); and returns
    RUN_ALL_TESTS();.

- **File 8**: tests/sample_test.cpp

  - **Purpose**: A minimal sample test file to confirm that the testing
    framework is correctly integrated and operational.

  - **Contents**: A simple test case using the TEST() macro and a basic
    assertion like EXPECT_TRUE(true); to ensure the test runner executes
    and reports a pass.

### **Acceptance Criteria**

1.  A developer can clone the repository, check out the restructure
    branch, and successfully configure and generate the build files in
    Visual Studio 2022 by selecting the vcpkg-qt preset without manual
    intervention.

2.  The project builds successfully from a clean state in both **Debug**
    and **Release** configurations without any compilation or linking
    errors.

3.  Running the main application executable launches a blank, empty
    window with the title \"Point Cloud Application\". The window is
    interactive and can be resized and closed gracefully.

4.  Running the test suite executable from the build directory executes
    successfully and reports that all tests (i.e., the single sample
    test) have passed, confirming the integrity of the test framework.

5.  All dependencies listed in vcpkg.json are correctly installed and
    linked by the vcpkg toolchain during the CMake generation step, with
    no need for manual library path configuration.

### **Testing Plan**

- **Test Case 1**: Build Verification

  - **Test Data**: The source code committed to the restructure branch.

  - **Steps**:

    1.  Perform a fresh clone of the repository and checkout the
        restructure branch.

    2.  Open the project folder in Visual Studio 2022.

    3.  Select the vcpkg-qt preset and wait for CMake generation to
        complete.

    4.  Build the entire solution.

  - **Expected Result**: The build completes with \"Build: 1 succeeded,
    0 failed\". Both the main application executable and the test suite
    executable are generated in the build directory. A failure here
    indicates a fundamental issue in the CMakeLists.txt or preset
    configurations.

  - **Testing Tool**: Visual Studio 2022 Build System.

- **Test Case 2**: Application Launch Verification

  - **Test Data**: The compiled application executable from the previous
    test.

  - **Steps**:

    1.  Navigate to the build directory (e.g., build/Debug).

    2.  Execute the main application .exe file.

  - **Expected Result**: A blank GUI window appears on the screen with
    the correct title. The window responds to user interaction and can
    be closed without crashing. A failure indicates a problem with Qt
    library linkage or runtime dependencies.

  - **Testing Tool**: Manual Execution (Windows Explorer).

- **Test Case 3**: Test Framework Verification

  - **Test Data**: The compiled test executable from the build
    verification test.

  - **Steps**:

    1.  Open a terminal in the build directory.

    2.  Execute the test .exe file from the command line.

  - **Expected Result**: The console output clearly shows \[==========\]
    1 test from 1 test suite ran. and \[ PASSED \] 1 test.. A failure
    indicates a problem with Google Test linkage or the test runner\'s
    entry point.

  - **Testing Tool**: Manual Execution (Windows Command Prompt).

### **Assumptions and Dependencies**

- **Assumptions**:

  - The developer\'s machine has the required toolset installed: Git,
    CMake (v3.20+), Visual Studio 2022 (with \"Desktop development with
    C++\" workload), and a system-wide clone of vcpkg.

  - The hardcoded paths in CMakePresets.json (C:/Qt/6.9.0/msvc2022_64,
    C:/vcpkg) are accurate for the initial development environment.
    These will be parameterized or documented for other developers in
    the future.

- **Dependencies**:

  - **Qt 6.9.0**: Chosen for its feature set and as a stable target for
    development.

  - **vcpkg**: The designated package manager for ensuring consistent
    and reproducible dependency resolution.

  - **Google Test**: The chosen C++ testing framework, foundational to
    our TDD workflow.

### **Non-Functional Requirements**

- **Maintainability**: The project structure must be strictly modular to
  facilitate parallel development and ease of maintenance. The purpose
  of each module should be self-evident from the directory structure.

- **Developer Experience**: The onboarding process for a new developer
  should be as simple as possible. After installing the prerequisites,
  they should be able to configure and build the project with a single
  click in the IDE.

- **Portability**: While initially targeting Windows and MSVC, the CMake
  scripts should avoid platform-specific commands where possible. This
  will ease any potential future efforts to port the application to
  other operating systems like Linux or macOS.

### **Conclusion**

Upon the successful completion of Sprint 0, the restructure branch will
contain a stable, well-structured, and fully testable foundation. This
foundational work is paramount; it provides the clean slate and robust
tooling necessary to confidently and efficiently implement the
user-facing features outlined in the PRD\'s subsequent sprints, ensuring
a higher quality and more maintainable final product.
