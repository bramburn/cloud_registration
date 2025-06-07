# **Backlog: Phase 1 \- Sprint 1: Project Setup & Basic File Loading (C++/Qt6)**

## **Introduction**

This document details the backlog for Sprint 1 of Phase 1: "Core Viewer & Basic Management" for the Open-Source Point Cloud Registration Software. The primary goal of this sprint is to establish the foundational application structure using **C++ and Qt6**, enable basic point cloud file loading, and implement initial 3D visualization capabilities. This sprint will deliver a visible and interactive desktop application capable of displaying a single point cloud.

## **User Stories**

* **User Story 1**: As a user, I want to load an E57 file into the application so that I can begin processing my scan data.  
  * **Description**: This user story focuses on the fundamental ability to import a point cloud file in the E57 format into the application. The application should initialize a 3D viewing environment using Qt's OpenGL capabilities and display the loaded point cloud. For this sprint, the focus is on handling a single, relatively small E57 file to establish the core loading and rendering pipeline.  
  * **Actions to Undertake**:  
    1. **Project Initialization**: Set up a new Qt6 C++ project (e.g., using CMake or qmake). Configure necessary modules like QtWidgets, QtGui, QtOpenGLWidgets.  
    2. **Main Window Setup**: Create a QMainWindow or QWidget as the main application window.  
    3. **OpenGL Widget Integration**: Create a custom QOpenGLWidget subclass (e.g., PointCloudViewerWidget) to handle 3D rendering. Override its initializeGL(), resizeGL(), and paintGL() methods.  
    4. **UI for File Input**: Add a QPushButton (e.g., "Open File") to the main window. Connect its clicked() signal to a slot that opens a QFileDialog.  
    5. **File Reading Logic**: In the slot, use QFile to open the selected E57 file. Read the file content into a QByteArray or std::vector\<char\>.  
    6. **Basic E57 Parsing**: Develop a simplified C++ E57 parser that can extract point coordinates (X, Y, Z) from the raw binary data. Initially, focus on uncompressed points and ignore color/intensity/metadata for simplicity. This will likely involve direct binary reading and understanding E57's internal structure.  
    7. **OpenGL VBO/VAO Setup**: In PointCloudViewerWidget, set up Vertex Buffer Objects (VBOs) and Vertex Array Objects (VAOs) to efficiently store and render the point cloud data.  
    8. **Shader Implementation**: Write basic OpenGL shaders (vertex and fragment shaders) to render points. The vertex shader will transform point coordinates, and the fragment shader will assign a basic color.  
    9. **Point Cloud Rendering (OpenGL)**: In paintGL(), bind the VAO and draw the points using glDrawArrays(GL\_POINTS, ...).  
    10. **Camera Controls**: Implement rudimentary mouse-based camera controls (e.g., fixed orbit around the scene origin) by handling mousePressEvent(), mouseMoveEvent(), and wheelEvent() in PointCloudViewerWidget to update the camera's view matrix.  
  * **References between Files**:  
    * CMakeLists.txt (or .pro file) defines project structure and links Qt modules.  
    * main.cpp initializes the Qt application and the main window.  
    * mainwindow.h/mainwindow.cpp define the main application window and its UI elements.  
    * pointcloudviewerwidget.h/pointcloudviewerwidget.cpp define the custom OpenGL widget for 3D rendering. This will interact with OpenGL APIs directly.  
    * e57parser.h/e57parser.cpp handle the binary parsing of E57 files.  
    * Shader files (.vert, .frag) will be loaded and compiled by pointcloudviewerwidget.cpp.  
  * **Acceptance Criteria**:  
    * The application launches as a desktop window, displaying an empty 3D viewport.  
    * An "Open File" button is clearly visible and clickable.  
    * Upon clicking "Open File" and selecting a valid, small .e57 file, the point cloud data is displayed within the 3D viewport.  
    * The displayed point cloud is visually coherent (i.e., not a jumbled mess of points).  
    * The user can use mouse interactions (e.g., click-and-drag) to orbit the camera around the displayed point cloud.  
    * No application crashes or significant errors are reported in the console.  
  * **Testing Plan**:  
    * **Test Case 1**: Verify application launch and empty viewport.  
      * **Test Data**: N/A  
      * **Expected Result**: Desktop application window opens, displays a blank OpenGL viewport and an "Open File" button.  
      * **Testing Tool**: Manual desktop application testing.  
    * **Test Case 2**: Load a valid small E57 file.  
      * **Test Data**: A pre-prepared, small .e57 file containing simple point coordinates (e.g., a cube or sphere).  
      * **Expected Result**: The point cloud from the E57 file is rendered correctly in the 3D viewport.  
      * **Testing Tool**: Manual desktop application testing, visual inspection.  
    * **Test Case 3**: Test basic camera orbit.  
      * **Test Data**: Any loaded point cloud.  
      * **Expected Result**: Clicking and dragging the mouse rotates the camera view around the point cloud smoothly.  
      * **Testing Tool**: Manual desktop application testing.  
    * **Test Case 4**: Attempt to load an invalid file type.  
      * **Test Data**: A .txt or .jpg file.  
      * **Expected Result**: The application gracefully handles the invalid file (e.g., displays an error message box, does not crash).  
      * **Testing Tool**: Manual desktop application testing.  
    * **Test Case 5**: E57 Parser Unit Test (Mock Data)  
      * **Description**: Verify the E57Parser class correctly extracts point data from a simulated E57 binary structure.  
      * **Test Data**: A small std::vector\<char\> or QByteArray manually crafted to mimic a simple E57 point data block.  
      * **Expected Result**: The parser function returns the expected std::vector of X, Y, Z coordinates.  
      * **Testing Tool**: C++ unit testing framework (e.g., Google Test, Catch2).

