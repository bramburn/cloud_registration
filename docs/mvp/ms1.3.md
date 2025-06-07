Sprint 1.3 Backlog: E57 Import and Visualization
1. Introduction

This document outlines the detailed backlog for Sprint 1.3. Following the establishment of robust E57 parsing and data integrity checks in Sprint 1.2, this sprint focuses on integrating these capabilities into the main application. The primary goal is to enable users to import E57 files through the UI and visualize the point cloud data in the 3D viewer. This sprint is a critical integration phase, bridging the gap between backend data processing and frontend user interaction. Successfully completing these user stories will provide the first end-to-end functionality for handling E57 files, a cornerstone of the MVP. This involves connecting the E57DataManager to the PointCloudLoadManager and ensuring that the data flows correctly and efficiently from file to renderer.

This phase is not just about connecting wires; it's about creating a seamless and reliable user experience. When a user imports a file, the application must respond intelligently, handling complex multi-scan files and providing clear, actionable feedback if something goes wrong. Similarly, when a user chooses to view a scan, the application must remain responsive, loading data asynchronously in the background while keeping the user informed of its progress. The successful execution of this sprint will validate the architectural decisions made in previous sprints and set a solid foundation for the subsequent registration and analysis features.
2. User Stories
User Story 1: Import E57 Files into the Project

    User Story: As a user, I want to be able to select and import one or more E57 files into my project, so that I can begin the registration process.

    Description: This story involves creating the user interface and backend logic for importing E57 files. The ScanImportManager will be updated to handle the .e57 extension and to use the E57DataManager for processing. The import process must be robust, providing clear feedback to the user on success or failure. This includes gracefully handling multi-scan E57 files by creating distinct, clearly named entries for each scan within the project structure. For example, importing Site-A.e57 which contains three scans might result in three items in the project tree: "Site-A - Scan 1", "Site-A - Scan 2", and "Site-A - Scan 3". Robust error handling is paramount; the system must detect and report issues such as invalid file formats, corrupted data (based on CRC checks from the previous sprint), or missing metadata without crashing. This means that if a user selects five files and one is invalid, the application should import the four valid ones and present a single, clear error message detailing which file failed and why (e.g., "File 'corrupt.e57' could not be imported because data corruption was detected.").

    Priority: Critical

    Estimated Effort: 5 Days

    Actions to Undertake:

        Modify the ScanImportDialog to include .e57 in its file filter, making it an explicitly supported and discoverable format for users.

        Update the ScanImportManager to recognize the .e57 extension. This involves adding logic to delegate the handling of these files to the E57DataManager, distinguishing it from the existing LAS file handling path.

        When an E57 file is imported, the ScanImportManager will first call the E57DataManager to perform an initial, lightweight metadata scan. This scan will not read the full point data but will quickly extract high-level information from the XML section, such as the number of Data3D sections (scans) and their respective GUIDs, names, and point counts.

        For each scan discovered within the E57 file, create and populate a new ScanInfo record. This record will be inserted into the project's SQLite database via the SQLiteManager. The record must include a newly generated unique scan ID, the scan name extracted from the E57 metadata, and a reference to the source E57 file path.

        The ProjectTreeModel must be updated to display the newly imported E57 scans. If a single E57 file contains multiple scans, they should appear as individual items in the project tree, logically grouped under a parent item representing the source file to maintain a clean and organized hierarchy.

        Implement comprehensive, robust error handling for the entire import process. This should involve try-catch blocks to handle specific exceptions thrown by the parsing modules (e.g., E57InvalidFileException, E57DataCorruptionException). Upon catching an exception, the ScanImportManager must present a user-friendly QMessageBox to the user with a clear, concise explanation of the problem. Crucially, no partial data from the failed file should be added to the project; the operation must be atomic for each file.

    Acceptance Criteria:

        The "Import Scans" dialog correctly shows and allows the selection of .e57 files alongside other supported formats.

        Importing an E57 file with multiple scans results in a separate, uniquely identified entry for each scan in the project tree. The naming convention must be clear and intuitive (e.g., "SourceFileName - ScanName").

        The application's SQLite database is correctly and transactionally updated with the metadata for each successfully imported scan. If an import fails for a file, the database state must remain unchanged for that file.

        Attempting to import a corrupted or invalid E57 file results in a non-blocking, user-friendly error dialog. No new items related to the failed file are added to the project tree.

        The main application UI must remain fully responsive and usable while the E57 metadata is being extracted during the import process.

