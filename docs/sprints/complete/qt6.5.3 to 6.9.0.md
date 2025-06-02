Okay, here are the consolidated "Actions to Undertake" for Sprint 1.4, based on the provided backlog document.

## Actions to Undertake (Sprint 1.4)

### **User Story 1: User can switch to predefined orthogonal views (Top, Left, Right, Bottom)**

**Frontend (Qt/UI - MainWindow):**
*   Add new `QPushButton` widgets for "Top View", "Left View", "Right View", "Bottom View" to `MainWindow`'s button layout.
*   Add corresponding `QAction` items to a new "View" menu in the `QMenuBar`.
*   Connect these buttons and menu actions to new slots in `MainWindow` (e.g., `onTopViewClicked()`, `onLeftViewClicked()`, etc.).

**Frontend (Qt/OpenGL - PointCloudViewerWidget):**
*   Add public slots to `PointCloudViewerWidget` (e.g., `setTopView()`, `setLeftView()`, `setRightView()`, `setBottomView()`).
*   Implement these slots to modify `m_cameraPosition`, `m_cameraTarget`, `m_cameraUp`, `m_cameraDistance`, `m_cameraYaw`, and `m_cameraPitch` to achieve the desired orthogonal view.
    *   **Top View**: Camera directly above `m_cameraTarget`, looking down. `m_cameraUp` should be aligned with a consistent "forward" direction (e.g., Y-axis).
    *   **Left View**: Camera to the left of `m_cameraTarget`, looking right.
    *   **Right View**: Camera to the right of `m_cameraTarget`, looking left.
    *   **Bottom View**: Camera directly below `m_cameraTarget`, looking up.
*   Ensure these methods call `updateCamera()` to refresh the view.

**MainWindow to PointCloudViewerWidget Connection:**
*   In `MainWindow`'s new slots (e.g., `onTopViewClicked()`), call the corresponding `PointCloudViewerWidget` slot (e.g., `m_viewer->setTopView()`).

### **User Story 2: User can see a 3D UCS indicator in the viewer**

**Frontend (Qt/OpenGL - PointCloudViewerWidget):**
*   **Shader for UCS**: Create a separate, simple shader program within `PointCloudViewerWidget` specifically for rendering the UCS arrows. This shader will likely use basic vertex and fragment shaders to draw lines.
*   **UCS Geometry**: Define vertex data for three lines representing the X, Y, and Z axes (e.g., from (0,0,0) to (1,0,0) for X, etc.). Assign distinct colors (e.g., Red for X, Green for Y, Blue for Z).
*   **Rendering Logic**: In `PointCloudViewerWidget::paintGL()`:
    *   After rendering the point cloud, bind the UCS shader program.
    *   Calculate a separate model-view-projection matrix for the UCS. This matrix should position the UCS in the top-right corner of the screen (using normalized device coordinates or similar fixed-screen-position logic) and apply only the camera's rotation (not translation) to the UCS, so it rotates with the view but stays fixed on screen.
    *   Draw the UCS lines using `glDrawArrays(GL_LINES, ...)`.
*   **Positioning**: Determine the appropriate screen-space coordinates for the UCS widget (e.g., using `glViewport` and `glScissor` or by adjusting the UCS's projection matrix) to place it in the top-right corner.
*   **Size**: Define a fixed size for the UCS indicator (e.g., 50x50 pixels).

### **Consolidated Actions to Undertake (From Section 3 of the backlog)**

**Frontend (Qt/UI - MainWindow):**
*   Add "View" menu to `MainWindow`'s menu bar.
*   Add `QActions` for "Top View", "Left View", "Right View", "Bottom View" to the "View" menu.
*   Add `QPushButtons` for "Top View", "Left View", "Right View", "Bottom View" to `MainWindow`'s `m_buttonLayout`.
*   Implement new private slots in `MainWindow.cpp` (e.g., `MainWindow::onTopViewClicked()`, etc.) that call corresponding methods on `m_viewer`.

**Frontend (Qt/OpenGL - PointCloudViewerWidget):**
*   **Fixed Views:**
    *   Add public slots `setTopView()`, `setLeftView()`, `setRightView()`, `setBottomView()` to `PointCloudViewerWidget.h`.
    *   Implement these slots in `PointCloudViewerWidget.cpp` to adjust `m_cameraPosition`, `m_cameraTarget`, `m_cameraUp`, `m_cameraDistance`, `m_cameraYaw`, `m_cameraPitch` for the specific orthogonal view, then call `updateCamera()`.
    *   Ensure `updateCamera()` correctly re-calculates `m_viewMatrix` based on these parameters.
*   **UCS Indicator:**
    *   Add new member variables to `PointCloudViewerWidget.h` for UCS rendering: `QOpenGLShaderProgram *m_ucsShaderProgram;`, `QOpenGLBuffer m_ucsVertexBuffer;`, `QOpenGLVertexArrayObject m_ucsVertexArrayObject;`, and uniform locations.
    *   Create `setupUCSShaders()` method in `PointCloudViewerWidget.cpp` to compile and link a simple vertex/fragment shader for drawing lines (UCS axes).
    *   Create `setupUCSBuffers()` method to define the vertex data (positions and colors) for the X, Y, Z axes and upload to `m_ucsVertexBuffer`.
    *   Create `drawUCS()` method in `PointCloudViewerWidget.cpp`:
        *   Calculate the UCS MVP matrix. This will involve:
            *   Getting the camera's rotation component from `m_viewMatrix`.
            *   Creating a projection matrix for a fixed screen-space position (e.g., orthographic projection mapped to a small corner of the viewport).
            *   Creating a model matrix to scale and position the UCS within that screen-space corner.
        *   Bind `m_ucsShaderProgram`.
        *   Set UCS-specific uniforms (MVP matrix, colors).
        *   Bind `m_ucsVertexArrayObject`.
        *   Draw the lines (`glDrawArrays(GL_LINES, ...)`).
        *   Release.
    *   Call `setupUCSShaders()` and `setupUCSBuffers()` in `initializeGL()`.
    *   Call `drawUCS()` in `paintGL()`.

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/bd654c26-4eac-4dac-8779-ed399c896b94/paste.txt
[2] https://www.qt.io/blog/qt-6.9-released
[3] https://spyro-soft.com/developers/qt5-to-qt-6migration-step-by-step
[4] https://wiki.qt.io/Deprecation
[5] https://www.qt.io/blog/qt-6.5.3-released
[6] https://stackoverflow.com/questions/71086422/cmake-cannot-find-packages-within-qt6-installation
[7] https://www.qt.io/resources/videos/qt-6.9-major-feature-updates
[8] https://github.com/qutebrowser/qutebrowser/issues/8464
[9] https://github.com/qt/qtbase
[10] https://github.com/qutebrowser/qutebrowser/issues/8444
[11] https://doc.qt.io/qt-6/configure-options.html

---
Answer from Perplexity: pplx.ai/share