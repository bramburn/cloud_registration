Product Requirements Document: Core Component Decoupling

Author: Gemini
Date: June 7, 2024
Version: 1.0
1. Introduction & Overview

This document outlines the requirements for a significant technical refactoring initiative. The primary goal is to decouple a set of large, monolithic C++ files within the Cloud Registration application. The current state of these files presents challenges in maintainability, testability, and scalability. By breaking them down into smaller, more focused, and loosely-coupled components, we aim to improve the overall health of the codebase, reduce complexity, and increase development velocity for future features.

Target Files for Decoupling:

    src/pointcloudviewerwidget.cpp

    src/mainwindow.cpp

    src/e57parserlib.cpp

    src/projectmanager.cpp

    src/sidebarwidget.cpp

2. Problem Statement

The identified C++ source files have grown significantly over time, leading to several issues:

    High Coupling & Low Cohesion: Classes have multiple, unrelated responsibilities (e.g., UI logic mixed with business logic, data parsing mixed with state management). This makes the code difficult to understand, modify, and debug.

    Poor Testability: The monolithic nature of these files makes it nearly impossible to write effective unit tests, leading to a reliance on manual, end-to-end testing which is slow and less reliable.

    Increased Risk of Regression: A small change in one part of a large file can have unintended consequences in another, increasing the risk of introducing bugs.

    Onboarding Difficulty: New developers face a steep learning curve in understanding the complex interactions within these large files.

    Reduced Development Speed: Modifying or extending functionality is slow and cumbersome due to the high cognitive load and complexity of the existing code.

3. Goals & Objectives

The primary objective is to refactor the target files to adhere to modern software design principles, specifically the Single Responsibility Principle (SRP) and Dependency Inversion.

Key Goals:

    Reduce Complexity: Break down large classes into smaller, more manageable ones with clear responsibilities.

    Improve Maintainability: Create a codebase that is easier to read, understand, and modify.

    Enhance Testability: Structure the code to allow for comprehensive unit and integration testing.

    Increase Code Reusability: Develop modular components that can be reused across the application.

    Lower Lines of Code (LOC): Achieve a measurable reduction in LOC for the specified files by eliminating redundancy and improving abstraction.

4. Scope
In Scope

    Refactoring the five specified C++ files.

    Creating new classes, interfaces, and modules to house the decoupled logic.

    Implementing design patterns (e.g., Model-View-Presenter, Strategy, Facade) to separate concerns.

    Creating a suite of unit tests for the new, decoupled components.

    Ensuring the application's external behavior remains unchanged after refactoring.

Out of Scope

    Adding any new user-facing features.

    Changing the existing UI/UX design.

    Refactoring files not on the specified list.

    Changing the application's core technology stack (Qt, C++).

5. Technical Requirements

    The refactored code must be written in C++17 or later.

    The solution must integrate seamlessly with the existing Qt 6 framework.

    All new, decoupled business logic must be covered by unit tests (e.g., using GTest/GMock).

    Interfaces (abstract base classes) must be used to decouple components, particularly between UI and business logic.

    The refactoring should not result in any performance degradation. Performance benchmarks should be established before and after the refactoring.

6. Phased Rollout Plan

This project will be executed in three phases, broken down into two-week sprints.
Phase 1: Foundational Decoupling (4 Weeks)

    Sprint 1: Interface Abstraction

        Goal: Create abstractions for the main UI components to break the direct dependency between UI and business logic.

        Tasks:

            Define and implement IPointCloudViewer and IMainView interfaces based on the public-facing logic in pointcloudviewerwidget.cpp and mainwindow.cpp.

            Modify MainWindow to interact with the viewer through the IPointCloudViewer interface.

            Introduce a MainPresenter to begin mediating between the view and the backend services.

    Sprint 2: Parser Refactoring

        Goal: Decouple the E57 file parsing logic.

        Tasks:

            Analyze e57parserlib.cpp and identify the core parsing logic versus the Qt-specific wrapper code.

            Extract the core parsing logic into a separate, non-Qt-dependent module.

            Ensure the E57ParserLib class acts as a thin wrapper that adapts the core parser to the existing IE57Parser interface.

            Write unit tests for the new core parsing module.

Phase 2: Component Separation (4 Weeks)

    Sprint 3: Project Management Decoupling

        Goal: Break down the ProjectManager into smaller, focused services.

        Tasks:

            Separate the recent projects logic from ProjectManager into the existing RecentProjectsManager class, ensuring it's fully independent.

            Create a new ProjectStateService to manage the currently active project state, removing that responsibility from ProjectManager.

            Refactor ProjectManager to be a facade that coordinates these smaller services.

    Sprint 4: UI Logic Separation

        Goal: Decouple the business logic from the SidebarWidget.

        Tasks:

            Analyze sidebarwidget.cpp to identify logic not directly related to UI rendering (e.g., handling context menu actions, drag-and-drop logic).

            Move this logic into the MainPresenter or a new dedicated controller.

            The SidebarWidget should only be responsible for displaying data and emitting signals in response to user actions.

Phase 3: Integration & Validation (4 Weeks)

    Sprint 5: Full Integration

        Goal: Ensure all decoupled components are fully integrated and the application is stable.

        Tasks:

            Perform end-to-end testing of all refactored user flows (e.g., opening a project, importing a scan, interacting with the viewer).

            Resolve any integration issues that arise.

            Conduct performance testing to ensure no regressions have been introduced.

    Sprint 6: Test Suite Finalization & Documentation

        Goal: Finalize the testing suite and document the new architecture.

        Tasks:

            Complete unit and integration test coverage for all new modules.

            Update developer documentation to reflect the new, decoupled architecture.

            Measure and document the final success metrics.

7. Success Metrics

    LOC Reduction: A minimum of a 25% reduction in the total lines of code across the five target files.

    Unit Test Coverage: Achieve at least 70% unit test coverage for all new, decoupled modules.

    Code Complexity: A measurable reduction in the cyclomatic complexity of the original classes.

    Build Time: A noticeable improvement in incremental build times after modifying a decoupled component compared to modifying the original monolithic file.

    Qualitative Feedback: Positive feedback from the development team regarding the ease of understanding and working with the new code structure.

8. Risks & Mitigation

    Risk: Introducing regressions or breaking existing functionality.

        Mitigation: A comprehensive suite of manual and automated tests will be run at the end of each phase. The phased approach limits the scope of potential issues at each step.

    Risk: The refactoring effort takes longer than anticipated.

        Mitigation: The work is broken into discrete, two-week sprints. If a sprint's goals are not met, the scope will be reassessed. The highest-impact files are tackled first to ensure maximum value early on.

    Risk: Performance degradation due to increased abstractions.

        Mitigation: Performance benchmarks will be established before starting and measured after each phase. Hot paths will be profiled to ensure abstractions do not introduce significant overhead.