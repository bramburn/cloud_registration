Sprint 2.3 Backlog: Manual Point Cloud Registration and Interaction
1. Introduction

With the application's data loading, state management, and UI feedback systems now established, Sprint 2.3 marks a pivotal transition towards the core functionality of the MVP: point cloud registration. This sprint is dedicated to implementing the manual registration workflow, which empowers users to align scans by identifying corresponding points.

The primary objectives of this sprint are to:

    Enable precise 3D point selection within the PointCloudViewerWidget.

    Implement the mathematical logic to compute a rigid transformation from these user-defined point pairs.

    Enhance the UI to manage the registration process and provide clear controls for locking and deleting project items.

Successfully completing this sprint will deliver the first tangible registration capabilities to the user, providing a solid foundation upon which the automated ICP registration can be built in subsequent sprints.
2. User Stories
User Story 1: Implement 3D Point Picking for Correspondence Selection

    User Story [1]: As a user, I need to be able to enter a "point picking mode" and accurately select individual points on two different scans within the 3D viewer, so that I can identify correspondence pairs for manual registration.

    Description: This story focuses on the primary user interaction for manual registration. The PointCloudViewerWidget will be enhanced to support a specific mode where mouse clicks are translated into 3D point selections. The application must provide clear visual feedback, highlighting the selected points and maintaining a list of these correspondence pairs for the user.

    Actions to Undertake:

        Point Picking Mode: Implement a toggleable "Point Picking Mode" in the MainWindow. When active, the mouse cursor should change, and the viewer should listen for selection clicks instead of camera navigation.

        Ray-Casting Implementation: In PointCloudViewerWidget, implement a method that takes a 2D mouse coordinate and performs ray-casting into the 3D scene to find the nearest point in the currently active point cloud. This will require un-projecting the screen coordinates back into world space.

        Selection Logic: Maintain state to track which scan is the "fixed" scan and which is the "moving" scan. Alternate point picks between the two.

        Visual Feedback: When a point is selected, change its color or size in the viewer to provide immediate feedback. Use a different color for points selected on the fixed vs. the moving scan.

        Correspondence Pair Management: Store the selected pairs of 3D points in a data structure (e.g., QList<QPair<QVector3D, QVector3D>>).

        UI Panel: Create a simple UI panel or widget where the user can see the list of selected point pairs and has the option to remove a pair or clear all pairs.

    References between Files:

        pointcloudviewerwidget.h/.cpp: Will contain the core logic for ray-casting and point selection.

        mainwindow.h/.cpp: Will manage the "Point Picking Mode" toggle and host the new UI panel for displaying correspondence pairs.

    Acceptance Criteria:

        The user can activate and deactivate a "Point Picking Mode" via a UI button.

        In picking mode, clicking on a point cloud selects the nearest point to the mouse cursor.

        The selected point is visually highlighted in the 3D viewer.

        The application alternates between selecting a point on the fixed scan and a point on the moving scan.

        Each pair of selected points (one from each scan) is added to a visible list of correspondences.

        The user can remove a correspondence pair from the list, which also removes the visual highlighting in the viewer.

    Testing Plan:

        Manual Test: Load two scans, enter picking mode, and select several points on each. Verify that the points are highlighted correctly and that the correspondence list is populated. Test removing pairs.

        Unit Test: Create a unit test for the ray-casting logic. Given a known camera setup and a simple point cloud, assert that a click at a specific 2D coordinate correctly identifies the expected 3D point.

