# **Product Requirements Document: CMake Rebuild & Optimization**

Author: Gemini  
Version: 1.1  
Date: June 10, 2025

## **1\. Introduction**

### **1.1. Problem Statement**

The current CMakeLists.txt for the Cloud Registration application is a large, monolithic file that has become difficult to manage and debug. It has led to several persistent build errors, including issues with dependency discovery (Eigen3, Vulkan), source file path resolution, and incorrect package manager usage (vcpkg). A complete, systematic rebuild is required to create a stable, modern, and maintainable build system.

### **1.2. Project Goal**

This project aims to incrementally rebuild the CMakeLists.txt file from the ground up. By adding source files and dependencies in logical, isolated phases, we will create a clean, optimized, and error-free build configuration. The final deliverable will be a new, fully functional CMakeLists.txt that is easy for developers to understand and extend.

## **2\. Guiding Principles**

* **Incremental & Iterative:** Start with a minimal, working build and add components one by one. Compile and test at each step to catch errors immediately.  
* **Modern CMake:** Utilize modern CMake targets and properties (target\_link\_libraries, target\_include\_directories, target\_compile\_features) instead of old-style global commands.  
* **Dependency Management First:** Correctly integrate vcpkg using its toolchain file to ensure all dependencies are found reliably. All dependencies will be listed in vcpkg.json.  
* **Modularization:** Group source files into logical components or libraries to improve build-time parallelism and code organization.

## **3\. The Rebuild Plan: Phases and Sprints**

This plan breaks the rebuild process into distinct, verifiable phases and sprints.

### **Phase 0: The Foundation**

**Goal:** Establish a minimal, compilable project structure that serves as a stable base for future development.

#### **Sprint 1: Barebones Project Setup**

| Requirement ID | Description |
| :---- | :---- |
| F0-S1-01 | Create a new, empty CMakeLists.txt file. |
| F0-S1-02 | Define the project with cmake\_minimum\_required(VERSION 3.16) and project(CloudRegistration). |
| F0-S1-03 | Set the C++ standard to 17 and enable standard-required flags: set(CMAKE\_CXX\_STANDARD 17), set(CMAKE\_CXX\_STANDARD\_REQUIRED ON). |
| F0-S1-04 | Add platform-specific compiler flags for MSVC, GCC, and Clang to enforce warnings, UTF-8, and optimizations. |
| F0-S1-05 | Create a minimal src/main.cpp that only contains a main function. |
| F0-S1-06 | Create a basic executable target: add\_executable(CloudRegistration src/main.cpp). |
| **Acceptance** | The project configures (cmake ..) and builds (cmake \--build .) successfully, producing a minimal executable without any link errors. |

### **Phase 1: Core Application Build-out**

**Goal:** Integrate all UI components, core logic, and third-party dependencies to achieve a fully-featured, runnable application.

#### **Sprint 2: Qt Integration & Main Window**

| Requirement ID | Description |
| :---- | :---- |
| F1-S2-01 | **Crucial:** Integrate vcpkg. At the top of CMakeLists.txt, set CMAKE\_TOOLCHAIN\_FILE to the path of vcpkg.cmake. |
| F1-S2-02 | Create vcpkg.json in the project root and add qtbase and qtopengl as initial dependencies. |
| F1-S2-03 | Use find\_package(Qt6 ...) to locate and link the necessary Qt components (Core, Gui, Widgets, OpenGLWidgets). |
| F1-S2-04 | Add src/mainwindow.cpp and src/mainwindow.h to the executable target. Ensure CMAKE\_AUTOMOC is ON. |
| **Acceptance** | The application builds and launches an empty QMainWindow without errors, confirming that Qt and vcpkg are correctly integrated. |

#### **Sprint 3: UI and Project Logic Integration**

| Requirement ID | Description |
| :---- | :---- |
| F1-S3-01 | **Crucial:** Gradually add UI-related source files (projecthubwidget.cpp, sidebarwidget.cpp, etc.) to the executable target. **Compile after adding each small group.** |
| F1-S3-02 | Add core project logic files (projectmanager.cpp, recentprojectsmanager.cpp, ProjectStateService.cpp, etc.). |
| F1-S3-03 | **Crucial:** Use ${CMAKE\_CURRENT\_SOURCE\_DIR} to prefix all source file paths (e.g., ${CMAKE\_CURRENT\_SOURCE\_DIR}/src/ui/ExportDialog.cpp) to resolve pathing errors. |
| **Acceptance** | The application builds successfully with all UI and project management source files included. The full UI should now be visible upon launch. |

