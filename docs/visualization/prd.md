Product Requirements Document: Interactive 3D Point Cloud 'Street View'
I. Introduction

This Product Requirements Document (PRD) outlines the requirements for developing an interactive "Street View"-like feature within our existing Qt6 point cloud application. The goal is to provide users with an immersive, fixed-point panoramic visualization of loaded point cloud data, enabling detailed exploration, feature identification, and precise 3D measurements from any user-defined location on the scan map. This feature aims to significantly enhance the user's ability to analyze and interact with complex 3D scan data, mirroring the intuitive experience of widely adopted geospatial viewing tools.
II. Goals

The primary goals of this feature are:

    Immersive Exploration: To enable users to virtually "drop a pin" anywhere on a loaded scan map and experience a 360-degree panoramic view from that precise location.

    Enhanced Interaction: To allow users to interactively pan and tilt within this spherical view, select features (e.g., target boards, spheres), and perform accurate 3D measurements.

    Contextual Awareness: To provide a dual-panel interface, similar to Faro Scene, that maintains an overview of the global scan map while simultaneously displaying the detailed spherical view.

    Performance & Scalability: To ensure smooth, real-time interactivity and efficient processing, even with large point cloud datasets (e.g., up to 20 million points).

III. User Stories

The following user stories describe the core interactions and desired outcomes from the user's perspective:

    As a user, I want to drop a pin anywhere on the 2D representation of the scan map so that I can define a virtual camera viewpoint for detailed inspection.

    As a user, I want to see a real-time 3D spherical panoramic view generated from the pinned location so that I can immerse myself in the environment and understand the surroundings as if I were standing there.

    As a user, I want to pan and tilt fluidly within the spherical view so that I can explore the entire 360-degree environment from the fixed pinned point.

    As a user, I want to be able to select features (such as target boards, spheres, or other distinct objects) directly within the spherical view so that I can interact with specific elements of the scan.

    As a user, I want to be able to take precise 3D measurements (e.g., distance between two selected points, angle between three selected points) within the spherical view so that I can analyze the spatial relationships of features.

    As a user, I want the application interface to feature a dual-panel UI (one for the overall scan map, one for the spherical view) similar to Faro Scene's registration view, so that I can maintain contextual awareness of my location within the larger point cloud while exploring details.

IV. Functional Requirements
FR1: Virtual Camera Placement

    FR1.1: Pin Dropping: The application shall allow the user to click on the main 2D/3D scan map view to designate a point of interest (X, Y coordinates).

    FR1.2: Local Floor Detection: The system shall automatically determine the local ground plane (Z-coordinate) at the specified (X, Y) location using a local neighborhood search and statistical analysis (e.g., lowest Z-value or average of lowest Z-values within a local cluster).

    FR1.3: Camera Offset: The virtual camera shall be positioned at the selected (X, Y) coordinates with a Z-coordinate offset of +1.6 meters from the detected local floor (simulating eye-level).

FR2: Spherical View Generation

    FR2.1: Ray Casting: The system shall generate a 360-degree spherical panoramic image by casting rays from the virtual camera's origin into the surrounding point cloud data. For each ray, the closest intersected point's color and depth shall be used to determine the corresponding pixel in the panoramic image.

    FR2.2: Sparsity Handling: The system shall employ techniques (e.g., surface splatting, nearest-neighbor interpolation, or similar hole-filling algorithms) to mitigate artifacts and generate a dense, continuous spherical image from sparse point cloud data.

    FR2.3: Panoramic Rendering: The generated panoramic image (either as a cubemap or equirectangular projection) shall be applied as a texture onto a large sphere or cuboid mesh centered around the virtual camera, creating the immersive "inside-out" view.

FR3: Spherical View Interaction

    FR3.1: Panning and Tilting: Users shall be able to interactively pan (rotate horizontally) and tilt (rotate vertically) the spherical view using mouse click-and-drag gestures.

    FR3.2: Fixed Position: The virtual camera's 3D position shall remain fixed at the pinned location during all panning and tilting interactions.

