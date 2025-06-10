# **Project Backlog: CMake Rebuild**

### **Introduction**

This document provides a detailed project backlog for Sprint 4 of the CMake rebuild. Having established a fully functional and tested build in the previous sprints, the focus now shifts from correctness to maintainability and optimization. This sprint, corresponding to **Phase 3: Refactoring for Maintainability**, will transform the flat list of source files into a modular structure of interconnected libraries. This architectural improvement will make the build system easier to understand, faster to compile, and simpler to extend for future development.

### **User Stories**

* **User Story 5**: Modularize the Codebase into Logical Libraries  
  * **Description**: As a developer, I want to refactor the single list of source files into multiple static libraries, each representing a logical component of the application (e.g., "Algorithms", "Parsers", "Rendering").  
  * **Goal**: To create a modular build system that mirrors the application's architecture, improving build times by only recompiling changed components and clarifying dependencies between different parts of the codebase.  
  * **Outcome**: A CMakeLists.txt file that defines and links several static libraries, resulting in a cleaner and more efficient build process.  
* **User Story 6**: Improve the Developer Experience with Build System Refinements  
  * **Description**: As a developer, I want to enhance the CMakeLists.txt with convenience features and clear documentation to make the build process more intuitive and robust.  
  * **Goal**: To finalize the build script with best practices, making it easy for any developer on the team to build, test, and package the application.  
  * **Outcome**: A polished, well-documented CMakeLists.txt file that includes custom targets for running tests and clear installation rules for deployment.

### **Actions to Undertake: Sprint 4 (Atomic Steps)**

This sprint is broken down into atomic tasks to refactor the existing, working build system.

1. **Refactor Core Logic into a "Core" Library**:  
   * **Task 1.1**: In CMakeLists.txt, identify a small set of foundational source files that have minimal dependencies (e.g., project.cpp, pointdata.h, octree.cpp).  
   * **Task 1.2**: Create a new static library target for these files: add\_library(Core STATIC src/project.cpp src/octree.cpp).  
   * **Task 1.3**: Define the public include directory for this library so other components can find its headers: target\_include\_directories(Core PUBLIC ${CMAKE\_CURRENT\_SOURCE\_DIR}/src).  
   * **Task 1.4**: Modify the main CloudRegistration executable to link against this new library: target\_link\_libraries(CloudRegistration PRIVATE Core).  
   * **Task 1.5**: Remove the source files now in the Core library from the main executable's source list.  
   * **Task 1.6**: Compile the project (cmake \--build build) to verify that the main executable can successfully link against the new Core library.  
2. **Create Specialized Static Libraries (Iterative Process)**:  
   * **Task 2.1**: **(Algorithms Library)** Create a new Algorithms library containing files like LeastSquaresAlignment.cpp and ICPRegistration.cpp. This library will depend on Core and Eigen3. Link it accordingly: target\_link\_libraries(Algorithms PRIVATE Core Eigen3::Eigen).  
   * **Task 2.2**: **(Parsers Library)** Create a Parsers library with e57parserlib.cpp and lasparser.cpp. Link it against its dependencies (E57Format, XercesC::XercesC).  
   * **Task 2.3**: **(Rendering Library)** Create a Rendering library with files from src/rendering/ and src/camera/. This will link against Qt6::OpenGL.  
   * **Task 2.4**: **(UI Library)** Create a UI library with custom widget sources from src/ui/. This will link against Qt components.  
   * **Task 2.5**: For each library created, remove its source files from the main CloudRegistration target's source list and add the new library to its target\_link\_libraries list. **Compile after each new library is integrated to immediately catch any linking or include path errors.**  
3. **Update Test Suite Dependencies**:  
   * **Task 3.1**: Go through each test executable defined in Sprint 3 (e.g., TargetTests, AlignmentEngineTests).  
   * **Task 3.2**: For each test, modify its target\_link\_libraries command to link against the new modular libraries (e.g., Core, Algorithms) instead of individual .cpp files.  
   * **Task 3.3**: Run ctest to ensure all tests still compile, link, and pass with the new modular dependency structure.  
