# **Project Backlog: CMake Rebuild**

### **Introduction**

This document provides a detailed project backlog for Sprint 2 of the CMake rebuild. Building upon the stable foundation created in Sprint 1, this sprint's objective is to integrate the full application source code, introduce Qt as a core dependency, and correctly configure all third-party libraries using vcpkg. The successful completion of this sprint will result in a fully buildable and runnable application, resolving the critical dependency and path-related errors that plagued the original build system.

### **User Stories**

* **User Story 2**: Integrate Core Dependencies and UI Framework  
  * **Description**: As a developer, I want to configure the build system to correctly find and link against the Qt 6 framework using the vcpkg dependency manager. This will allow me to compile the application's main window and user interface components.  
  * **Goal**: To establish a robust and reproducible method for handling the project's primary framework dependency (Qt) and validate the vcpkg integration.  
  * **Outcome**: The application builds and launches a QMainWindow, proving that the Qt framework and vcpkg toolchain are correctly configured.  
* **User Story 3**: Build the Full Application with All Source Code  
  * **Description**: As a developer, I want to add all the application's source files to the build system and link against all necessary libraries (like Eigen3) to produce a fully-featured, runnable executable.  
  * **Goal**: To achieve a complete, successful build of the entire application using the new CMake script, resolving all previous compilation and linking errors.  
  * **Outcome**: A runnable CloudRegistration executable that includes all the features and functionality of the original application, built cleanly without errors.

### **Actions to Undertake: Sprint 2 (Atomic Steps)**

This sprint is broken down into atomic tasks to ensure a meticulous and verifiable process.

1. **Dependency Management Setup (vcpkg)**:  
   * **Task 1.1**: Create a new file named vcpkg.json in the project's root directory.  
   * **Task 1.2**: Add initial Qt dependencies to vcpkg.json:  
     {  
       "name": "cloud-registration",  
       "version-string": "1.0.0",  
       "dependencies": \[  
         "qtbase",  
         "qtopengl"  
       \]  
     }

   * **Task 1.3**: **(Crucial)** Edit CMakeLists.txt and add set(CMAKE\_TOOLCHAIN\_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file") at the very top, before the project() command. *Note: The path should be adjusted to the developer's local vcpkg installation.*  
2. **Qt Framework Integration**:  
   * **Task 2.1**: In CMakeLists.txt, enable Qt's automatic tools:  
     set(CMAKE\_AUTOMOC ON)  
     set(CMAKE\_AUTORCC ON)  
     set(CMAKE\_AUTOUIC ON)

   * **Task 2.2**: Add the find\_package command for Qt: find\_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets).  
   * **Task 2.3**: Restore the original src/main.cpp and src/mainwindow.cpp / src/mainwindow.h files from the old repository state.  
   * **Task 2.4**: Update the add\_executable command to include the main window files: add\_executable(CloudRegistration src/main.cpp src/mainwindow.cpp).  
   * **Task 2.5**: Link the required Qt libraries: target\_link\_libraries(CloudRegistration PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets).  
   * **Task 2.6**: Build the project. It should compile and link successfully, launching an empty main window. This validates the Qt and vcpkg setup.  
3. **Integrating Application Source Code**:  
   * **Task 3.1**: Create a variable to hold the list of source files: set(APP\_SOURCES src/main.cpp src/mainwindow.cpp).  
   * **Task 3.2**: **(Incremental)** Add the UI source files (from the src/ui/ directory) to the APP\_SOURCES variable. Use ${CMAKE\_CURRENT\_SOURCE\_DIR} to ensure correct pathing, e.g., ${CMAKE\_CURRENT\_SOURCE\_DIR}/src/ui/ExportDialog.cpp. **Compile after adding each small group of related files.**  
   * **Task 3.3**: **(Incremental)** Add the core logic files (from src/, src/registration/, src/camera/, etc.) to the APP\_SOURCES variable. Compile frequently.  
   * **Task 3.4**: Update the executable target to use the variable: add\_executable(CloudRegistration ${APP\_SOURCES}).  
4. **Integrating Third-Party Libraries**:  
   * **Task 4.1**: Update vcpkg.json to include all required libraries: "eigen3", "libe57format", "xerces-c", "gtest", "vulkan".  
   * **Task 4.2**: In CMakeLists.txt, add find\_package() commands for the new dependencies:  
     find\_package(Eigen3 REQUIRED)  
     find\_package(E57Format CONFIG REQUIRED)  
     find\_package(XercesC REQUIRED)

   * **Task 4.3**: Add the remaining parser and algorithm source files to the APP\_SOURCES list.  
   * **Task 4.4**: Update target\_link\_libraries to include the new dependencies:  
     target\_link\_libraries(CloudRegistration PRIVATE  
         Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGLWidgets  
         Eigen3::Eigen  
         E57Format  
         XercesC::XercesC  
     )

   * **Task 4.5**: Perform a final, full build of the application.