#### **Sprint 4: Data Parsers and Algorithms**

| Requirement ID | Description |
| :---- | :---- |
| F1-S4-01 | Add eigen3, libe57format, and xerces-c to vcpkg.json. |
| F1-S4-02 | Use find\_package() for Eigen3, E57Format, and XercesC. |
| F1-S4-03 | Add all algorithm files (LeastSquaresAlignment.cpp, ICPRegistration.cpp, etc.) to the executable. |
| F1-S4-04 | Add all parser files (e57parserlib.cpp, lasparser.cpp, etc.) to the executable. |
| F1-S4-05 | Link the required libraries (Eigen3::Eigen, E57Format, XercesC::XercesC) to the CloudRegistration target using target\_link\_libraries. |
| **Acceptance** | The application is now fully functional, compiling with all source code and linking against all required third-party dependencies. |

### **Phase 2: Test Suite Integration**

**Goal:** Re-integrate the entire test suite, ensuring all tests compile, link, and are runnable via CTest.

#### **Sprint 5: Compiling and Running All Tests**

| Requirement ID | Description |
| :---- | :---- |
| F2-S5-01 | Enable testing via enable\_testing(). |
| F2-S5-02 | Add gtest to vcpkg.json and find GTest in CMake. |
| F2-S5-03 | For each test file in the tests/ directory, create a new executable target (e.g., add\_executable(AlignmentEngineTests tests/sprint4/test\_alignment\_engine.cpp)). |
| F2-S5-04 | Link each test target against its required dependencies, including the main application sources or libraries, Qt6::Test, and GTest::gtest\_main. |
| F2-S5-05 | Use add\_test(NAME \<TestName\> COMMAND \<TestExecutable\>) for each test executable. |
| **Acceptance** | Running ctest \--output-on-failure from the build directory successfully executes all unit and integration tests without link or runtime errors. |

### **Phase 3: Refactoring for Maintainability**

**Goal:** Refactor the CMakeLists.txt to be modular, clean, and easy to maintain for future development.

#### **Sprint 6: Codebase Modularization**

| Requirement ID | Description |
| :---- | :---- |
| F3-S6-01 | Create a static library for core algorithms: add\_library(Algorithms STATIC ...) |
| F3-S6-02 | Create a static library for data parsers: add\_library(Parsers STATIC ...) |
| F3-S6-03 | Create a static library for core registration logic: add\_library(RegistrationCore STATIC ...) |
| F3-S6-04 | Refactor the main executable and test executables to link against these new libraries (target\_link\_libraries) instead of a flat list of source files. |
| F3-S6-05 | Use target\_include\_directories with PUBLIC, PRIVATE, and INTERFACE keywords on the new libraries to correctly manage header visibility and compiler include paths. |
| **Acceptance** | The project builds and runs identically to before, but the CMakeLists.txt is now organized around modular library targets. |

#### **Sprint 7: Final Polish & CI Preparation**

| Requirement ID | Description |
| :---- | :---- |
| F3-S7-01 | Review and clean up the entire CMakeLists.txt, adding comments and ensuring consistency. |
| F3-S7-02 | Create a custom target run\_tests that depends on all test executables and runs ctest. This provides a single command to run all tests. |
| F3-S7-03 | (Optional) Add a coverage target for code coverage analysis using lcov and genhtml, if on a compatible platform (Linux/macOS). |
| F3-S7-04 | Add installation rules using install(TARGETS ...) to define how the application and its dependencies should be packaged for deployment. |
| F3-S7-05 | Document the new build process, including how to configure dependencies with vcpkg and how to build the project and run tests. |
| **Acceptance** | The final CMakeLists.txt is clean, modular, well-documented, and ready for integration into a Continuous Integration (CI) pipeline. The build process is robust and easy for new developers to follow. |

## **4\. Key Resolutions for Existing Errors**

This rebuild plan directly addresses the errors documented in the provided files:

* **Eigen3 Not Found:** Resolved in Sprint 4 by using vcpkg and find\_package(Eigen3). The toolchain file handles setting the necessary paths automatically.  
* **vcpkg Manifest Mode Error:** Resolved in Sprint 2 by adding all dependencies (qt6, eigen3, gtest, libe57format, etc.) to the vcpkg.json file and letting CMake/vcpkg handle the installation, rather than calling vcpkg install manually.  
* **Cannot Find Source File:** Resolved in Sprint 3 by using ${CMAKE\_CURRENT\_SOURCE\_DIR} to construct paths relative to the CMakeLists.txt file, ensuring CMake can always locate the source files.