FR4: Feature Picking

    FR4.1: Point Selection: Users shall be able to click on visible points or features (e.g., target boards, spheres, distinct objects) within the spherical view to select them.

    FR4.2: Coordinate Retrieval: Upon selection, the system shall retrieve and internally store the precise 3D world coordinates of the picked point.

    FR4.3: Visual Feedback: Selected features shall be highlighted with clear visual cues (e.g., color change, temporary bounding box, or custom highlight effect).

FR5: Measurement Tools

    FR5.1: Distance Measurement: The system shall calculate and display the Euclidean distance between any two user-selected points in the 3D spherical view.

    FR5.2: Angle Measurement: The system shall calculate and display the angle formed by any three user-selected points, with the middle point serving as the vertex.

    FR5.3: Visual Annotations: Measurements (lines and text) shall be dynamically drawn as temporary 3D annotations within the spherical view. These annotations shall be billboarded to remain readable regardless of camera orientation.

FR6: User Interface

    FR6.1: Dual-Panel Layout: The application shall implement a dual-panel user interface, with one panel displaying the overall scan map (likely the existing PointCloudViewerWidget) and the other panel displaying the interactive spherical "street view."

    FR6.2: Resizable Panels: The dual panels shall be resizable by the user (e.g., using a QSplitter).

    FR6.3: Contextual Indicator: The main scan map view shall display a visual indicator (e.g., a frustum or cone) representing the virtual camera's current position and field of view within the spherical view.

V. Non-Functional Requirements
NFR1: Performance

    NFR1.1: Real-time Interactivity: The spherical view shall maintain a minimum average frame rate of 30 frames per second (FPS) during interactive panning and tilting for point cloud datasets up to 20 million points.

    NFR1.2: Efficient Data Processing: Point cloud loading and the generation of the spherical view shall be optimized to minimize latency and ensure a responsive user experience.

    NFR1.3: GPU Utilization: The rendering pipeline shall primarily leverage GPU processing to minimize CPU bottlenecks and optimize data transfer between CPU and GPU.

NFR2: Usability

    NFR2.1: Intuitive Interaction: The "pin-drop" and panoramic navigation should feel natural and familiar to users accustomed to similar geospatial viewing applications.

    NFR2.2: Clear Feedback: All interactive elements (pin-drop, picking, measurements) shall provide immediate and unambiguous visual feedback to the user.

NFR3: Integration

    NFR3.1: Existing Application Compatibility: The new feature shall seamlessly integrate with the existing Qt6 application framework, including PointCloudViewerWidget, LasParser, and E57Parser, utilizing their current data structures and parsing capabilities.

    NFR3.2: Data Format Support: The feature must support point cloud data loaded from existing E57 and LAS file formats.

NFR4: Robustness

    NFR4.1: Sparse Data Handling: The spherical view generation shall gracefully handle sparse areas within the point cloud, minimizing visual "holes" or artifacts.

    NFR4.2: Error Handling: The system shall include robust error handling for file I/O, point cloud parsing, 3D rendering, and user interactions, providing informative messages to the user.

VI. High-Level Technical Approach