### **References between Files**

* CMakeLists.txt will now reference vcpkg.json implicitly via the CMAKE\_TOOLCHAIN\_FILE.  
* CMakeLists.txt will reference all .cpp files in the src/ directory and its subdirectories.  
* The CloudRegistration target will have link-time dependencies on Qt 6, Eigen3, E57Format, and Xerces-C libraries.

### **List of Files being Created/Modified**

* **File 1**: CMakeLists.txt (Modified)  
  * **Purpose**: To define the complete build configuration for the main application.  
  * **Contents**: Will be updated with CMAKE\_TOOLCHAIN\_FILE setting, find\_package calls for all dependencies, a comprehensive list of all source files, and target\_link\_libraries for all required libraries.  
  * **Relationships**: Drives the entire build process.  
* **File 2**: vcpkg.json (New File)  
  * **Purpose**: To declare all external third-party dependencies for the vcpkg package manager.  
  * **Contents**: A JSON object listing the names of all required libraries (e.g., qtbase, eigen3).  
  * **Relationships**: Read by the vcpkg.cmake toolchain file to automate dependency installation.  
* **File 3**: src/main.cpp and src/mainwindow.cpp/h (Restored)  
  * **Purpose**: These files are restored to their original state to build the full application UI.  
  * **Contents**: The original application entry point and main window implementation.  
  * **Relationships**: Part of the CloudRegistration executable target.

### **Acceptance Criteria**

* **AC-1**: Running cmake \-B build successfully configures the project and triggers vcpkg to install all declared dependencies.  
* **AC-2**: The cmake \--build build command compiles all application source files and links against all dependencies without any errors.  
* **AC-3**: The known build errors ("Eigen3 not found", "Cannot find source file") are permanently resolved.  
* **AC-4**: The final executable launches and displays the full user interface, including the project hub and main viewer components.  
* **AC-5**: The application is runnable and stable at startup (further functional testing will occur after the test suite is re-integrated).

### **Testing Plan**

* **Test Case 1**: Validate Dependency and Qt Integration  
  * **Test Data**: The CMakeLists.txt and vcpkg.json after completing Actions 1 & 2\.  
  * **Actions**: Run rm \-rf build and cmake \-B build. Check the console output to ensure vcpkg downloads and installs Qt. Then run cmake \--build build.  
  * **Expected Result**: The build succeeds, and the application launches to show an empty QMainWindow.  
  * **Testing Tool**: Command Line / Terminal, Visual Inspection.  
* **Test Case 2**: Full Application Build  
  * **Test Data**: The CMakeLists.txt and vcpkg.json after completing Actions 3 & 4\.  
  * **Actions**: Run rm \-rf build and cmake \-B build. Then run cmake \--build build.  
  * **Expected Result**: The build completes with \[100%\] Built target CloudRegistration and no errors.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 3**: Application Launch and UI Verification  
  * **Test Data**: The fully compiled executable from Test Case 2\.  
  * **Actions**: Launch the CloudRegistration executable.  
  * **Expected Result**: The application starts without crashing. The Project Hub view is visible and all UI elements appear correctly rendered.  
  * **Testing Tool**: Manual Execution and Visual Inspection.

### **Assumptions and Dependencies**

* **Assumptions**:  
  * vcpkg is installed on the developer's machine and its location is known.  
  * The C++ compiler is compatible with the library versions specified in vcpkg.  
* **Dependencies**:  
  * **New**: Qt 6 (Core, Gui, Widgets, OpenGLWidgets)  
  * **New**: Eigen3  
  * **New**: E57Format (and its dependency, Xerces-C)  
  * **New**: Google Test (gtest) and Vulkan (declared for future sprints)

### **Non-Functional Requirements**

* **NFR-1 (Maintainability)**: CMakeLists.txt should group source files logically to make it easier to add or remove components in the future.  
* **NFR-2 (Build Performance)**: The build process should leverage pre-compiled libraries from vcpkg to ensure that clean builds are reasonably fast, as only the application's source code needs to be compiled.

### **Conclusion**

Completing Sprint 2 will mark a major milestone: a fully functional application built with a clean, modern, and dependency-managed CMake system. This resolves the primary build-time issues and sets the stage for re-integrating the test suite in Sprint 3 (Phase 2), which will validate the application's correctness.