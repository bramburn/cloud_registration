Sprint 2.2 Backlog: UI Refinement, Performance Profiling, and Stability
1. Introduction

With the core logic for state and memory management established in Sprint 2.1, this sprint focuses on enhancing the application's usability, performance, and robustness. The primary goals are to provide users with a more intuitive and responsive interface, equip developers with powerful performance analysis tools, and rigorously test the application's stability under memory-intensive scenarios.

This sprint will bridge the gap between backend logic and frontend presentation by making the application's state visually explicit in the UI. It will also introduce a formal performance profiling framework, a critical step for optimizing the data loading pipeline and ensuring the application meets its non-functional performance requirements. Finally, we will stress-test the memory management system to validate its effectiveness and ensure the application remains stable when handling large-scale projects.
2. User Stories
User Story 1: Enhance UI with Context-Aware Controls and Real-Time Feedback

    User Story [1]: As a user, I want the application's interface to provide clear, dynamic feedback about the state of my project and the actions I can perform. The context menu options in the sidebar should be enabled or disabled based on the selected item's state (e.g., "Unload" is only enabled for a loaded scan), and I want to see real-time performance metrics in the status bar.

    Description: This story is about making the application smarter and more informative. By dynamically updating the UI, we prevent user error and provide valuable insight into the application's performance. The SidebarWidget's context menu will become context-aware, only showing relevant actions. The MainWindow's status bar will be enhanced to display key performance indicators (KPIs) like Frames Per Second (FPS) and memory usage, giving the user a constant, unobtrusive overview of the application's health.

    Actions to Undertake:

        Context-Aware Menu: In SidebarWidget::contextMenuEvent, retrieve the LoadedState for the selected item from the ProjectTreeModel. Enable or disable the "Load" and "Unload" QActions based on this state.

        UI Feedback Integration: In PointCloudViewerWidget, implement a QTimer to periodically calculate and emit an statsUpdated(float fps, int visiblePoints) signal.

        Memory Feedback Integration: In PointCloudLoadManager, emit a memoryUsageChanged(size_t totalBytes) signal whenever a scan is loaded or unloaded.

        Status Bar Display: In MainWindow, create slots to receive the statsUpdated and memoryUsageChanged signals. Update QLabel widgets in the status bar to display the FPS, visible point count, and total memory usage.

        Refine Icons: Ensure the IconManager and ProjectTreeModel provide distinct, easily understandable icons for all LoadedStates, including "loading" and "error" states.

    References between Files:

        sidebarwidget.cpp will read state data from projecttreemodel.h to configure its context menu.

        pointcloudviewerwidget.cpp will emit performance statistics.

        pointcloudloadmanager.cpp will emit memory usage updates.

        mainwindow.cpp will connect to signals from both PointCloudViewerWidget and PointCloudLoadManager to update its status bar.

    Acceptance Criteria:

        The "Unload Scan" context menu action is disabled for scans that are not currently loaded.

        The "Load Scan" action is disabled for scans that are already loaded or are in a loading state.

        The main window's status bar continuously displays the current FPS of the 3D viewer.

        The status bar displays the total memory consumed by all loaded point clouds, updating in real-time as scans are loaded and unloaded.

    Testing Plan:

        Manual Test: Right-click on various scans in different states (loaded, unloaded) to confirm that the context menu options are enabled/disabled correctly. Observe the status bar while navigating the 3D view and loading/unloading scans to ensure the FPS and memory usage displays are updating correctly and appear accurate.

        Integration Test: In tests/demos/test_sprint2_2_profiling.cpp, add a test that simulates the full UI flow. Create a mock SidebarWidget, trigger a "load" action, and verify that the PointCloudLoadManager receives the request and that a mock MainWindow receives the subsequent memoryUsageChanged signal.

