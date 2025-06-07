Product Requirements Document: Cloud Registration Application Enhancements
1. Introduction

This document outlines the requirements for enhancing our cloud registration application. The primary focus of these enhancements is to improve project organization, scan management, and the user's ability to prepare data for registration through a more intuitive and powerful interface. Key features include a hierarchical project sidebar, robust project setup and management, flexible scan file handling, and cluster-based organization of scans, similar to functionalities found in industry-standard software like FARO SCENE.
2. Goals

    Improve Project Organization: Provide users with a clear, hierarchical way to manage and navigate their scan projects, scans, and clusters.

    Streamline Scan Import: Offer flexible options for importing scan files (.las, .e57) into projects, including choices for file movement and source file retention.

    Enhance Data Management: Implement a structured approach for storing project-level information and detailed scan/cluster data.

    Facilitate Pre-Registration Setup: Enable users to group scans into logical clusters and sub-clusters, and manage their state (e.g., lock/unlock) in preparation for registration.

    Improve User Experience: Create an intuitive sidebar and project management interface that allows for efficient interaction with scan data, starting from an accessible project hub.

3. Target Users

    Surveyors

    Engineers

    Architects

    Construction Professionals

    Anyone working with 3D point cloud data requiring registration and analysis.

4. Proposed Features
4.1. Project Setup and Management

    New Project Creation:

        Users shall be able to create a new project via the startup UI/Project Hub.

        This process shall include prompting the user to select a name and a local folder where the project will be saved.

        Upon creation, a dedicated project folder shall be established in the selected location.

        The project folder will house all project-related files, including imported scans, project metadata, and databases.

    Project Metadata:

        A .json file within the project folder shall store general project-level information (e.g., project ID, project name, creation date, description, user-defined tags, coordinate system if defined, and in the future: geolocation, specific location details, client name).

    Open Existing Project:

        Users shall be able to open existing projects by selecting them from a list of recent projects or by browsing to the project folder via the startup UI/Project Hub.

    Project Structure Persistence:

        The application shall save and reload the project's structure, including scan organization and cluster hierarchy, primarily managed through the SQLite database.

    Recent Projects List:

        The application shall maintain and display a list of recently opened projects on the startup UI/Project Hub for quick access.

4.2. Sidebar (Hierarchical Tree Structure)

    Tree View:

        The application shall feature a sidebar with a tree-like structure to display the project hierarchy once a project is open.

        The root of the tree shall be the main project folder.

    Display Elements:

        Scans: Individual scan files (.las, .e57) shall be displayed as items in the tree.

        Clusters/Sub-folders: Users shall be able to create folders (clusters) and sub-folders within the project to organize scans. These shall be displayed as expandable/collapsible nodes in the tree.

    Contextual Information:

        The sidebar should ideally provide visual indicators for the state of scans and clusters (e.g., loaded, unloaded, locked, registration status - future).

    Interaction:

        Right-Click Context Menus: Right-clicking on items in the tree (scans, clusters) shall open a context menu with relevant actions.

4.3. Scan Management

    Scan Import:

        After a project is created or opened, users shall be guided or provided with clear options to import scan files (e.g., .las, .e57) into the current project.

        File Handling Options: Upon import, users shall be prompted with options:

            Move to Project Folder: Move the original scan file into the project's dedicated scan folder.

            Copy to Project Folder: Copy the original scan file into the project's scan folder, leaving the source file untouched.

            Link to Source (Keep Source): Keep the scan file in its original location and create a link/reference within the project. The application should clearly indicate if linked files are missing or inaccessible.

        Imported scans shall be added to the sidebar tree structure, initially under the main project folder or a user-selected cluster.

    Scan Loading & Unloading (Memory Management):

        Load Scan: Users shall be able to load one or more selected scans (or all scans within a cluster) into memory for viewing and processing. This action will make the point cloud data available in the main viewing window.

        Unload Scan: Users shall be able to unload scans from memory to conserve system resources. Unloaded scans remain part of the project but their point cloud data is not actively held in RAM.

        The sidebar should visually indicate whether a scan is currently loaded or unloaded (e.g., FARO SCENE uses a blue square for loaded scans).

    View Point Cloud:

        Through a right-click context menu on a scan (or cluster), users shall be able to "View Point Cloud."

        This action will load the selected scan(s) (if not already loaded) and display the point cloud data in the main application window.

    Multiple Scans:

        The project shall support multiple scans within the main project folder and within clusters/sub-folders.