User Story 2: Visualize E57 Point Cloud Data

    User Story: As a user, after importing an E57 file, I want to be able to click on a scan in the project tree and see its point cloud data rendered in the 3D viewer.

    Description: This story focuses on the data pipeline from the E57DataManager to the PointCloudViewerWidget. When a user requests to view a scan, the PointCloudLoadManager will orchestrate the process, using the E57DataManager to read the full point data from the correct binary section within the E57 file. This entire operation must be executed asynchronously to prevent the UI from freezing, which is especially critical for large, multi-gigabyte E57 files. The user should see an immediate visual indication that loading is in progress (e.g., a progress bar or spinner), and the point cloud should appear in the viewer upon successful completion. This data flow is the cornerstone of the application's interactive experience.

    Priority: Critical

    Estimated Effort: 6 Days

    Actions to Undertake:

        Implement the logic in PointCloudLoadManager to handle requests for viewing E57 scans. This requires creating a mapping between the scan ID selected in the project tree and the corresponding E57 source file path and internal scan GUID stored in the database.

        When a view request is received, the PointCloudLoadManager must delegate the file I/O and parsing to a background thread to prevent UI blocking. This can be achieved using QtConcurrent::run or by moving the E57DataManager to a dedicated QThread.

        Upon successful loading and parsing on the background thread, the PointCloudLoadManager will emit the pointCloudDataReady signal. The payload of this signal will be the loaded point data, structured as a std::vector<float> of interleaved XYZ coordinates, ready for GPU consumption.

        The MainWindow will have a slot connected to the pointCloudDataReady signal. To ensure thread safety, this connection must be a Qt::QueuedConnection. The slot's responsibility is to call the PointCloudViewerWidget's loadPointCloud method, passing the point data to the main UI thread.

        The PointCloudViewerWidget's loadPointCloud method will take the vector of points, upload it to a VBO on the GPU, and trigger a repaint to render the new data. As part of this process, it should also automatically calculate the point cloud's bounding box and adjust the 3D camera to frame the data appropriately, providing an immediate, optimal view.

        Implement a detailed progress reporting mechanism. The E57DataManager should emit progress updates (e.g., percentage of the binary section read) during the file read operation. The PointCloudLoadManager will catch these signals and re-emit them for the UI. The MainWindow will then update a QProgressBar in the status bar to provide continuous visual feedback to the user.

    Acceptance Criteria:

        Double-clicking an E57 scan item in the project tree successfully loads and displays its corresponding point cloud in the 3D viewer.

        The viewer's camera automatically adjusts to center and frame the newly loaded point cloud, ensuring it is immediately visible.

        The application UI remains fully responsive and interactive while a large E57 file (>1 GB) is loading in the background. The user can still interact with menus and other UI elements.

        A progress indicator in the status bar accurately reflects the data loading progress, from 0% to 100%.

        If the data loading fails (e.g., due to a CRC error detected by E57BinaryReader), a non-blocking, user-friendly error message is displayed in a dialog, and the 3D viewer remains in its previous state without displaying partial or corrupted data.

        Attempting to load another scan while one is already in progress should be handled gracefully, either by disabling the action or by queuing the request.

3. List of Files to be Modified/Created

    Modified: src/scanimportdialog.cpp/.h: Update UI to include .e57 in the list of supported file types for import. This ensures users can easily find and select their E57 files.

    Modified: src/scanimportmanager.cpp/.h: Add the core logic to handle the E57 import process. This includes interfacing with E57DataManager for metadata extraction and SQLiteManager for creating the new database entries for each scan.

    Modified: src/pointcloudloadmanager.cpp/.h: Implement the asynchronous data loading logic. This class will be responsible for using E57DataManager on a background thread to load the full point data and emitting the results via signals.

    Modified: src/mainwindow.cpp/.h: Act as the central hub connecting UI actions to the backend logic. This involves creating and connecting the necessary signals and slots for both E57 import and visualization, as well as managing the progress bar display.

    No New Files Expected: This sprint is primarily focused on integration and modification of existing components rather than the creation of new classes.

4. Testing Plan

    Unit Tests:

        ScanImportManager:

            Create a test case with a mock E57DataManager to verify that importing a multi-scan E57 file results in the correct number of calls to SQLiteManager::insertScan.

            Test the rejection of files with invalid extensions (e.g., .txt, .jpg) and ensure no processing is attempted.

            Simulate an exception being thrown by E57DataManager and verify that the ScanImportManager catches it and does not proceed.

        PointCloudLoadManager:

            Add tests to verify that a request to load an E57 scan correctly triggers a call to the E57DataManager.

            Using QSignalSpy, confirm that the pointCloudDataReady signal is emitted with correctly formatted data upon successful load.

            Test the error path: if E57DataManager reports a failure, ensure the pointCloudViewFailed signal is emitted with a descriptive error message.

    Integration Tests:

        Import Workflow:

            Set up an end-to-end test with a sample multi-scan E57 file.

            Programmatically trigger the import via ScanImportDialog.

            Verify that the ProjectTreeModel is updated with the correct number of new items, each correctly named.

            Directly query the test database to confirm that the ScanInfo records have been created correctly and contain the right metadata.

        Visualization Workflow:

            Pre-populate the project with an imported E57 scan.

            Simulate a user double-clicking the scan item in the SidebarWidget.

            Use QSignalSpy to listen for the PointCloudViewerWidget's data loading slot. Verify that it is called and that the std::vector<float> payload it receives is non-empty and has a size divisible by 3.

    Manual Tests:

        File Variety: Import single-scan and multi-scan E57 files from various real-world sources (e.g., FARO Scene, Leica Cyclone, ReCap) to check for compatibility issues.

        Corrupted Data: Attempt to import a known corrupted E57 file (e.g., with a manually altered CRC checksum in a hex editor). Verify that a clear, understandable error message is presented to the user.

        Performance and Responsiveness: Load a very large E57 file (>1 GB) and confirm that the UI remains responsive. The progress bar in the status bar should update smoothly and accurately. The cancellation feature should be tested mid-load.

        Viewer Interaction: After a scan is loaded, manipulate the 3D view extensively (orbit, pan, zoom) to ensure rendering is correct, stable, and performant.

5. Assumptions and Dependencies

    The E57DataManager and its constituent parsing components (E57HeaderParser, E57XmlParser, E57BinaryReader), developed and tested in Sprint 1.2, are assumed to be complete, stable, and functionally correct. Any bugs or performance bottlenecks in these underlying components will directly impact this sprint's success and will need to be addressed.

    The libe57format library is assumed to be correctly and stably integrated into the project's build environment via vcpkg. The successful compilation and linking of this external library is a non-negotiable hard dependency.

    The existing UI components (ScanImportDialog, SidebarWidget, PointCloudViewerWidget) are assumed to be sufficiently mature and their interfaces stable, allowing for the integration of new logic without requiring major, time-consuming refactoring.

    The development environment is assumed to have sufficient memory and processing power to handle the large test files that will be used for performance and stability testing.