User Story 2: Implement Performance Profiling and Benchmarking System

    User Story [2]: As a developer, I need a robust performance profiling system to identify bottlenecks in the E57 and LAS loading pipelines. This system should measure execution time for key operations and monitor memory usage, generating detailed reports for analysis.

    Description: To optimize performance, we first need to measure it. This story involves creating a dedicated PerformanceProfiler class that can be used to instrument the code. We will create simple macros (PROFILE_FUNCTION, PROFILE_SECTION) to make it easy to add timing probes to the E57 and LAS parsing logic. A PerformanceBenchmark class will use the profiler to run automated tests against a suite of files and generate reports that compare the performance of different loading strategies or code versions.

    Actions to Undertake:

        Profiler Class: Create a PerformanceProfiler singleton class in src/performance_profiler.h and .cpp. It will use QElapsedTimer to measure execution times. It should store results in a QMap<QString, ProfileSection>.

        Instrumentation Macros: Define macros like PROFILE_SECTION(name) that create a PerformanceProfiler::SectionTimer RAII object, automatically starting and stopping the timer for a given scope.

        Code Instrumentation: Add the profiling macros to key functions within E57ParserLib and LasParser, such as parse, readHeader, and the point data reading loops.

        Benchmark Class: Create a PerformanceBenchmark class in src/performance_benchmark.h and .cpp. This class will be responsible for running a series of loading tests.

        Metrics Struct: Define a BenchmarkResult struct to hold the detailed results of a single benchmark run, including timings, memory usage, and point count.

        Report Generation: Implement methods in PerformanceBenchmark to generate human-readable text reports and machine-readable JSON reports summarizing the results.

    List of Files being Created:

        src/performance_profiler.h / .cpp

        src/performance_benchmark.h / .cpp

    References between Files:

        e57parserlib.cpp and lasparser.cpp will include performance_profiler.h and use the profiling macros.

        The test suite will use performance_benchmark.h to run automated performance tests.

    Acceptance Criteria:

        The PerformanceProfiler can be enabled or disabled globally.

        When enabled, the profiler accurately measures and records the execution time of instrumented code sections.

        Calling PerformanceProfiler::generateReport() produces a text file with a clear breakdown of timings for each profiled section.

        The PerformanceBenchmark class can successfully run a loading test on a given E57 or LAS file and return a populated BenchmarkResult struct.

    Testing Plan:

        Unit Test: In tests/test_performance_profiler.cpp, create tests that call startSection and endSection with a known delay (QThread::msleep) and assert that the recorded duration is accurate within a small margin of error.

        Integration Test: In tests/demos/test_sprint2_2_profiling_demo.cpp, create a test application that runs the PerformanceBenchmark on the sample E57 and LAS files and verifies that a report file is generated.

User Story 3: Implement Stability Stress Testing

    User Story [3]: As a developer, I want to create a suite of stress tests for the memory management system to ensure it behaves correctly under pressure, remains stable, and does not leak memory.

    Description: This story focuses on validating the automatic memory management system developed in the previous sprint. We will create a dedicated test suite that intentionally pushes the PointCloudLoadManager beyond its memory limits to verify that the LRU eviction policy works as expected. The test will also monitor the application's overall memory footprint to detect potential memory leaks that could lead to long-term instability.

    Actions to Undertake:

        Stress Test Suite: Create a new test file, tests/demos/test_sprint2_2_stress.cpp.

        Test Setup: In the test, instantiate a PointCloudLoadManager and set a low, artificial memory limit (e.g., 100 MB) to make it easy to trigger the eviction logic.

        Load Loop: Programmatically load a series of mock or real point clouds in a loop. After each load, assert that the total memory usage reported by PointCloudLoadManager does not exceed the configured limit.

        Eviction Verification: After a known LRU eviction should have occurred, verify that the corresponding scan's state is now Unloaded in the ProjectTreeModel.

        Memory Leak Detection: Record the application's baseline memory usage before the test begins. After the test completes and all scans have been explicitly unloaded, record the final memory usage. The final usage should be within a small threshold (e.g., 10%) of the baseline.

    List of Files being Created:

        tests/demos/test_sprint2_2_stress.cpp

    References between Files:

        The new test file will include and interact heavily with pointcloudloadmanager.h.

    Acceptance Criteria:

        The application does not crash when the memory limit is reached during the stress test.

        The PointCloudLoadManager correctly identifies and unloads the least recently used scan when the memory limit is exceeded.

        After the stress test completes and all scans are unloaded, the application's memory usage returns to within 10% of its pre-test baseline.

    Testing Plan:

        The user story itself is a testing plan. The primary deliverable is the automated stress test.

4. Assumptions and Dependencies

    The state management and memory control logic from Sprint 2.1 is implemented and functional.

    A set of sample E57 and LAS files of varying sizes is available for benchmarking.

    The Qt Test framework is integrated into the project for creating the new unit and integration tests.

5. Non-Functional Requirements

    Performance: The performance profiling system itself must have minimal overhead (less than 5% impact on execution time) when enabled.

    Stability: The application must remain stable and responsive even when the memory management system is actively unloading scans in the background.

    Maintainability: The profiling macros should be clean and easy to add or remove from the codebase without causing compilation errors.

6. Conclusion

Sprint 2.2 is focused on refinement and hardening. By improving the UI's responsiveness to the application's state, we enhance the user experience. By introducing a formal profiling framework, we empower ourselves to make data-driven performance optimizations. And by rigorously stress-testing the memory management system, we ensure the application's stability, delivering a more reliable and professional tool.