## **Actions to Undertake**

1. **Setup Project Structure**:  
   * Create a new Qt project using CMake (CMakeLists.txt) or qmake (.pro file).  
   * Define the project name, source files, and link necessary Qt modules (QtWidgets, QtGui, QtOpenGLWidgets).  
2. **Main Application Entry Point**:  
   * Implement main.cpp to create a QApplication instance and instantiate MainWindow.  
3. **Main Window (UI)**:  
   * Create mainwindow.h and mainwindow.cpp for the MainWindow class.  
   * Design the MainWindow layout using QVBoxLayout or QGridLayout.  
   * Add a QPushButton labeled "Open File" to the layout.  
   * Add an instance of PointCloudViewerWidget to the layout.  
   * Connect the "Open File" button's clicked() signal to a slot in MainWindow (e.g., onOpenFileClicked()).  
4. **OpenGL Viewer Widget**:  
   * Create pointcloudviewerwidget.h and pointcloudviewerwidget.cpp for the PointCloudViewerWidget class, inheriting from QOpenGLWidget.  
   * Override initializeGL(): Set up OpenGL context, clear color, enable depth testing. Compile and link basic vertex and fragment shaders.  
   * Override resizeGL(int w, int h): Update the OpenGL viewport and projection matrix for the camera.  
   * Override paintGL(): Clear buffers, activate shaders, bind VAO/VBOs, draw points using glDrawArrays(GL\_POINTS, ...).  
   * Implement mousePressEvent(), mouseMoveEvent(), wheelEvent() to update camera parameters (view matrix) based on user input. Call update() to trigger paintGL().  
   * Add a public method (e.g., loadPointCloud(const std::vector\<float\>& points)) to receive point data from MainWindow.  
5. **E57 Parser Implementation**:  
   * Create e57parser.h and e57parser.cpp for the E57Parser class.  
   * Define a method (e.g., parse(const QString& filePath)) that takes a file path.  
   * Inside parse(), use QFile to open and read the E57 file as binary data.  
   * Implement logic to navigate the E57 file structure (which is HDF5-based) to find the point data block. For this sprint, a very basic direct binary read of a known simple E57 structure is acceptable.  
   * Extract X, Y, Z coordinates and store them in a std::vector\<float\>.  
   * Return the std::vector\<float\> containing the point data.  