The implementation will leverage Qt6's modern 3D capabilities, specifically Qt3D, for robust scene management and rendering.

    Core Framework: Qt3DCore, Qt3DRender, and Qt3DExtras will form the foundation for 3D visualization.

    User Interface: A QSplitter will house two Qt3DWindow instances (wrapped by QWidget::createWindowContainer) to create the dual-panel layout. One will display the main point cloud map (potentially an enhanced PointCloudViewerWidget), and the other will render the spherical view.

    Point Cloud Management:

        Spatial Data Structures: Efficient spatial partitioning structures such as Octrees or K-D trees will be implemented to accelerate ray casting for panoramic generation, nearest neighbor searches for local floor detection, and precise point picking. This is crucial for performance with large datasets.

        Pre-processing: Consideration will be given to integrating or leveraging capabilities from the Point Cloud Library (PCL) for initial filtering, downsampling (e.g., VoxelGridFilter which already exists in the codebase), and noise reduction of large datasets before rendering.

    Spherical View Generation:

        Ray Casting Pipeline: A custom rendering pipeline will be developed to perform ray casting from the virtual camera's position into the point cloud (accelerated by spatial data structures).

        Texture Generation: The results of ray casting will be used to generate either a cubemap (six 2D textures) or a single equirectangular panoramic image.

        Rendering: This generated texture will then be applied to a large QSphereMesh or QCuboidMesh (acting as a skybox) centered around the virtual camera.

    Interaction:

        Custom Camera Controller: A custom QAbstractCameraController or direct manipulation of QCamera properties will be implemented to allow fixed-point rotation (panning and tilting) of the spherical view.

        Picking: Qt3DRender::QObjectPicker will be used for feature selection. For sparse point clouds, this will be augmented by nearest-neighbor searches to accurately identify the intended 3D point.

        Measurements: Measurement lines will be rendered using custom QGeometry and QGeometryRenderer with GL_LINES primitive type. 3D text annotations will be displayed using Qt3DExtras::QText2DEntity or custom billboarded shaders.

    Performance Optimization:

        Frustum Culling: Qt3DRender::QFrustumCulling will be used to cull non-visible point cloud segments.

        Level of Detail (LOD): Adaptive rendering strategies using QLevelOfDetail or hierarchical Octree-based LOD will be explored to manage rendering complexity based on distance.

        Progressive Rendering: Techniques to progressively refine the spherical view over multiple frames will be considered to maintain interactivity during initial rendering or complex updates.

        GPU Shaders (GLSL) & VBOs: Point cloud data will be stored in Vertex Buffer Objects (VBOs) on the GPU, and custom GLSL shaders will be used for efficient and visually enhanced point rendering (e.g., controlling point size, applying Eye-Dome Lighting).

        CUDA-OpenGL Interop: For extremely large datasets and complex real-time processing, the potential for CUDA-OpenGL interoperation for GPU-accelerated point cloud processing will be investigated.

VII. Project Phases and Sprints

This section outlines the proposed development phases and their breakdown into two-week sprints, detailing the key goals and deliverables for each.
Phase 1: Foundation & Core Spherical View (MVP)

Goal: Establish the fundamental Qt3D environment, implement the dual-panel layout, enable accurate virtual camera placement, and achieve a rudimentary spherical view rendering with basic interaction. This phase aims to deliver a Minimum Viable Product (MVP) of the "Street View" feature.

    Sprint 1: Environment Setup & Dual-Panel UI

        Goal: Set up the Qt3D development environment and create the basic dual-panel user interface.

        Key Deliverables:

            Qt3D project configured and running.

            QSplitter integrated into MainWindow layout.

            Two Qt3DWindow instances (wrapped by QWidget::createWindowContainer) embedded in the QSplitter.

            Existing PointCloudViewerWidget (or its equivalent) integrated into one panel for the main map view.

            Basic 3D scene (e.g., a simple cube or coordinate axes) rendered in both panels to confirm setup.

        Technical Tasks:

            Update .pro file (or CMakeLists.txt) to include Qt3D modules.

            Refactor MainWindow to use QSplitter and QWidget::createWindowContainer.

            Create a new Qt3DWindow-based class for the spherical view panel.

            Ensure both panels can display separate 3D content.

    Sprint 2: Virtual Camera Placement & Local Floor Detection

        Goal: Enable user pin-dropping on the main map and accurately calculate the virtual camera's 3D position.

        Key Deliverables:

            Mouse event handling in the main map view to capture 2D click coordinates.

            Implementation of a basic spatial data structure (e.g., a simple K-D tree or grid-based search) for the loaded point cloud to perform local neighborhood queries.

            Algorithm for local floor detection (lowest Z-value or statistical average) at the pinned (X,Y) location.

            Virtual camera's 3D position calculated (X,Y from pin, Z = detected floor + 1.6m offset).

            Qt3DRender::QCamera instance positioned correctly for the spherical view.

        Technical Tasks:

            Add mouse event listener to the main map viewer to get click coordinates.

            Develop or integrate a basic K-D tree/Octree for point cloud spatial indexing.

            Implement findLocalFloorZ(QVector2D pinnedXY) method.

            Set QCamera::setPosition() for the spherical view camera.

    Sprint 3: Basic Spherical View Rendering (Ray Casting & Texture)

        Goal: Generate a rudimentary spherical view by projecting point cloud data onto a texture and applying it to a sphere.

        Key Deliverables:

            Initial ray casting mechanism from the virtual camera position into the point cloud (leveraging spatial data structure).

            Generation of a low-resolution cubemap or equirectangular texture based on ray intersections (e.g., simply mapping point colors to pixels).

            A large QSphereMesh or QCuboidMesh rendered around the virtual camera in the spherical view panel.

            The generated texture applied to the sphere/cuboid.

        Technical Tasks:

            Develop a RayCaster class that queries the spatial data structure.

            Implement generatePanoramicTexture(QCamera camera, QSize resolution) function.

            Create QSphereMesh entity in the spherical view scene.

            Apply QTextureImage (or QTexture2D) to the sphere's material.

    Sprint 4: Spherical View Interaction & Initial Refinement

        Goal: Enable smooth panning and tilting within the spherical view and improve visual quality.

        Key Deliverables:

            Custom QAbstractCameraController (or direct QCamera manipulation) for fixed-point rotation (panning and tilting) of the spherical view.

            Mouse click-and-drag gestures mapped to camera orientation changes.

            Basic sparsity handling/hole-filling (e.g., nearest-neighbor interpolation or simple point splatting) to reduce visual gaps.

        Technical Tasks:

            Implement FixedPointCameraController class.

            Connect mouse events to controller logic to update QCamera::setViewCenter() and QCamera::setUpVector().

            Integrate basic interpolation/splatting into the panoramic texture generation.

