Sprint 1.4 Backlog: Viewer Navigation & UCS Enhancements

This backlog details the tasks for Sprint 1.4, which aims to significantly improve the navigation and spatial awareness within the point cloud viewer. The primary goals are to provide users with quick access to standard orthogonal views and to introduce a visual User Coordinate System (UCS) indicator that updates with camera movement.
1. Introduction

Sprint 1.4 builds upon the viewer enhancements from Sprint 1.3 by adding more intuitive navigation controls and a crucial visual aid for understanding the 3D space. Implementing fixed "top", "left", "right", and "bottom" views will simplify common viewing tasks, while a dynamic UCS indicator will provide continuous spatial context, making it easier for users to orient themselves within the point cloud.
2. User Stories

    User Story 1: User can switch to predefined orthogonal views (Top, Left, Right, Bottom)

        Description: As a user, I want to be able to quickly switch to standard orthogonal views (Top, Left, Right, Bottom) by clicking dedicated buttons or selecting from a dropdown menu, so that I can easily inspect the point cloud from common perspectives.

        Actions to Undertake:

            Frontend (Qt/UI - MainWindow):

                Add new QPushButton widgets for "Top View", "Left View", "Right View", "Bottom View" to MainWindow's button layout.

                Add corresponding QAction items to a new "View" menu in the QMenuBar.

                Connect these buttons and menu actions to new slots in MainWindow (e.g., onTopViewClicked(), onLeftViewClicked(), etc.).

            Frontend (Qt/OpenGL - PointCloudViewerWidget):

                Add public slots to PointCloudViewerWidget (e.g., setTopView(), setLeftView(), setRightView(), setBottomView()).

                Implement these slots to modify m_cameraPosition, m_cameraTarget, m_cameraUp, m_cameraDistance, m_cameraYaw, and m_cameraPitch to achieve the desired orthogonal view.

                    Top View: Camera directly above m_cameraTarget, looking down. m_cameraUp should be aligned with a consistent "forward" direction (e.g., Y-axis).

                    Left View: Camera to the left of m_cameraTarget, looking right.

                    Right View: Camera to the right of m_cameraTarget, looking left.

                    Bottom View: Camera directly below m_cameraTarget, looking up.

                Ensure these methods call updateCamera() to refresh the view.

            MainWindow to PointCloudViewerWidget Connection:

                In MainWindow's new slots (e.g., onTopViewClicked()), call the corresponding PointCloudViewerWidget slot (e.g., m_viewer->setTopView()).

        References between Files:

            src/mainwindow.h: New QPushButton members, new QAction members, new slots for view changes.

            src/mainwindow.cpp: UI setup for buttons/menu, implementation of new slots to call PointCloudViewerWidget.

            src/pointcloudviewerwidget.h: New public slots for setting views.

            src/pointcloudviewerwidget.cpp: Implementation of new view setting slots, modification of updateCamera() to handle new m_cameraPosition/m_cameraTarget/m_cameraUp values.

        Acceptance Criteria:

            Dedicated buttons for "Top View", "Left View", "Right View", "Bottom View" are present in the UI.

            These view options are also available in a "View" dropdown menu in the menu bar.

            Clicking any of these buttons/menu items instantly reorients the camera to the corresponding orthogonal view.

            The point cloud remains centered and fully visible in the new view.

            Subsequent mouse interactions (orbit, pan, zoom) work correctly from the new orthogonal view.

        Testing Plan:

            Test Case 1.4.1.1: Switch to Top View.

                Test Data: Any loaded point cloud.

                Expected Result: Camera moves to directly above the point cloud, looking down. Z-axis points upwards.

                Testing Tool: Manual UI observation.

            Test Case 1.4.1.2: Switch to Left View.

                Test Data: Any loaded point cloud.

                Expected Result: Camera moves to the left of the point cloud, looking right.

                Testing Tool: Manual UI observation.

            Test Case 1.4.1.3: Switch to Right View.

                Test Data: Any loaded point cloud.

                Expected Result: Camera moves to the right of the point cloud, looking left.

                Testing Tool: Manual UI observation.

            Test Case 1.4.1.4: Switch to Bottom View.

                Test Data: Any loaded point cloud.

                Expected Result: Camera moves to directly below the point cloud, looking up. Z-axis points downwards.

                Testing Tool: Manual UI observation.

            Test Case 1.4.1.5: Test all views from dropdown menu.

                Test Data: Any loaded point cloud.

                Expected Result: Behavior is identical to button clicks.

                Testing Tool: Manual UI observation.

            Test Case 1.4.1.6: Interact after view change.

                Test Data: After switching to any orthogonal view, try orbiting, panning, and zooming.

                Expected Result: Interactions work as expected from the new orientation.

                Testing Tool: Manual UI interaction.

    User Story 2: User can see a 3D UCS indicator in the viewer

        Description: As a user, I want to see a small, dynamic 3D arrow representation of the X, Y, and Z axes in the top-right corner of the viewer that updates with camera movement, so that I can always understand the current orientation of the point cloud relative to a global coordinate system.

        Actions to Undertake:

            Frontend (Qt/OpenGL - PointCloudViewerWidget):

                Shader for UCS: Create a separate, simple shader program within PointCloudViewerWidget specifically for rendering the UCS arrows. This shader will likely use basic vertex and fragment shaders to draw lines.

                UCS Geometry: Define vertex data for three lines representing the X, Y, and Z axes (e.g., from (0,0,0) to (1,0,0) for X, etc.). Assign distinct colors (e.g., Red for X, Green for Y, Blue for Z).

                Rendering Logic: In PointCloudViewerWidget::paintGL():

                    After rendering the point cloud, bind the UCS shader program.

                    Calculate a separate model-view-projection matrix for the UCS. This matrix should position the UCS in the top-right corner of the screen (using normalized device coordinates or similar fixed-screen-position logic) and apply only the camera's rotation (not translation) to the UCS, so it rotates with the view but stays fixed on screen.

                    Draw the UCS lines using glDrawArrays(GL_LINES, ...).

                Positioning: Determine the appropriate screen-space coordinates for the UCS widget (e.g., using glViewport and glScissor or by adjusting the UCS's projection matrix) to place it in the top-right corner.

                Size: Define a fixed size for the UCS indicator (e.g., 50x50 pixels).

        References between Files:

            src/pointcloudviewerwidget.h: New QOpenGLShaderProgram for UCS, VAO/VBO for UCS geometry, members for UCS uniform locations.

            src/pointcloudviewerwidget.cpp: New setupUCSShaders(), setupUCSBuffers(), and drawUCS() methods. Modifications to paintGL() to call drawUCS(). Logic for calculating UCS MVP matrix.

        Acceptance Criteria:

            A small 3D arrow indicator (X, Y, Z axes) is visible in the top-right corner of the PointCloudViewerWidget.

            The X, Y, and Z axes are clearly distinguishable (e.g., Red for X, Green for Y, Blue for Z).

            When the user orbits the camera, the UCS indicator rotates in sync with the point cloud's apparent rotation, always indicating the global orientation.

            The UCS indicator remains fixed in its screen position (top-right corner) regardless of camera pan or zoom.

            The UCS indicator does not obstruct the main point cloud view significantly.

        Testing Plan:

            Test Case 1.4.2.1: UCS indicator presence and initial orientation.

                Test Data: Any loaded point cloud.

                Expected Result: UCS indicator is visible in top-right. X, Y, Z axes are colored correctly and show default orientation.

                Testing Tool: Manual UI observation.

            Test Case 1.4.2.2: UCS indicator rotation with camera orbit.

                Test Data: Loaded point cloud. Orbit camera around.

                Expected Result: UCS indicator rotates in sync with the point cloud's rotation.

                Testing Tool: Manual UI observation.

            Test Case 1.4.2.3: UCS indicator fixed position with camera pan/zoom.

                Test Data: Loaded point cloud. Pan and zoom the camera.

                Expected Result: UCS indicator remains in the top-right corner, its size and position on screen do not change relative to the window.

                Testing Tool: Manual UI observation.

            Test Case 1.4.2.4: Orthogonal view changes and UCS.

                Test Data: Switch to Top, Left, Right, Bottom views.

                Expected Result: UCS indicator correctly reflects the new global orientation in each view.

                Testing Tool: Manual UI observation.

3. Actions to Undertake (Consolidated)

    Frontend (Qt/UI - MainWindow):

        Add "View" menu to MainWindow's menu bar.

        Add QActions for "Top View", "Left View", "Right View", "Bottom View" to the "View" menu.

        Add QPushButtons for "Top View", "Left View", "Right View", "Bottom View" to MainWindow's m_buttonLayout.

        Implement new private slots in MainWindow.cpp (e.g., MainWindow::onTopViewClicked(), etc.) that call corresponding methods on m_viewer.

    Frontend (Qt/OpenGL - PointCloudViewerWidget):

        Fixed Views:

            Add public slots setTopView(), setLeftView(), setRightView(), setBottomView() to PointCloudViewerWidget.h.

            Implement these slots in PointCloudViewerWidget.cpp to adjust m_cameraPosition, m_cameraTarget, m_cameraUp, m_cameraDistance, m_cameraYaw, m_cameraPitch for the specific orthogonal view, then call updateCamera().

            Ensure updateCamera() correctly re-calculates m_viewMatrix based on these parameters.

        UCS Indicator:

            Add new member variables to PointCloudViewerWidget.h for UCS rendering: QOpenGLShaderProgram *m_ucsShaderProgram;, QOpenGLBuffer m_ucsVertexBuffer;, QOpenGLVertexArrayObject m_ucsVertexArrayObject;, and uniform locations.

            Create setupUCSShaders() method in PointCloudViewerWidget.cpp to compile and link a simple vertex/fragment shader for drawing lines (UCS axes).

            Create setupUCSBuffers() method to define the vertex data (positions and colors) for the X, Y, Z axes and upload to m_ucsVertexBuffer.

            Create drawUCS() method in PointCloudViewerWidget.cpp:

                Calculate the UCS MVP matrix. This will involve:

                    Getting the camera's rotation component from m_viewMatrix.

                    Creating a projection matrix for a fixed screen-space position (e.g., orthographic projection mapped to a small corner of the viewport).

                    Creating a model matrix to scale and position the UCS within that screen-space corner.

                Bind m_ucsShaderProgram.

                Set UCS-specific uniforms (MVP matrix, colors).

                Bind m_ucsVertexArrayObject.

                Draw the lines (glDrawArrays(GL_LINES, ...)).

                Release.

            Call setupUCSShaders() and setupUCSBuffers() in initializeGL().

            Call drawUCS() in paintGL().

4. References between Files (Consolidated)

    src/mainwindow.h & src/mainwindow.cpp: UI elements (buttons, menu actions) and their corresponding slots. These slots will call methods in m_viewer.

    src/pointcloudviewerwidget.h & src/pointcloudviewerwidget.cpp:

        Public slots for setTopView(), etc., which modify internal camera parameters.

        Private methods setupUCSShaders(), setupUCSBuffers(), drawUCS() for rendering the UCS.

        initializeGL() and paintGL() will be modified to integrate the UCS rendering.

        updateCamera() will be updated to reflect changes from fixed view settings.

    OpenGL Shaders: New vertex and fragment shader code will be embedded within src/pointcloudviewerwidget.cpp for the UCS rendering.

5. List of Files being Created

    No new files are being created in this sprint. All changes will be modifications to existing files.

    Modified Files:

        File 1: src/mainwindow.h

            Purpose: Main window class definition.

            Contents: Add new QPushButton members, new QAction members, new slots for view changes.

            Relationships: Connects user input to viewer functionality.

        File 2: src/mainwindow.cpp

            Purpose: Main window implementation.

            Contents: UI setup for new buttons/menu, implementation of new slots (e.g., onTopViewClicked()) to call m_viewer methods.

            Relationships: Orchestrates UI and viewer interaction.

        File 3: src/pointcloudviewerwidget.h

            Purpose: Point cloud viewer widget definition.

            Contents: Add new public slots for setting orthogonal views. Add new private members for UCS shader, buffers, and uniform locations.

            Relationships: Provides API for view control, manages OpenGL resources for UCS.

        File 4: src/pointcloudviewerwidget.cpp

            Purpose: Point cloud viewer widget implementation.

            Contents: Implement new orthogonal view setting slots. Implement setupUCSShaders(), setupUCSBuffers(), drawUCS() methods. Modify initializeGL() to call UCS setup, and paintGL() to call drawUCS(). Refine camera logic in updateCamera().

            Relationships: Handles 3D rendering and camera manipulation.

6. Acceptance Criteria (Consolidated)

    Fixed Views:

        Dedicated buttons for "Top View", "Left View", "Right View", "Bottom View" are present in the UI.

        These view options are also available in a "View" dropdown menu in the menu bar.

        Clicking any of these buttons/menu items instantly reorients the camera to the corresponding orthogonal view (top, left, right, bottom).

        The point cloud remains centered and fully visible in the new view.

        Subsequent mouse interactions (orbit, pan, zoom) work correctly from the new orthogonal view.

    UCS Indicator:

        A small 3D arrow indicator (X, Y, Z axes) is visible in the top-right corner of the PointCloudViewerWidget.

        The X, Y, and Z axes are clearly distinguishable (e.g., Red for X, Green for Y, Blue for Z).

        When the user orbits the camera, the UCS indicator rotates in sync with the point cloud's apparent rotation, always indicating the global orientation.

        The UCS indicator remains fixed in its screen position (top-right corner) regardless of camera pan or zoom.

        The UCS indicator does not obstruct the main point cloud view significantly.

7. Testing Plan

    Test Case 1.4.1.1: Switch to Top View.

        Test Data: Any loaded point cloud.

        Expected Result: Camera moves to directly above the point cloud, looking down. Z-axis points upwards.

        Testing Tool: Manual UI observation.

    Test Case 1.4.1.2: Switch to Left View.

        Test Data: Any loaded point cloud.

        Expected Result: Camera moves to the left of the point cloud, looking right.

        Testing Tool: Manual UI observation.

    Test Case 1.4.1.3: Switch to Right View.

        Test Data: Any loaded point cloud.

        Expected Result: Camera moves to the right of the point cloud, looking left.

        Testing Tool: Manual UI observation.

    Test Case 1.4.1.4: Switch to Bottom View.

        Test Data: Any loaded point cloud.

        Expected Result: Camera moves to directly below the point cloud, looking up. Z-axis points downwards.

        Testing Tool: Manual UI observation.

    Test Case 1.4.1.5: Test all views from dropdown menu.

        Test Data: Any loaded point cloud.

        Expected Result: Behavior is identical to button clicks.

        Testing Tool: Manual UI observation.

    Test Case 1.4.1.6: Interact after view change.

        Test Data: After switching to any orthogonal view, try orbiting, panning, and zooming.

        Expected Result: Interactions work as expected from the new orientation.

        Testing Tool: Manual UI interaction.

    Test Case 1.4.2.1: UCS indicator presence and initial orientation.

        Test Data: Any loaded point cloud.

        Expected Result: UCS indicator is visible in top-right. X, Y, Z axes are colored correctly and show default orientation.

        Testing Tool: Manual UI observation.

    Test Case 1.4.2.2: UCS indicator rotation with camera orbit.

        Test Data: Loaded point cloud. Orbit camera around.

        Expected Result: UCS indicator rotates in sync with the point cloud's rotation.

        Testing Tool: Manual UI observation.

    Test Case 1.4.2.3: UCS indicator fixed position with camera pan/zoom.

        Test Data: Loaded point cloud. Pan and zoom the camera.

        Expected Result: UCS indicator remains in the top-right corner, its size and position on screen do not change relative to the window.

        Testing Tool: Manual UI observation.

    Test Case 1.4.2.4: Orthogonal view changes and UCS.

        Test Data: Switch to Top, Left, Right, Bottom views.

        Expected Result: UCS indicator correctly reflects the new global orientation in each view.

        Testing Tool: Manual UI observation.

8. Assumptions and Dependencies

    Sprint 1.3 Completion: The PointCloudViewerWidget and MainWindow are functional, with asynchronous loading and "zoom to extent" implemented.

    OpenGL Knowledge: Developers are familiar with OpenGL rendering pipelines, shaders, and matrix transformations for 3D graphics.

    Qt Integration: Familiarity with integrating OpenGL rendering within Qt widgets.

    Camera Model: The existing camera model (m_cameraPosition, m_cameraTarget, m_cameraUp, m_cameraDistance, m_cameraYaw, m_cameraPitch) is sufficiently flexible to implement fixed views.

    Performance: Rendering the UCS indicator will have a negligible impact on overall performance.

9. Non-Functional Requirements

    Usability:

        The fixed view buttons/menu items should be clearly labeled and easily discoverable.

        The UCS indicator should be small enough not to obscure the main view but large enough to be clearly visible and understandable.

    Performance:

        Switching between fixed views should be instantaneous.

        The rendering of the UCS indicator should not cause any noticeable performance degradation.

    Reliability:

        The camera logic for fixed views should be robust and not lead to unexpected camera orientations or crashes.

        The UCS rendering should be stable and not introduce visual artifacts.

    Maintainability:

        The code for fixed views and UCS should be modular and well-commented.

10. Conclusion

Sprint 1.4 will significantly enhance the user's ability to navigate and understand 3D point cloud data. By providing quick access to common orthogonal views and a persistent visual orientation cue through the UCS indicator, the application will become more intuitive and powerful for spatial analysis.