4. **Final Polish and Documentation**:  
   * **Task 4.1**: Review the entire CMakeLists.txt file. Add comments to explain the purpose of each library target and the overall structure.  
   * **Task 4.2**: Verify that the source list for the main CloudRegistration executable is now minimal, containing primarily main.cpp, mainwindow.cpp, and other top-level files.  
   * **Task 4.3**: Add a custom target to simplify running the test suite: add\_custom\_target(run\_all\_tests COMMAND ctest \--output-on-failure).  
   * **Task 4.4**: Define installation rules using install(TARGETS ...) and install(FILES ...) to specify how the executable, libraries, and resources should be packaged for deployment.

### **References between Files**

* The main target CloudRegistration will now depend on the new library targets: Core, Algorithms, Parsers, Rendering, and UI.  
* Specialized libraries will depend on the Core library (e.g., Algorithms links to Core).  
* Test targets will depend on the application libraries they need to test (e.g., AlignmentEngineTests will link against Algorithms).

### **List of Files being Created/Modified**

* **File 1**: CMakeLists.txt (Heavily Modified)  
  * **Purpose**: To define a modular build system based on static libraries.  
  * **Contents**: Will be refactored to use multiple add\_library() commands. The add\_executable() command for the main target will be simplified, and target\_link\_libraries() will be used extensively to define the dependency graph.  
  * **Relationships**: Orchestrates the build of all libraries, the main executable, and all test executables.

### **Acceptance Criteria**

* **AC-1**: The project successfully compiles and links, producing a functional application and passing test suite.  
* **AC-2**: The CMakeLists.txt file contains multiple add\_library( ... STATIC ...) commands for different logical components of the application.  
* **AC-3**: The main CloudRegistration executable's source list in CMake is significantly smaller, primarily linking against the new libraries.  
* **AC-4**: The build process remains correct; the final application is functionally identical to the one produced at the end of Sprint 2\.  
* **AC-5**: The run\_all\_tests custom target successfully executes the entire CTest suite.

### **Testing Plan**

* **Test Case 1**: Modular Library Compilation  
  * **Test Data**: The refactored CMakeLists.txt.  
  * **Actions**: Run cmake \--build build \--target Core (or another library).  
  * **Expected Result**: The specified library and its dependencies compile successfully without errors.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 2**: Full Application Build with Libraries  
  * **Test Data**: The refactored CMakeLists.txt.  
  * **Actions**: Run a clean build: rm \-rf build, cmake \-B build, cmake \--build build.  
  * **Expected Result**: The entire project, including all libraries and executables, compiles and links successfully.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 3**: Test Suite Validation Post-Refactor  
  * **Test Data**: The compiled test executables from TC2.  
  * **Actions**: Run ctest \--output-on-failure.  
  * **Expected Result**: All tests pass, confirming that linking against libraries instead of source files did not break functionality.  
  * **Testing Tool**: CTest (via command line).  
* **Test Case 4**: Application Regression Test  
  * **Test Data**: The final compiled application.  
  * **Actions**: Launch the application. Load a point cloud file. Perform basic camera manipulations.  
  * **Expected Result**: The application runs without crashing and core functionality remains intact.  
  * **Testing Tool**: Manual Execution.

### **Assumptions and Dependencies**

* **Assumptions**:  
  * The logical grouping of source files into libraries is sound and will not introduce circular dependencies.  
  * The functionality of the application and tests is correct (as verified in Sprint 3), so any new issues are likely due to the refactoring of the build script.  
* **Dependencies**:  
  * No new third-party dependencies are introduced in this sprint. This sprint only rearranges existing code and dependencies.

### **Non-Functional Requirements**

* **NFR-1 (Maintainability)**: The primary goal. The final CMakeLists.txt must be significantly easier to navigate and understand than the previous monolithic version.  
* **NFR-2 (Build Performance)**: By modularizing the code, incremental build times should improve. A change in one library (e.g., Algorithms) should not trigger a complete re-compilation of all source files.

### **Conclusion**

Sprint 4 marks the final and most significant refactoring effort. Upon its completion, the project will have a professional-grade, modular, and maintainable CMake build system. This robust foundation will simplify future development, improve build times, and make the project significantly easier for new developers to join.