Phase 2: Advanced Interaction & Performance

Goal: Enhance the "Street View" with interactive picking, precise measurement tools, and significant performance optimizations for larger point cloud datasets.

    Sprint 5: Feature Picking & Distance Measurement

        Goal: Allow users to select features within the spherical view and measure distances.

        Key Deliverables:

            Qt3DRender::QObjectPicker integrated into the spherical view scene.

            Ability to click on points/features in the spherical view to retrieve their 3D world coordinates.

            Visual highlighting of selected points (e.g., color change, marker).

            Implementation of Euclidean distance calculation between two selected points.

            Dynamic rendering of a 3D line between the two selected points.

        Technical Tasks:

            Add QObjectPicker component to the spherical view's root entity or relevant sub-entities.

            Implement slot to handle QObjectPicker::clicked() signal.

            Store selected points in a QList<QVector3D>.

            Create custom QGeometry and QGeometryRenderer for drawing lines.

    Sprint 6: Angle Measurement & Text Annotations

        Goal: Add angle measurement capability and display measurements as 3D text.

        Key Deliverables:

            Implementation of angle calculation between three selected points.

            Qt3DExtras::QText2DEntity (or custom billboarded text rendering) for displaying measurement values (distance, angle).

            Dynamic placement and updating of 3D text annotations.

        Technical Tasks:

            Implement calculateAngle(P1, P2, P3) function.

            Create QText2DEntity instances for annotations.

            Develop billboard shader/logic for text to always face the camera.

    Sprint 7: Performance Optimization I (Spatial Structures & Frustum Culling)

        Goal: Improve rendering performance for large point clouds using advanced culling and spatial structures.

        Key Deliverables:

            Optimized and robust Octree or K-D tree implementation for the entire point cloud.

            Integration of Qt3DRender::QFrustumCulling to efficiently cull non-visible point cloud segments in both main and spherical views.

            Performance benchmarks demonstrating improved frame rates.

        Technical Tasks:

            Refine spatial data structure implementation for optimal query performance.

            Apply QFrustumCulling to point cloud entities.

            Measure and log FPS before and after optimizations.

    Sprint 8: Performance Optimization II (LOD & Progressive Rendering)

        Goal: Further enhance interactivity and visual quality with Level of Detail and progressive rendering.

        Key Deliverables:

            Implementation of a basic Level of Detail (LOD) strategy (e.g., using QLevelOfDetail or Octree-based LOD) for the main map view.

            Exploration and initial implementation of progressive rendering for the spherical view to maintain responsiveness during complex updates.

        Technical Tasks:

            Define LOD levels for point cloud rendering.

            Integrate QLevelOfDetail component or custom LOD switching logic.

            Implement a multi-pass rendering approach for the spherical view texture generation.

