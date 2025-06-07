Sprint 2.1 Backlog: Point Cloud State Management and Memory Control
1. Introduction

Following the successful integration of E57 file import and visualization in Sprint 1.3, this backlog for Sprint 2.1 focuses on implementing a robust system for managing the state and memory of point clouds within the application. The current implementation allows users to view point clouds, but lacks mechanisms to control which scans are active in memory or to prevent memory over-allocation when dealing with large datasets.

This sprint addresses these critical gaps by introducing three core features:

    State Management: Providing clear visual feedback in the UI to indicate whether a scan is loaded, unloaded, or in a transient state.

    Manual Control: Empowering users to explicitly load and unload scans to manage system resources.

    Automatic Memory Management: Implementing a memory-aware system that automatically unloads less-used scans to maintain application stability.

Completing this sprint will significantly enhance the application's usability and robustness, making it capable of handling large, multi-scan projects without compromising performance or stability.
2. User Stories
User Story 1: Implement Loaded State Management in the UI

    User Story [1]: As a user, I want to see the current state (e.g., loaded, unloaded, loading, error) of each scan and cluster in the project tree, so I can immediately understand what data is active in memory and what is not.

    Description: This story focuses on the visual feedback layer. The ProjectTreeModel will be enhanced to track the loaded state of each item. The SidebarWidget, which uses this model, will then display different icons or styles for each state, making the application's memory usage intuitive at a glance. For example, a loaded scan might have a green icon, while an unloaded one has a grey icon. Clusters will reflect the state of their children, showing a "partially loaded" icon if some but not all of their scans are in memory.

    Actions to Undertake:

        Enum Creation: Define a LoadedState enum (Unloaded, Loaded, Partial, Loading, Error) in projecttreemodel.h to represent all possible states of a tree item.

        State Tracking: In ProjectTreeModel, implement a mechanism (e.g., a QHash<QString, LoadedState>) to store the current state for each scan ID.

        Cluster State Logic: Implement a method ProjectTreeModel::updateClusterLoadedStates() that iterates through clusters and calculates their state (Loaded, Unloaded, or Partial) based on the collective state of their child scans.

        Manager Integration: The PointCloudLoadManager must be updated to call ProjectTreeModel::setScanLoadedState() whenever a scan's state changes (e.g., after a successful load, a failed load, or an unload operation).

        Visual Representation: In ProjectTreeModel::data(), override the Qt::DecorationRole. Use the IconManager to return a different icon for each LoadedState. This will visually distinguish loaded items from unloaded ones in the SidebarWidget.

    References between Files:

        pointcloudloadmanager.cpp will call methods in projecttreemodel.h.

        projecttreemodel.cpp will need to know about the LoadedState enum and will use iconmanager.h to fetch state-specific icons.

        sidebarwidget.cpp will display the icons and styles provided by the projecttreemodel.cpp.

    Acceptance Criteria:

        Scans in the project tree display a distinct icon when loaded into memory.

        Unloaded scans display a different, "inactive" icon.

        A scan being loaded displays a "loading" indicator icon.

        A cluster containing a mix of loaded and unloaded scans displays a "partially loaded" icon.

        A cluster where all scans are loaded displays a "fully loaded" icon.

        If a scan fails to load, it displays an "error" icon.

    Testing Plan:

        Unit Test: In tests/test_projecttreemodel.cpp, create tests to verify that setScanLoadedState correctly updates the internal state and that getScanLoadedState returns the correct state.

        Integration Test: In tests/demos/test_sprint2_1_integration.cpp, simulate loading a scan via PointCloudLoadManager and assert that the corresponding item's data role in ProjectTreeModel is updated to LoadedState::Loaded.