6. **Integration (MainWindow to Parser to Viewer)**:  
   * In MainWindow::onOpenFileClicked(), use QFileDialog::getOpenFileName() to get the E57 file path.  
   * Instantiate E57Parser and call its parse() method.  
   * If parsing is successful, pass the returned point data (std::vector\<float\>) to PointCloudViewerWidget::loadPointCloud().  
   * Implement a simple loading indicator (e.g., QProgressDialog or a status bar message).

## **References between Files**

* **CMakeLists.txt (or .pro file)**:  
  * **Dependency**: Links main.cpp, mainwindow.cpp, pointcloudviewerwidget.cpp, e57parser.cpp, and shader files.  
  * **Dependency**: Specifies Qt modules (QtWidgets, QtGui, QtOpenGLWidgets) required for compilation.  
* **main.cpp**:  
  * **Dependency**: Includes mainwindow.h.  
  * **Interaction**: Creates and shows MainWindow.  
* **mainwindow.h/mainwindow.cpp**:  
  * **Dependency**: Includes pointcloudviewerwidget.h, e57parser.h.  
  * **Interaction**: MainWindow owns an instance of PointCloudViewerWidget. It calls E57Parser methods and passes data to PointCloudViewerWidget.  
* **pointcloudviewerwidget.h/pointcloudviewerwidget.cpp**:  
  * **Dependency**: Includes \<QOpenGLWidget\>, \<QOpenGLFunctions\>, \<QOpenGLBuffer\>, \<QOpenGLVertexArrayObject\>, \<QOpenGLShader\>, \<QMatrix4x4\>, \<QVector3D\>.  
  * **Interaction**: Receives point data from MainWindow and uses OpenGL APIs to render it. Handles mouse events for camera control.  
* **e57parser.h/e57parser.cpp**:  
  * **Dependency**: Includes \<QFile\>, \<QDataStream\>, \<QByteArray\>, \<QString\>, \<vector\>.  
  * **Interaction**: Reads E57 file data and returns processed point data to MainWindow.  
* **point.vert (Vertex Shader)**:  
  * **Dependency**: Loaded and compiled by pointcloudviewerwidget.cpp.  
  * **Interaction**: Takes vertex position, applies model-view-projection transformations.  
* **point.frag (Fragment Shader)**:  
  * **Dependency**: Loaded and compiled by pointcloudviewerwidget.cpp.  
  * **Interaction**: Assigns a color to each fragment (pixel).

## **List of Files being Created**

* **File 1**: CMakeLists.txt (or project\_name.pro)  
  * **Purpose**: Build system configuration file for the C++/Qt project.  
  * **Contents**: Specifies source files, header files, Qt modules to link, and build targets.  
  * **Relationships**: Orchestrates the compilation and linking of all C++ source and header files.  
* **File 2**: main.cpp  
  * **Purpose**: The entry point of the C++ application.  
  * **Contents**: Contains the main function, which initializes the QApplication and creates and shows the MainWindow.  
  * **Relationships**: Includes mainwindow.h.  
* **File 3**: mainwindow.h  
  * **Purpose**: Header file for the MainWindow class, defining the main application window.  
  * **Contents**: Class declaration for MainWindow, including private member variables for UI elements (e.g., QPushButton, PointCloudViewerWidget) and private slots for event handling (e.g., onOpenFileClicked()).  
  * **Relationships**: Included by mainwindow.cpp and main.cpp. Declares PointCloudViewerWidget and E57Parser instances.  
* **File 4**: mainwindow.cpp  
  * **Purpose**: Implementation file for the MainWindow class.  
  * **Contents**: Constructor to set up the UI layout, connect signals/slots, and implement the onOpenFileClicked() slot, which handles file dialog, calls E57Parser, and passes data to PointCloudViewerWidget.  
  * **Relationships**: Includes mainwindow.h, pointcloudviewerwidget.h, e57parser.h, \<QFileDialog\>, \<QMessageBox\>.  