Phase 3: Refinement & Future Enhancements

Goal: Polish the user experience, enhance robustness, and lay the groundwork for future advanced features.

    Sprint 9: UI/UX Refinement & Contextual Indicator

        Goal: Improve the overall user interface and add a visual contextual indicator.

        Key Deliverables:

            Refined QSplitter behavior and visual styling.

            Implementation of a visual frustum or cone indicator on the main map view, showing the spherical camera's current position and field of view.

            General UI/UX improvements based on internal testing feedback.

        Technical Tasks:

            Draw frustum/cone QGeometry in the main map view.

            Update frustum/cone transform based on spherical view camera orientation.

    Sprint 10: Robustness & Comprehensive Error Handling

        Goal: Enhance the stability and reliability of the new feature.

        Key Deliverables:

            Comprehensive error handling for all new components (e.g., invalid pin drops, sparse data leading to empty views, picking failures).

            Thorough unit and integration testing of the new features.

            Improved logging for debugging and performance monitoring.

        Technical Tasks:

            Add try-catch blocks and validation checks for all critical operations.

            Implement custom error messages for user feedback.

    Sprint 11+: Future Considerations & Iteration

        Goal: Begin implementing features from the "Future Considerations" section, based on project priorities and user feedback.

        Key Deliverables: (Examples)

            Integration of advanced point cloud segmentation algorithms.

            Support for rendering additional point cloud attributes (intensity, normals).

            User-defined pinned viewpoint saving/loading.

        Technical Tasks: (Examples)

            Research and integrate PCL segmentation modules.

            Extend point data structures and shaders to handle additional attributes.

            Implement serialization/deserialization for viewpoints.

VIII. Out of Scope for Initial Release

The following functionalities are explicitly out of scope for this initial phase of development:

    Advanced point cloud compression/decompression beyond the capabilities of the existing E57 and LAS parsers.

    Real-time point cloud acquisition or streaming directly from sensors.

    Complex surface reconstruction algorithms (e.g., Poisson reconstruction) for generating solid meshes from point clouds, beyond basic hole-filling for panoramic views.

    Multi-user collaboration features within the "Street View" environment.

    Functionality to save or export generated spherical panoramic images or measurement results.

IX. Future Considerations

Potential enhancements for future iterations include:

    Integration of advanced point cloud segmentation algorithms for automatic detection and labeling of features (e.g., poles, trees, building facades).

    Support for rendering additional point cloud attributes (e.g., intensity, normals, timestamps) within the spherical view for richer visualization.

    Integration with Virtual Reality (VR) or Augmented Reality (AR) devices for an even more immersive exploration experience.

    Ability to save and load user-defined pinned viewpoints and measurement annotations.

    Implementation of more sophisticated lighting models and rendering techniques for improved visual fidelity in the spherical view.

X. Success Metrics

The success of this feature will be measured by the following criteria:

    Performance: The spherical view maintains an average frame rate of at least 30 FPS on target hardware during interactive panning for point clouds up to 20 million points.

    Accuracy: Distance measurements are accurate to within ±1 cm, and angle measurements are accurate to within ±1 degree.

    Usability: User feedback (e.g., through internal testing or user surveys) indicates that the "pin-drop" and panoramic navigation are intuitive and easy to use, and the measurement tools are effective.

    Stability: The application demonstrates high stability, with minimal crashes or unexpected behavior during prolonged use of the new feature.

    Integration: The feature is seamlessly integrated into the existing application, with no negative impact on existing functionalities or performance.