4.4. Cluster Management

    Cluster Creation:

        Users shall be able to create new clusters (folders) within the project via the sidebar (e.g., right-click context menu on the project root or another cluster).

    Sub-folder Creation:

        Users shall be able to create sub-folders (nested clusters) within existing clusters to further organize scans.

    Organizing Scans:

        Users shall be able to drag and drop scans between clusters/folders in the sidebar.

    Load/Unload Cluster:

        Right-clicking a cluster shall provide options to "Load All Scans in Cluster" and "Unload All Scans in Cluster."

    Lock/Unlock Cluster:

        Users shall be able to "Lock" and "Unlock" clusters (similar to FARO SCENE).

        Locked Cluster: Prevents modifications to the scans within the cluster and their relative positions during registration processes involving other clusters. The cluster itself can still be transformed as a rigid unit.

        Unlocked Cluster: Allows scans within the cluster to be adjusted during registration.

        The locked/unlocked state should be visually indicated in the sidebar.

4.5. Data Storage

    Project Information (.json file):

        A .json file within the project root folder will store general project-level information:

            Project ID (unique identifier).

            Project name, description, creation date, last modified date.

            User-defined tags or categories.

            Defined project coordinate system (if any).

            Future extensions: geolocation data (latitude, longitude, altitude), specific site/location address, client name, project lead, etc.

            Pointers or references to essential project files (like the SQLite database name/path relative to the project folder).

            List of recently opened project paths (this might be stored globally or per user profile rather than in each project's JSON).

    Scan, Cluster, and Hierarchy Data (.sqlite database):

        A .sqlite database file within the project folder will store:

            Cluster Hierarchy & Structure: Detailed information about clusters, including cluster ID, parent cluster ID (for hierarchy), name, creation date, and state information (e.g., locked/unlocked).

            Scan Membership: Mapping of scans to their respective parent clusters.

            Detailed Scan Metadata: Scan ID, file path (relative to project folder if copied/moved, or absolute if linked), import type (copied, moved, linked), original source path (if linked), estimated point count, calculated bounding box, date added to project, last modified date of scan data.

            Registration Status: For individual scans and clusters (e.g., unregistered, processing, registered, failed, requires review). This includes storing overall registration error metrics per scan/cluster.

            Transformation Matrices: (e.g., 4x4 matrix) for each scan and cluster relative to a project origin or parent cluster once registered.

            Control Points & Targets (Future): Information about surveyed control points, user-defined points, detected/manual registration targets (type, size, detected locations in scans), and registration constraints (e.g., plane-to-plane, point-to-point correspondences).

5. User Interface (UI) / User Experience (UX) Considerations
5.1. Application Startup / Project Hub

    Initial View: On application launch, users shall be presented with a startup screen or "Project Hub."

    Recent Projects: This screen shall display a list of recently opened projects.

        Each item in the list should show the project name and its file path.

        Clicking a recent project shall open it directly.

        The list should be ordered by most recently opened.

        Consider options to remove projects from the recent list or clear the list.

    Create New Project: A clear button or option to "Create New Project."

        This action shall open a dialog prompting the user for a project name and to select a directory where the project folder will be saved.

        After successful creation, the application should open the new project and ideally guide the user to the scan import functionality.

    Open Existing Project: A button or option to "Open Project..." which opens a system file dialog allowing users to browse to and select a project folder.

    User Guidance: After a new project is created, the UI should make it obvious how to proceed, for example, by highlighting an "Import Scans" button or showing a contextual message.

5.2. In-Project UI/UX

    Sidebar:

        Intuitive Hierarchy: The tree structure should be easy to understand and navigate, similar to a file explorer. Standard expand/collapse icons and clear labeling are essential.

        Clear Visual Cues: Icons should distinctly differentiate between scans, clusters, and potentially other project elements (e.g., registration objects, measurements - future). Visual indicators for loaded/unloaded status (e.g., filled vs. empty icon, color change) and locked/unlocked status (e.g., lock icon overlay) are crucial for at-a-glance understanding.

        Responsive Interaction: Drag-and-drop for organizing scans and clusters should be smooth and provide clear visual feedback during the drag operation (e.g., highlighting potential drop targets). Context menus should be easily accessible (right-click) and provide relevant, context-aware options.

    Project Setup (Post-Hub):

        Clear, step-by-step dialogs for creating new projects (e.g., specifying project name, location) if not handled entirely by the hub.

        Intuitive dialogs for importing scans, allowing multi-selection of files.

        Explicit choices for scan file handling (move, copy, link) with clear explanations of the implications of each option (e.g., disk space usage, data integrity if source files are moved/deleted).

    Loading/Unloading:

        Non-blocking feedback to the user during loading/unloading operations, especially for large scans (e.g., progress indicators in the status bar or next to the scan item in the sidebar). The UI should remain responsive.

        Clear indication when an operation is complete or if an error occurred during loading/unloading.

    Main View:

        The main window should dynamically update to display point clouds of selected/loaded scans or clusters.

        Consider a "welcome" or "empty project" state for the main view when a project is open but no scans are loaded, with clear calls to action (e.g., "Import Scans").

6. Technical Considerations (Initial Focus)

    Sidebar Implementation: Utilize a suitable tree view component (e.g., QTreeView if using Qt) that supports custom item delegates for visual indicators and icons.

    Data Model: Develop a robust and efficient data model (e.g., using Qt's model/view framework) to back the tree view, primarily reflecting the project structure stored in the SQLite database. The .json file will provide top-level project context.

    File I/O: Efficiently handle reading and writing of .json and .sqlite files. Consider asynchronous operations for saving to prevent UI freezes.

    Point Cloud Loading: Initial implementation can focus on loading entire point clouds. Future iterations should consider partial loading, Level of Detail (LOD) streaming, and out-of-core rendering techniques for very large datasets to manage memory effectively.

    Recent Projects Cache: Implement a mechanism (e.g., separate settings file, system registry) to store the list of recent project paths.

7. Development Phases and Sprints

This section outlines a potential breakdown of the development into phases and sprints. Sprint duration is assumed to be 2 weeks.
Phase 1: Core Project and Sidebar Foundation (Estimated 6-8 weeks)

    Goal: Establish basic project creation via a startup UI, scan import, and a functional sidebar for organizing scans into clusters.

    Sprint 1.1: Startup UI & Project Creation/Opening

        Features:

            Implement the "Application Startup / Project Hub" UI (5.1).

                Display list of recent projects (initially empty, mechanism to store/load paths).

                "Create New Project" option: Dialog for project name and folder selection. Creates project folder and basic .json (project ID, name, creation date).

                "Open Project" option: Browse to select project folder, load basic .json.

            Basic sidebar UI (e.g., QTreeView) to display project root once a project is open.

        Deliverables: User can launch the app to a startup screen. From there, they can create a new project (folder and .json are made) or open an existing one. The selected project's root is shown in the sidebar.

    Sprint 1.2: Scan Import & Initial Display in Sidebar

        Features:

            After project creation/opening, guide user or provide clear UI element to "Import Scans" (4.3).

            Implement "Scan Import" (4.3): Focus on "Copy to Project Folder" and "Move to Project Folder" options. Include a file dialog for selecting scans.

            Display imported scans as simple items (with filename) under the project root or a default "Scans" folder in the sidebar.

            Set up initial .sqlite database upon project creation. Store basic scan information (ID, name, path within project, import type) in an SQLite Scans table.

        Deliverables: Users can import scans (copied or moved to project folder) and see them listed in the sidebar. Scan metadata is stored in SQLite. Post-creation, user is prompted/guided to import scans.

    Sprint 1.3: Basic Cluster Creation & Scan Organization

        Features:

            Implement "Cluster Creation" (4.4): Allow creating new folders (clusters) in the sidebar via context menu (on project root or other clusters). Assign unique IDs to clusters.

            Implement "Sub-folder Creation" (4.4) to allow nested cluster structures.

            Store cluster hierarchy (e.g., cluster_id, name, parent_cluster_id) and scan membership (e.g., scan_id, parent_cluster_id) in SQLite tables.

            Implement drag-and-drop of scans into clusters/sub-folders in the sidebar. Update SQLite on successful drop.

        Deliverables: Users can create a hierarchical structure of clusters and sub-folders. Scans can be dragged and dropped into these clusters, and the structure is saved in the project SQLite database.

Phase 2: Scan & Cluster Management Functionality (Estimated 6 weeks)

    Goal: Enable loading/unloading of scans and clusters, introduce advanced import options, and refine SQLite data storage.

    Sprint 2.1: Scan/Cluster Loading, Unloading & View Action

        Features:

            Implement "Load Scan" (4.3): Basic in-memory loading of point cloud data (initially, this could be just reading file headers or a small subset of points as a placeholder for actual rendering).

            Implement "Unload Scan" (4.3): Release memory associated with the loaded scan.

            Implement "Load All Scans in Cluster" and "Unload All Scans in Cluster" (4.4) from cluster context menu.

            Add visual indicators (e.g., icons, text styling) in the sidebar for loaded/unloaded state of scans and clusters. A cluster's loaded state could reflect if any of its scans are loaded.

            Implement "View Point Cloud" (4.3) from context menu: Triggers loading (if needed). For now, this can log to console or show a message; actual rendering is in Phase 3.

        Deliverables: Users can load/unload individual scans and all scans within a cluster, with clear visual feedback in the sidebar. The "View" action initiates the loading process.

    Sprint 2.2: Advanced Scan Import Options & SQLite Refinement

        Features:

            Implement remaining "Scan Import" file handling option (4.3): "Link to Source." Ensure UI clearly explains this option.

            Refine SQLite schema for Scans table to include all fields mentioned in 4.5 (original_path, point_count_estimate, bounding_box, etc.).

            Modify scan import to populate these additional fields in the Scans table.

            Ensure the sidebar data model reads hierarchy and scan details primarily from SQLite. The .json file provides overall project context.

        Deliverables: All three scan import options (copy, move, link) are functional. Comprehensive scan metadata is stored in the SQLite database. Sidebar reflects data from SQLite.

    Sprint 2.3: Cluster Locking & Context Menu Expansion

        Features:

            Implement "Lock/Unlock Cluster" (4.4): Update is_locked status in the Clusters table in SQLite via context menu.

            Add visual indicators (e.g., lock icon) in the sidebar for locked/unlocked cluster state.

            Expand right-click context menus for scans and clusters to include all implemented actions (Load/Unload, View, Lock/Unlock, Create Cluster/Sub-folder, Delete Item - with confirmation and appropriate data cleanup in SQLite).

        Deliverables: Users can lock and unlock clusters, with visual feedback. Sidebar context menus provide comprehensive access to all core scan and cluster management functions with data correctly managed in SQLite.

Phase 3: Data Persistence, Viewer Integration & UX Refinements (Estimated 6-8 weeks)

    Goal: Ensure robust data persistence, integrate a basic point cloud viewer, and refine the user experience.

    Sprint 3.1: Robust Data Persistence & Error Handling

        Features:

            Ensure all project settings (from .json), and the complete project structure including scan and cluster details and states (from SQLite) are reliably saved and loaded.

            Implement robust error handling for missing linked scan files (e.g., visual warning in sidebar, option to re-link or remove the reference from SQLite).

            Implement error handling for corrupted .json or .sqlite files (e.g., attempt recovery or notify user, offer to load from backup if available).

            Thorough testing of project open/save/close cycles with various configurations (different import types, nested clusters, locked states).

        Deliverables: Stable and reliable project data persistence using both .json for project metadata and SQLite for detailed structure and scan info. Application gracefully handles missing linked files and basic data corruption scenarios.

    Sprint 3.2: Basic Point Cloud Viewer Integration

        Features:

            Integrate a basic 3D point cloud rendering component into the main application window (e.g., using a simple OpenGL widget or a lightweight point cloud library).

            The "View Point Cloud" action (4.3) now triggers loading the point cloud data from the file path (stored in SQLite) and renders it in the viewer.

            Implement basic camera controls for the viewer (e.g., orbit, pan, zoom).

            Display single or multiple selected/loaded scans.

        Deliverables: Users can visually inspect the point cloud data of loaded scans or all scans within a loaded cluster in a 3D view.

    Sprint 3.3: UI/UX Refinements and Initial Feedback Implementation

        Features:

            Refine visual cues and icons in the sidebar for better clarity and aesthetics.

            Implement non-modal progress indicators for time-consuming operations (e.g., scan import, loading large scans into the viewer).

            Improve dialogs for project creation and scan import based on initial usability testing and feedback.

            Add tooltips for sidebar items and actions.

            Address any critical bugs or usability issues identified during internal testing.

        Deliverables: A more polished and user-friendly experience with better feedback mechanisms and refined UI elements.

    Sprint 3.4 (Optional, if time permits): Advanced Memory Management Teaser & Basic Registration Data Storage

        Features:

            Investigate and prototype basic Level of Detail (LOD) for point cloud display (e.g., random subsampling based on distance or a fixed percentage).

            Display basic memory usage statistics related to loaded point clouds.

            Extend SQLite schema to include tables for RegistrationStatus (scan_id/cluster_id, status, error_metric) and TransformationMatrices (scan_id/cluster_id, matrix_data). This is preparatory for future registration features.

        Deliverables: Proof-of-concept for more advanced point cloud handling. SQLite database is ready to store initial registration-related data.

8. Future Considerations (Post Phase 3)

    Detailed Registration Workflows:

        Implement dedicated UI modules and workflows for various registration methods:

            Top View Registration: Allow users to align scans based on a 2D top-down view, picking corresponding points or features. This would involve UI for selecting points on a 2D projection and an algorithm to compute the 2D transformation, potentially extensible to 3D.

            Target-based Registration: Support for importing target coordinates (e.g., from CSV), manual and semi-automatic detection of various target types (spheres, checkerboards, coded targets) in scans. UI for managing targets, their correspondences across scans, and calculating registration based on these. This includes storing target data in SQLite.

            Cloud-to-Cloud (C2C) Registration: Implement Iterative Closest Point (ICP) or its variants (e.g., Normal Distributions Transform - NDT). Provide user controls for C2C parameters (e.g., max correspondence distance, max iterations, convergence criteria, subsampling strategy).

            Hybrid Registration: Develop workflows that allow users to combine different registration methods sequentially (e.g., coarse alignment with Top View, followed by fine C2C, refined with targets).

    Visualization of Registration Quality and Errors:

        Display visual feedback on registration quality directly in the 3D viewer. This could include color-coding points based on overlap confidence or registration error, showing tension lines or error vectors for corresponding targets, or heatmaps of deviation.

        Generate comprehensive, exportable registration reports (e.g., PDF, HTML) including overall error metrics (e.g., mean error, RMSE, standard deviation, overlap percentage), target residuals, transformation parameters, and visualizations of error distribution.

        Allow users to interactively inspect individual scan-pair alignments and their associated error metrics, potentially with split views or difference clouds.

    Advanced Memory Management and Dynamic Point Cloud Density Controls:

        Implement sophisticated Level of Detail (LOD) / Level of Accuracy (LOA) mechanisms for rendering massive point clouds efficiently. This could involve octree-based streaming, continuous LOD algorithms, or other progressive loading techniques.

        Allow users to dynamically adjust point cloud density in the viewer (e.g., via a slider or preset quality levels) to balance performance and visual detail, adapting to the current task (e.g., quick overview vs. detailed inspection).

        Implement out-of-core rendering techniques to handle datasets that are significantly larger than available system RAM, ensuring smooth interaction.

        Provide users with clear, real-time feedback on memory usage (RAM and VRAM) and rendering performance (FPS), potentially with automatic adjustments to density or LOD based on detected hardware capabilities or performance bottlenecks.

    Integration with Cloud Storage and Collaboration:

        Enable projects (including the .json, .sqlite, and potentially scan data if not linked) to be saved to and loaded from common cloud storage providers (e.g., AWS S3, Azure Blob, Google Cloud Storage, or a proprietary cloud solution).

        Implement basic collaborative features such as project sharing with different permission levels (read-only, comment, edit).

        Introduce version control for project states, allowing users to revert to previous versions of the project structure or registration.

        Consider mechanisms for synchronizing changes when multiple users might be working on different aspects of the same project, potentially with conflict resolution strategies.

    Storing and Utilizing Transformation Matrices and Registration Status in SQLite (as per 4.5):

        Fully implement the storage and retrieval of 4x4 transformation matrices for each registered scan and cluster in the SQLite database. Ensure these matrices correctly represent the position and orientation relative to a well-defined project origin or parent entity.

        Persistently store the registration status (e.g., 'unregistered', 'processing', 'registered-manual', 'registered-auto-verified', 'failed', 'requires-review') and associated quantitative error metrics for each scan and cluster. This data should be clearly and accurately reflected in the sidebar and any registration reports.

    Storing and Managing Control Points, Targets, and Constraints in SQLite (as per 4.5):

        Develop a comprehensive schema and UI for managing user-defined or imported control points (ID, X, Y, Z, name/description, accuracy).

        Implement functionality to store information about detected or manually identified registration targets (ID, type, size, coordinates in individual scan spaces, corresponding control point ID if surveyed).

        Allow users to define, store, and manage registration constraints (e.g., point-to-point, plane-to-plane correspondences between scans, fixed scan constraints) within the SQLite database. These constraints would then be used by the registration algorithms.

    Performance Optimization for Large Datasets:

        Continuously profile and optimize all aspects of data loading, data processing (especially for registration algorithms), and rendering pipelines to ensure scalability with very large point clouds.

        Investigate and implement spatial indexing techniques (e.g., k-d trees, octrees, R-trees) within the SQLite database (if feasible for the queries needed) or primarily in-memory for faster querying, neighborhood searches, and collision detection during registration.

    Automation Features:

        Explore options for suggesting initial scan alignments or potential registration pairs based on available metadata (e.g., GPS coordinates, IMU data from scans, timestamps, spatial proximity of scan bounding boxes).

        Investigate possibilities for semi-automated or fully automated registration workflows, where the system attempts to register scans/clusters with minimal user intervention, flagging areas requiring manual review.

    Expanded Export Options:

        Allow export of fully registered and merged point clouds (or selected subsets) in various industry-standard formats, ensuring transformation matrices are correctly applied.

        Provide options to export detailed registration reports, target lists with residuals, and control point coordinates for use in other software or for documentation.

    User-Configurable Settings:

        Expose relevant parameters for registration algorithms (e.g., ICP convergence, target search tolerances) and visualization settings (e.g., point size, colorization schemes) to advanced users through a dedicated and well-documented settings panel.

    Undo/Redo Functionality:

        Implement a robust undo/redo mechanism for critical operations such as scan organization in the sidebar, cluster modifications, and eventually for individual steps within the registration process.

This PRD provides a foundational set of requirements. Further details will be elaborated upon during the design and development phases for each specific feature.