* **File 5**: pointcloudviewerwidget.h  
  * **Purpose**: Header file for the custom QOpenGLWidget subclass responsible for 3D point cloud rendering.  
  * **Contents**: Class declaration for PointCloudViewerWidget, inheriting from QOpenGLWidget and QOpenGLFunctions. Declares private member variables for OpenGL buffers (VBO, VAO), shaders, camera matrices, and point data. Declares overridden OpenGL event handlers (initializeGL, resizeGL, paintGL) and mouse event handlers.  
  * **Relationships**: Included by pointcloudviewerwidget.cpp and mainwindow.h.  
* **File 6**: pointcloudviewerwidget.cpp  
  * **Purpose**: Implementation file for PointCloudViewerWidget, containing all OpenGL rendering logic.  
  * **Contents**: Implementations of initializeGL(), resizeGL(), paintGL(), and mouse event handlers. Contains logic for setting up shaders, VBOs/VAOs, and drawing points. Implements loadPointCloud() to update OpenGL buffers.  
  * **Relationships**: Includes pointcloudviewerwidget.h, \<QOpenGLFunctions\>, \<QOpenGLShader\>, \<QMatrix4x4\>, \<QVector3D\>, etc. Loads point.vert and point.frag.  
* **File 7**: e57parser.h  
  * **Purpose**: Header file for the E57Parser class, responsible for parsing E57 files.  
  * **Contents**: Class declaration for E57Parser, including a public method parse(const QString& filePath) that returns a std::vector\<float\> of point coordinates.  
  * **Relationships**: Included by e57parser.cpp and mainwindow.cpp.  
* **File 8**: e57parser.cpp  
  * **Purpose**: Implementation file for the E57Parser class.  
  * **Contents**: Implementation of the parse() method, which uses QFile and potentially QDataStream or direct binary reading to extract XYZ point data from the E57 file.  
  * **Relationships**: Includes e57parser.h, \<QFile\>, \<QDataStream\>, \<QByteArray\>, \<QString\>, \<vector\>.  
* **File 9**: point.vert  
  * **Purpose**: OpenGL Vertex Shader for rendering points.  
  * **Contents**: GLSL code for a basic vertex shader that takes vertex positions, applies model-view-projection transformations, and passes the transformed position to the fragment shader.  
  * **Relationships**: Loaded and compiled by pointcloudviewerwidget.cpp.  
* **File 10**: point.frag  
  * **Purpose**: OpenGL Fragment Shader for rendering points.  
  * **Contents**: GLSL code for a basic fragment shader that outputs a fixed color for each point.  
  * **Relationships**: Loaded and compiled by pointcloudviewerwidget.cpp.

## **Acceptance Criteria**

* The desktop application loads successfully and displays a main window with an empty 3D viewport.  
* A clear "Open File" button is present on the UI.  
* Clicking the "Open File" button opens a native file selection dialog.  
* Selecting a small, valid .e57 file results in its point cloud being rendered in the 3D viewport.  
* The rendered point cloud appears correctly, without visual artifacts or missing points (for the small test file).  
* Basic camera controls (orbiting) function smoothly using mouse interaction, allowing the user to view the point cloud from different angles.  
* The application does not crash or display critical errors in the console when loading or interacting with the point cloud.  
* A simple loading indication (e.g., a message in the status bar or a temporary dialog) is shown during file parsing.

## **Testing Plan**

The testing plan for Sprint 1 will primarily involve manual testing and visual inspection, complemented by basic unit tests for the E57 parsing logic.

* **Testing Methodology**:  
  * **Manual Desktop Application Testing**: Directly interact with the compiled desktop application to verify UI responsiveness, file loading, and 3D navigation.  
  * **Unit Testing (C++)**: Write isolated tests for the E57Parser class to ensure it correctly extracts point data from mock E57 binary structures.  
  * **Console Monitoring**: Continuously monitor the application's console output for any errors, warnings, or performance issues.  
