Product Requirements Document: Cloud Registration Application Enhancements - Phase 4 Onwards
1. Introduction

This document outlines the requirements for the next phases of development for the Cloud Registration Application, building upon the features implemented up to Sprint 3.3. The primary focus of these subsequent enhancements is to enrich the data model with more detailed scan information, prepare the system for core registration functionalities by establishing necessary data storage, and improve user feedback regarding application performance and data density.
2. Goals

    **Enhance Data Richness:** Automatically extract and store comprehensive metadata (point counts, bounding boxes) from scan files.
    **Prepare for Registration:** Implement the database schema required to store registration results, statuses, and transformation matrices.
    **Improve Project Context:** Allow users to add more descriptive information to their projects (tags, coordinate systems).
    **Increase User Awareness:** Provide users with real-time feedback on memory usage and basic controls for managing display performance (LOD).
    **Complete Foundational Elements:** Address the remaining optional items from Sprint 3.4 of the original PRD to solidify the application's core.

3. Target Users

    (As per original PRD: Surveyors, Engineers, Architects, Construction Professionals, etc.)

4. Proposed Features (Continuing from original PRD)

This PRD details features primarily related to completing data handling aspects and preparing for advanced registration capabilities.

**Phase 4: Core Data Enhancement & Initial Registration Prep (Estimated 6-8 weeks)**

    Goal: Fully implement scan metadata extraction, establish the database schema for registration data, and introduce initial performance-related UI feedback and controls.

    **Sprint 4.1: Full Scan Metadata Extraction & Enhanced Project Info (2 weeks)**

        **User Story 1: Automatic Extraction of Scan Point Count & Bounding Box**
            * **Description:** As a user, when I import a scan file (.las, .e57), I want the application to automatically parse its header to determine the actual point count and calculate its spatial bounding box, so this accurate metadata is stored and available for project management and future processing.
            * **Actions to Undertake:**
                * Implement header parsing logic within `ScanImportManager` (or dedicated parser utilities like `ScanFileHeaderParserLAS`, `ScanFileHeaderParserE57`) for both .las and .e57 file formats.
                * This logic should extract:
                    * Total number of points.
                    * Minimum and maximum X, Y, Z coordinates (bounding box).
                * Modify the scan import process in `ScanImportManager` to call this parsing logic and populate the `point_count_estimate` and `bounding_box_min/max_x/y/z` fields in the `ScanInfo` struct with actual values.
                * Ensure these actual values are then saved to the `Scans` table in `project_data.sqlite` by `SQLiteManager`.
            * **References between Files:**
                * `ScanImportManager.cpp` (or new parser utilities) <-> File System (to read scan file headers).
                * `ScanImportManager.cpp` -> `SQLiteManager.cpp` (to store extracted metadata).
                * `project_data.sqlite` (Scans table updated).
            * **Acceptance Criteria:**
                * When a .las or .e57 file is imported, its `point_count_estimate` in the SQLite `Scans` table reflects the actual number of points in the file.
                * The `bounding_box_min/max_x/y/z` fields in the `Scans` table accurately represent the spatial extents of the scan data.
                * The import process remains performant, even with header parsing (consider efficient header-only reads).
                * Graceful error handling if a file header is corrupt or unparsable for this metadata.
            * **Testing Plan:**
                * Test Case 1.1: Import various valid .las files (different PDRFs, versions) with known point counts and bounding boxes. Verify SQLite data.
                * Test Case 1.2: Import various valid .e57 files with known point counts and bounding boxes. Verify SQLite data.
                * Test Case 1.3: Attempt to import a scan file with a corrupted header section. Verify error handling and that no incorrect metadata is stored.

        **User Story 2: Store User-Defined Tags and Coordinate System in Project Metadata**
            * **Description:** As a user, I want to be able to define custom tags and specify a project-level coordinate system (e.g., EPSG code or WKT string) for my project, so I can better categorize and contextualize my work. This information should be saved in the `project_meta.json` file.
            * **Actions to Undertake:**
                * Extend the `ProjectMetadata` struct (in `ProjectManager.h`) to include fields for `tags` (e.g., `QStringList`) and `coordinateSystem` (e.g., `QString`).
                * Modify `ProjectManager::saveProjectMetadataTransactional()` to write these new fields to `project_meta.json`.
                * Modify `ProjectManager::loadProjectMetadataWithValidation()` to read these fields from `project_meta.json`.
                * (UI for editing these fields is out of scope for this sprint but could be a simple placeholder in `CreateProjectDialog` or a future project properties dialog). For now, focus on storage and retrieval.
            * **References between Files:**
                * `ProjectManager.h` (struct update).
                * `ProjectManager.cpp` (save/load logic).
                * `project_meta.json` (schema extended).
            * **Acceptance Criteria:**
                * The `project_meta.json` file can store a list of user-defined tags.
                * The `project_meta.json` file can store a string representing the project's coordinate system.
                * This information is correctly loaded when a project is opened and saved when a project is saved.
            * **Testing Plan:**
                * Test Case 2.1: Manually add `tags` (e.g., `["survey", "buildingA"]`) and `coordinateSystem` (e.g., `"EPSG:4326"`) to a `project_meta.json`. Open the project and verify (e.g., via debugger or logging) that `ProjectManager` loads these values.
                * Test Case 2.2: (If placeholder UI exists) Set tags and coordinate system, save project, close, reopen. Verify values persist.

    **Sprint 4.2: SQLite Schema for Registration & Basic Matrix Storage (2 weeks)**

        **User Story 1: Implement SQLite Tables for Registration Status and Transformation Matrices**
            * **Description:** As a developer, I need the `RegistrationStatus` and `TransformationMatrices` tables, as defined in the original PRD (Sprint 3.4), to be created in the `project_data.sqlite` database, so the application has the necessary schema to store future registration results.
            * **Actions to Undertake:**
                * In `SQLiteManager.cpp`, add the DDL (Data Definition Language) statements to `initializeSchema()` (or a schema migration function) to create the `RegistrationStatus` table:
                    * `item_id` (TEXT, PK) - Can be scan_id or cluster_id
                    * `item_type` (TEXT) - "SCAN" or "CLUSTER"
                    * `status` (TEXT) - e.g., "UNREGISTERED", "REGISTERED_MANUAL", "REGISTERED_AUTO"
                    * `error_metric_value` (REAL, NULLABLE)
                    * `error_metric_type` (TEXT, NULLABLE) - e.g., "RMSE"
                    * `last_registration_date` (TEXT, ISO 8601 Timestamp)
                * In `SQLiteManager.cpp`, add DDL to create the `TransformationMatrices` table:
                    * `item_id` (TEXT, PK)
                    * `item_type` (TEXT)
                    * `matrix_m00` to `matrix_m33` (REAL) for 16 matrix elements (or a single TEXT/BLOB field for serialized matrix). Storing individual elements is often better for querying.
                    * `relative_to_item_id` (TEXT, NULLABLE) - If relative to another scan/cluster, or NULL for project origin.
                    * `last_transform_date` (TEXT, ISO 8601 Timestamp)
                * Ensure these tables are created if they don't exist when a project is opened or a new project is created.
                * Handle basic schema migration if opening a project created before these tables existed (i.e., add tables if missing).
            * **References between Files:**
                * `SQLiteManager.cpp` (DDL implementation).
                * `project_data.sqlite` (schema updated).
            * **Acceptance Criteria:**
                * When a new project is created, `project_data.sqlite` contains empty `RegistrationStatus` and `TransformationMatrices` tables with the correct schema.
                * When an older project (without these tables) is opened, the tables are added to its `project_data.sqlite`.
                * The application does not crash if these tables already exist.
            * **Testing Plan:**
                * Test Case 1.1: Create a new project. Use an SQLite browser to verify the `RegistrationStatus` and `TransformationMatrices` tables exist with all specified columns and correct types.
                * Test Case 1.2: Manually remove these tables from an existing project's SQLite file. Open the project in the application. Verify the tables are re-created.

        **User Story 2: Basic Storage of Identity Transformation Matrix (Placeholder)**
            * **Description:** As a developer, when a new scan or cluster is added to the project, I want a placeholder identity transformation matrix to be stored for it in the `TransformationMatrices` table, so that every item has an initial matrix entry.
            * **Actions to Undertake:**
                * Modify `ProjectManager` (or relevant manager for scan/cluster creation/import) to insert a new record into `TransformationMatrices` when a scan or cluster is created/imported.
                * The matrix stored should be an identity matrix (diagonal 1s, rest 0s).
                * `item_type` should be set to "SCAN" or "CLUSTER".
                * `relative_to_item_id` can be NULL initially (implying relative to project origin).
            * **References between Files:**
                * `ProjectManager.cpp` / `ScanImportManager.cpp` / `ClusterManager.cpp` -> `SQLiteManager.cpp`.
                * `SQLiteManager.cpp` (DML for inserting into `TransformationMatrices`).
            * **Acceptance Criteria:**
                * Upon importing a new scan, an identity matrix record is added for it in `TransformationMatrices`.
                * Upon creating a new cluster, an identity matrix record is added for it in `TransformationMatrices`.
            * **Testing Plan:**
                * Test Case 2.1: Import a new scan. Verify an identity matrix record is created for it in `TransformationMatrices` in SQLite.
                * Test Case 2.2: Create a new cluster. Verify an identity matrix record is created for it.

    **Sprint 4.3: UI for Memory Stats & Basic LOD Controls (2 weeks)**

        **User Story 1: Display Live Memory Usage in UI**
            * **Description:** As a user, I want to see a display in the application's UI (e.g., status bar) that shows an estimate of the current memory being used by loaded point cloud data, so I can monitor resource consumption.
            * **Actions to Undertake:**
                * Ensure `PointCloudLoadManager` accurately tracks the memory footprint of loaded point clouds (e.g., `points.size() * sizeof(float)` for coordinates, plus color/intensity if applicable).
                * `PointCloudLoadManager` should emit a signal (e.g., `memoryUsageChanged(quint64 bytes)`) whenever this usage changes (on load/unload).
                * In `MainWindow`, add a `QLabel` to the status bar dedicated to displaying memory usage.
                * Connect this label to the `memoryUsageChanged` signal from `PointCloudLoadManager`.
                * Format the display in user-friendly units (e.g., MB or GB).
            * **References between Files:**
                * `PointCloudLoadManager.h/.cpp` (memory tracking and signal emission).
                * `MainWindow.h/.cpp` (status bar UI element and slot to update it).
            * **Acceptance Criteria:**
                * A label in the status bar displays current estimated memory usage by loaded point clouds.
                * The value updates correctly when scans/clusters are loaded into memory.
                * The value updates correctly (decreases) when scans/clusters are unloaded from memory.
                * Memory is displayed in MB or GB.
            * **Testing Plan:**
                * Test Case 1.1: Launch application with no project. Memory display should be 0 or minimal.
                * Test Case 1.2: Load a scan. Verify memory display updates to a reasonable value.
                * Test Case 1.3: Load multiple scans. Verify memory display reflects the sum.
                * Test Case 1.4: Unload a scan. Verify memory display decreases. Unload all. Verify it returns to near zero.

        **User Story 2: Implement Basic LOD Toggle/Control in Viewer**
            * **Description:** As a user, I want a simple way to toggle a basic Level of Detail (LOD) or adjust point cloud density in the viewer (e.g., via a debug menu or a simple slider), so I can improve rendering performance for large clouds.
            * **Actions to Undertake:**
                * Implement a basic LOD mechanism in `PointCloudLoadManager` or `PointCloudViewerWidget`. This could be a simple random subsampling (e.g., display 50% of points) or a voxel grid filter applied on the fly if fast enough.
                * Add a UI control (e.g., a checkbox "Enable Basic LOD" in a view menu, or a simple slider for "Point Density [10%-100%]") to `MainWindow` or `PointCloudViewerWidget`.
                * When the LOD setting is changed:
                    * If LOD is enabled/density reduced, the `PointCloudViewerWidget` should re-process or request subsampled data from `PointCloudLoadManager` and re-render.
                    * If LOD is disabled/density increased, the viewer should render the full data.
                * This is a prototype; focus on functionality over visual perfection of LOD transitions.
            * **References between Files:**
                * `PointCloudViewerWidget.h/.cpp` (LOD rendering logic, UI control handling).
                * `PointCloudLoadManager.h/.cpp` (potential data subsampling logic).
                * `MainWindow.h/.cpp` (if UI control is placed in main menus).
            * **Acceptance Criteria:**
                * A UI control allows toggling/adjusting a basic LOD.
                * Activating LOD results in a visibly less dense point cloud in the viewer.
                * Deactivating LOD restores the full point cloud display.
                * A noticeable performance difference (e.g., frame rate) is observable with LOD on vs. off for a very large scan.
            * **Testing Plan:**
                * Test Case 2.1: Load a large scan. Enable LOD. Verify point density decreases visually.
                * Test Case 2.2: Toggle LOD on and off multiple times. Verify viewer updates correctly.
                * Test Case 2.3: (Informal) Observe frame rate or responsiveness with LOD on vs. off for a large scan.

5. Future Considerations (Post Phase 4)

    * Actual registration algorithms (Top View, Target-based, Cloud-to-Cloud).
    * UI for managing registration status and transformations.
    * Advanced LOD techniques (octree-based, dynamic streaming).
    * Editing project metadata (tags, coordinate system) via UI.
    * And other items from the original PRD's "Future Considerations."

This PRD provides a structured plan to complete the remaining foundational elements, focusing on data enrichment and preparing the application for its core registration capabilities. Each sprint builds upon the last, ensuring a stable and progressively more feature-rich application.