User Story 2: Compute and Apply Registration Transformation

    User Story [2]: As a user, once I have selected at least three pairs of corresponding points, I want to be able to click a "Register" button to compute and apply the alignment transformation, so that I can see the "moving" scan snap into place with the "fixed" scan.

    Description: This story covers the computational core of manual registration. It involves taking the user-selected correspondence pairs and using them to calculate the optimal rigid transformation matrix (rotation and translation). This matrix will then be applied to the "moving" scan, updating its position in the 3D viewer to reflect the new alignment.

    Actions to Undertake:

        RegistrationManager Class: Create a new class, RegistrationManager, in src/registrationmanager.h and .cpp to encapsulate the registration logic.

        SVD Algorithm: Inside RegistrationManager, implement a method to compute the transformation matrix. This method will use an algorithm like Singular Value Decomposition (SVD) to find the least-squares solution for the rigid transformation. Leverage the PCL library's TransformationEstimationSVD class for a robust implementation.

        UI Control: Add a "Register" button to the UI, which is enabled only when three or more correspondence pairs have been selected.

        Transformation Application: When the "Register" button is clicked, pass the list of point pairs to the RegistrationManager. The manager computes the 4x4 transformation matrix.

        Visual Update: The computed matrix must be applied to the "moving" scan. In PointCloudViewerWidget, implement a mechanism to store and apply a model matrix for each loaded point cloud. The registration matrix will be multiplied with the moving scan's model matrix.

        State Update: After registration, the computed transformation matrix must be saved to the project's database, associated with the moving scan.

    List of Files being Created:

        src/registrationmanager.h

        src/registrationmanager.cpp

    Acceptance Criteria:

        The "Register" button is disabled until at least three point pairs are selected.

        Clicking "Register" computes a transformation matrix and applies it to the moving scan.

        The moving scan's position and orientation in the 3D viewer are visibly updated to align with the fixed scan.

        The computed transformation matrix is persisted in the database.

        The alignment should be mathematically correct for the given point pairs.

    Testing Plan:

        Unit Test: In a new tests/test_registrationmanager.cpp, create a test case with two sets of known 3D points related by a known transformation. Pass these point pairs to the SVD algorithm and assert that the computed matrix is nearly identical to the known transformation matrix.

        Manual Test: Load two scans, select 3-4 clear corresponding points (e.g., corners of a room), and click "Register". Visually inspect the result to confirm that the scans are well-aligned.

User Story 3: Refine Cluster Locking and Deletion UI

    User Story [3]: As a user, I want the UI to prevent me from accidentally modifying or deleting a locked cluster, and I want to be clearly warned about the consequences of deleting items.

    Description: This story enhances the stability and usability of the project management features. It ensures that the "locked" state of a cluster is respected throughout the UI, preventing unintended modifications. It also improves the deletion workflow by providing a more informative confirmation dialog that clearly explains what will be deleted.

    Actions to Undertake:

        UI State Control: In SidebarWidget::contextMenuEvent, after determining the item's lock state, enable or disable the QActions for "Rename", "Delete", and "Create Sub-Cluster".

        Drag-and-Drop Validation: In SidebarWidget::dropEvent, add a check to prevent any items from being dropped onto a locked cluster.

        Informative Deletion Dialog: Enhance the ConfirmationDialog used for deletion. When deleting a cluster, the dialog should dynamically list the number of sub-clusters and scans that will also be affected or deleted.

        Physical File Deletion Option: For the recursive cluster deletion, ensure the ConfirmationDialog correctly presents the option to "Also delete physical scan files" only if the cluster contains scans that were copied or moved into the project.

    References between Files:

        sidebarwidget.cpp: Logic for enabling/disabling actions and validating drag-and-drop will be implemented here.

        confirmationdialog.h/.cpp: Will be modified to support more dynamic and detailed messaging.

        projectmanager.cpp: Will provide the data needed for the enhanced confirmation dialog.

    Acceptance Criteria:

        All modification actions (rename, delete, add sub-cluster) are disabled in the context menu for a locked cluster.

        Dragging and dropping scans or other clusters onto a locked cluster is not permitted.

        When a user attempts to delete a cluster, the confirmation dialog clearly states how many sub-items will be affected.

        The option to delete physical files is only shown when relevant (for non-linked scans).

    Testing Plan:

        Manual Test: Lock a cluster. Verify that all modification actions are disabled. Try to drag a scan into it and confirm the action is rejected. Attempt to delete the cluster and verify the confirmation dialog is accurate.

6. Assumptions and Dependencies

    The PCL library is successfully integrated via vcpkg and its registration components are available.

    The PointCloudViewerWidget can be modified to intercept mouse events for point picking without conflicting with camera controls.

    The SQLite database schema can be extended to store transformation matrices.

7. Non-Functional Requirements

    Performance: The point picking process, including ray-casting, must be real-time and not introduce any noticeable lag. The transformation calculation should be near-instantaneous for a small number of point pairs.

    Accuracy: The SVD-based transformation calculation must be numerically stable and precise.

    Usability: The manual registration workflow, from entering picking mode to seeing the final alignment, must be intuitive and require minimal steps.

8. Conclusion

Sprint 2.3 represents a major milestone by delivering the first interactive registration functionality. By the end of this sprint, the application will not only be a robust data viewer and manager but also a functional tool for manual scan alignment. This work paves the way for more advanced features like automated registration and quality analysis in future sprints.