* **Test Cases**:  
  * **Test Case 1**: Application Launch and Initial State  
    * **Description**: Verify that the desktop application launches correctly and presents the expected initial UI.  
    * **Test Data**: N/A  
    * **Expected Result**: A desktop application window opens, displaying a blank OpenGL viewport and an "Open File" button. No console errors.  
    * **Testing Tool**: Manual desktop application testing.  
  * **Test Case 2**: Valid Small E57 File Loading  
    * **Description**: Test the core functionality of loading and displaying a point cloud from a valid E57 file.  
    * **Test Data**: A pre-prepared, small .e57 file (e.g., 10,000-50,000 points) with known geometry (e.g., a simple cube or sphere).  
    * **Expected Result**: The point cloud from the E57 file is rendered accurately in the 3D viewport. A "Loading..." indicator appears and then disappears.  
    * **Testing Tool**: Manual desktop application testing, visual inspection.  
  * **Test Case 3**: Basic Camera Orbit Functionality  
    * **Description**: Verify that the implemented camera controls allow for smooth 3D navigation.  
    * **Test Data**: Any successfully loaded point cloud.  
    * **Expected Result**: Using the mouse (e.g., left-click and drag), the camera orbits around the center of the loaded point cloud smoothly and responsively.  
    * **Testing Tool**: Manual desktop application testing.  
  * **Test Case 4**: Loading a Non-E57 File  
    * **Description**: Ensure the application handles incorrect file types gracefully.  
    * **Test Data**: A .txt or .jpg file.  
    * **Expected Result**: The application should not crash. It should display an informative error message box to the user (e.g., "Invalid file type" or "Failed to parse E57 file"). No critical console errors.  
    * **Testing Tool**: Manual desktop application testing.  
  * **Test Case 5**: E57 Parser Unit Test (Mock Data)  
    * **Description**: Verify the E57Parser class correctly extracts point data from a simulated E57 QByteArray or std::vector\<char\>.  
    * **Test Data**: A small QByteArray or std::vector\<char\> manually crafted or generated to mimic a simple E57 point data block.  
    * **Expected Result**: The parser function returns the expected std::vector\<float\> of X, Y, Z coordinates.  
    * **Testing Tool**: C++ unit testing framework (e.g., Google Test, Catch2).

## **Assumptions and Dependencies**

* **Operating System**: Development and testing will primarily target common desktop operating systems (e.g., Windows, macOS, Linux).  
* **Qt6 Framework**: Qt6 libraries (specifically QtWidgets, QtGui, QtOpenGLWidgets) are assumed to be installed and configured for the development environment.  
* **OpenGL**: A compatible OpenGL driver and hardware are assumed to be available on the target system for 3D rendering.  
* **Build System**: CMake or qmake is assumed to be the build system.  
* **E57 File Structure (Simplified)**: For Sprint 1, the E57 parser will assume a relatively simple E57 file structure, focusing only on uncompressed XYZ point data. It will not handle complex E57 features like advanced compression, multiple data blocks, or intricate metadata.  
* **Small File Sizes**: Initial testing and performance expectations are based on small to medium-sized E57 files (up to a few million points). Optimization for very large files will be addressed in later sprints.  
* **No Persistence**: No data saving or project persistence will be implemented in this sprint.

## **Non-Functional Requirements**

* **Performance**: The application should load and render small E57 files smoothly, maintaining a responsive frame rate (e.g., \>30 FPS) during camera navigation. OpenGL rendering should be efficient.  
* **Usability**: The "Open File" button should be intuitive and easily accessible. Camera controls should feel natural for users familiar with 3D applications. Error messages should be clear and informative.  
* **Maintainability**: Code should be modular, well-commented, follow C++ best practices, and adhere to Qt coding conventions to facilitate future development and community contributions.  
* **Security**: As a desktop application, standard security practices for C++ development should be followed (e.g., input validation, memory safety). File handling should be robust against malformed input.  
* **Scalability (Future)**: The initial architecture should be designed to allow for future expansion to handle larger datasets, more complex E57 features, and additional file formats without major re-architecture.

## **Conclusion**

Sprint 1 lays the essential groundwork for the open-source point cloud registration software using **C++ and Qt6**. By successfully implementing file loading, basic 3D rendering with OpenGL, and navigation, we will have a tangible and interactive desktop prototype that demonstrates the core viewing capability. This foundational sprint is crucial for validating the chosen technology stack and setting the stage for subsequent development phases focused on data management and registration.