User Story 2: Implement Manual Scan Loading and Unloading

    User Story [2]: As a user, I want to be able to manually load and unload individual scans or entire clusters from memory using a context menu, so I can have direct control over the application's resource usage.

    Description: This story gives the user direct control over memory. The SidebarWidget will be enhanced with a right-click context menu. When a user right-clicks an unloaded scan, a "Load" option will appear. For a loaded scan, an "Unload" option will be available. These actions will trigger the PointCloudLoadManager to perform the corresponding operation and update the application state. The same functionality will apply to clusters, loading or unloading all child scans at once.

    Actions to Undertake:

        Context Menu Logic: In sidebarwidget.cpp, override contextMenuEvent. Based on the selected item's type ("scan" or "cluster") and its current LoadedState (retrieved from the model), dynamically build a QMenu.

        Menu Actions: Create QAction members in SidebarWidget for "Load Scan", "Unload Scan", "Load Cluster", and "Unload Cluster".

        Signal Emission: Connect the triggered() signal of each QAction to new slots in SidebarWidget. These slots will emit corresponding signals (e.g., loadScanRequested(scanId)).

        Load Manager API: In pointcloudloadmanager.h, create public slots (onLoadScanRequested, onUnloadScanRequested, etc.) to receive these signals.

        Core Logic: Implement the private logic in PointCloudLoadManager to handle loading (reading data from disk) and unloading (freeing memory associated with a scan) of point cloud data. Unloading should clear the point data from the internal cache and update the memory usage counter.

    References between Files:

        sidebarwidget.cpp will emit signals like loadScanRequested(QString).

        pointcloudloadmanager.h will declare slots to receive these signals.

        pointcloudloadmanager.cpp will contain the core implementation for loading/unloading data and updating the ProjectTreeModel.

    Acceptance Criteria:

        Right-clicking an unloaded scan in the SidebarWidget shows a "Load Scan" context menu option.

        Right-clicking a loaded scan shows an "Unload Scan" option.

        Selecting "Load Scan" successfully loads the point cloud data into memory and updates its icon to the "loaded" state.

        Selecting "Unload Scan" frees the point cloud data from memory and updates its icon to the "unloaded" state.

        The "Load Cluster" and "Unload Cluster" actions correctly apply the operation to all child scans within that cluster.

    Testing Plan:

        Manual Test: Right-click various items in the sidebar to verify the correct context menu appears. Select "Load" and "Unload" and confirm the icon changes and memory usage (in a future story's UI) is updated.

        Integration Test: In tests/demos/test_sprint2_1_integration.cpp, get a scan item from the model, emit the loadScanRequested signal from a mock sidebar, and verify that the PointCloudLoadManager loads data and updates the model's state.

User Story 3: Implement Automatic Memory Management

    User Story [3]: As a developer, I want the application to automatically unload the least recently used (LRU) scans when memory usage exceeds a configurable limit, so that the application remains stable and avoids crashing when working with very large projects.

    Description: This is a critical stability feature. The PointCloudLoadManager will be enhanced to act as a memory manager. It will track the memory footprint of each loaded scan and maintain a total memory count. When a new scan is loaded that would push the total usage over a predefined limit (e.g., 2GB), the manager will automatically identify and unload the least recently used scan(s) to make room.

    Actions to Undertake:

        Memory Tracking: In the PointCloudData struct (pointcloudloadmanager.h), add a size_t memoryUsage member. When parsing a file, calculate and store the memory footprint of the point data.

        Access Timestamp: In the ScanLoadState struct, add a QDateTime lastAccessed member. This timestamp should be updated every time a scan's data is accessed (e.g., for viewing).

        Memory Limit: In PointCloudLoadManager, add a member variable for the memory limit (e.g., m_memoryLimitMB) and a method setMemoryLimit(size_t).

        Enforcement Logic: In PointCloudLoadManager::loadScanData, before loading new data, check if adding it will exceed the memory limit.

        LRU Eviction: If the limit will be exceeded, implement an evictLeastRecentlyUsed() method. This method will find the loaded scan with the oldest lastAccessed timestamp, call unloadScanData() on it, and repeat until enough memory is freed.

        Periodic Check: Implement a QTimer (m_memoryCheckTimer) in PointCloudLoadManager to periodically call a method like enforceMemoryLimit() to ensure the limit is respected even if no new scans are being loaded.

    References between Files:

        All changes are internal to pointcloudloadmanager.h and pointcloudloadmanager.cpp.

    Acceptance Criteria:

        PointCloudLoadManager accurately tracks the total memory usage of all loaded scans.

        When loading a new scan that would exceed the configured memory limit, one or more of the least recently used scans are automatically unloaded.

        The total memory usage stays at or below the configured limit after the eviction process.

        Unloading a scan correctly decrements the total memory usage count.

    Testing Plan:

        Unit Test: Create a test in tests/test_pointcloudloadmanager.cpp. Mock several ScanLoadState objects with different memory sizes and lastAccessed timestamps. Call a mock loadScan that exceeds the limit and verify that the correct LRU scan is identified for eviction.

3. List of Files to be Created

    No new files are expected for this sprint. All work will be modifications to existing files.

Files to be Modified:

    src/pointcloudloadmanager.h / .cpp

    src/projecttreemodel.h / .cpp

    src/sidebarwidget.h / .cpp

4. Assumptions and Dependencies

    The E57DataManager and LasParser are stable and can reliably provide point cloud data.

    The SQLiteManager and existing project structure are functional for retrieving scan metadata.

    The SidebarWidget and ProjectTreeModel can be extended without major refactoring.

    The IconManager exists and can provide icons for different states as required by the ProjectTreeModel.

5. Non-Functional Requirements

    Performance: The UI must remain responsive. Loading and unloading operations must be performed asynchronously to prevent freezing the main thread.

    Memory: Memory usage calculations must be reasonably accurate (within 5% of actual usage). The eviction process should be efficient and not cause noticeable stalls.

    Usability: The state icons and context menu actions must be clear and intuitive to the user.

6. Conclusion

Sprint 2.1 is a pivotal step in maturing the application from a simple file viewer into a professional tool capable of managing large-scale projects. By implementing state management, manual memory controls, and an automatic eviction policy, this sprint will deliver a more stable, responsive, and user-empowering experience, directly addressing the core requirements of handling substantial point cloud datasets.