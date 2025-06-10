# **Project Backlog: CMake Rebuild**

### **Introduction**

This document provides a detailed project backlog for Sprint 3 of the CMake rebuild. With a fully compiling application now established, the primary goal of this sprint is to re-integrate the entire suite of unit and integration tests. This critical phase will validate the correctness of the application logic and ensure that the new build system produces a functionally identical and correct executable. This sprint corresponds to **Phase 2: Test Suite Integration**.

### **User Stories**

* **User Story 4**: Validate Application Correctness with Unit Tests  
  * **Description**: As a developer, I want to compile and run all existing unit and integration tests using the new CMake build system. This will verify that the core application logic, algorithms, and component interactions have not been broken by the build system refactoring.  
  * **Goal**: To achieve a state where ctest can execute the entire test suite successfully, confirming the application's functional correctness.  
  * **Outcome**: A high-confidence build where all tests pass, proving the new CMakeLists.txt correctly handles the complexities of both the main application and its associated tests.

### **Actions to Undertake: Sprint 3 (Atomic Steps)**

This sprint is broken down into atomic tasks to ensure a meticulous and verifiable process.

1. **Enable Testing in CMake**:  
   * **Task 1.1**: In CMakeLists.txt, add the enable\_testing() command to activate CTest support for the project.  
   * **Task 1.2**: Add the tests directory to the project's subdirectories if not already present: add\_subdirectory(tests). *Note: For this project, we will add test targets directly in the root CMakeLists.txt for simplicity, so this step is skipped.*  
2. **Integrate Google Test Framework**:  
   * **Task 2.1**: Modify the vcpkg.json file to include the Google Test library. Add "gtest" to the "dependencies" array.  
   * **Task 2.2**: Run cmake \-B build to trigger vcpkg to download and install Google Test and Google Mock.  
   * **Task 2.3**: In CMakeLists.txt, add the find\_package(GTest CONFIG REQUIRED) command to locate the installed libraries.  
   * **Task 2.4**: Add a message to confirm that GTest and GMock were found:  
     if(TARGET GTest::gtest\_main AND TARGET GTest::gmock\_main)  
         message(STATUS "Google Test and Google Mock found.")  
     else()  
         message(WARNING "Google Mock not found \- some tests may be disabled.")  
     endif()

3. **Create Test Executable Targets (Iterative Process)**:  
   * **Task 3.1**: **(Start with one test)** Identify a simple test file, e.g., tests/test\_target.cpp.  
   * **Task 3.2**: Create the first test executable: add\_executable(TargetTests tests/test\_target.cpp src/registration/Target.cpp). Note that the test's source dependencies must also be included.  
   * **Task 3.3**: Link the test against its dependencies:  
     target\_link\_libraries(TargetTests PRIVATE GTest::gtest\_main GTest::gmock\_main Qt6::Core)

   * **Task 3.4**: Register the test with CTest: add\_test(NAME TargetTests COMMAND TargetTests).  
   * **Task 3.5**: Compile and run just this single test (cmake \--build build \--target TargetTests and ctest \-R TargetTests).  
   * **Task 3.6**: **(Iterate)** Repeat steps 3.2-3.5 for every single test file in the tests/ directory (test\_alignment\_engine.cpp, test\_error\_analysis.cpp, etc.), creating a unique executable and test for each one. **Address any include path or link errors as they appear.** For each test, carefully add its specific source dependencies.  
4. **Final Verification**:  
   * **Task 4.1**: After all test targets have been added and build successfully, run the full test suite from the command line: ctest \--output-on-failure.  
   * **Task 4.2**: Ensure all tests pass. If any fail, debug and fix the application code or the test's CMake configuration.  
   * **Task 4.3**: Create a custom target to simplify running tests: add\_custom\_target(run\_tests COMMAND ${CMAKE\_CTEST\_COMMAND} \--output-on-failure).

### **References between Files**

* Each new test executable (e.g., TargetTests) will reference its corresponding test source file (e.g., tests/test\_target.cpp) and any application source files it depends on (e.g., src/registration/Target.cpp).  
* All test targets will link against the GTest and Qt6::Test libraries provided by vcpkg.  
* The CMakeLists.txt file will be the central point defining all test targets and their dependencies.

### **List of Files being Created/Modified**

* **File 1**: CMakeLists.txt (Modified)  
  * **Purpose**: To define all test executables and their dependencies.  
  * **Contents**: Will be updated with enable\_testing(), a find\_package(GTest ...) call, and a series of add\_executable, target\_link\_libraries, and add\_test commands for every test case.  
  * **Relationships**: Orchestrates the compilation and registration of all tests.  
* **File 2**: vcpkg.json (Modified)  
  * **Purpose**: To add Google Test as a project dependency.  
  * **Contents**: The "dependencies" array will be updated to include "gtest".  
  * **Relationships**: Informs vcpkg to install the GTest framework.

### **Acceptance Criteria**

* **AC-1**: The cmake \--build build command successfully compiles the main application AND all test executables without errors.  
* **AC-2**: The ctest command successfully discovers and runs every single test defined in the tests/ directory.  
* **AC-3**: **100% of the tests must pass.** This is the primary validation that the new build system is producing a correct application.  
* **AC-4**: The run\_tests custom target, when executed (cmake \--build build \--target run\_tests), runs the full CTest suite.

### **Testing Plan**

* **Test Case 1**: Single Test Compilation  
  * **Test Data**: The CMakeLists.txt with only one test target defined (e.g., TargetTests).  
  * **Actions**: Run cmake \--build build \--target TargetTests.  
  * **Expected Result**: The specific test executable is compiled successfully without errors.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 2**: Single Test Execution  
  * **Test Data**: The compiled test executable from TC1.  
  * **Actions**: Run ctest \-R TargetTests \--output-on-failure.  
  * **Expected Result**: CTest finds and runs the single test, and it passes.  
  * **Testing Tool**: CTest (via command line).  
* **Test Case 3**: Full Test Suite Execution  
  * **Test Data**: The final CMakeLists.txt with all test targets defined.  
  * **Actions**: Run rm \-rf build, cmake \-B build, cmake \--build build, and finally ctest \--output-on-failure.  
  * **Expected Result**: All tests are discovered, executed, and report a 100% pass rate.  
  * **Testing Tool**: CTest (via command line).

### **Assumptions and Dependencies**

* **Assumptions**:  
  * The existing test source code is correct and is expected to pass. Any failures are assumed to be due to build configuration errors (missing links, incorrect includes) until proven otherwise.  
* **Dependencies**:  
  * **New**: gtest (from vcpkg).  
  * **Existing**: Qt6 Test library (Qt6::Test) will be required for tests involving Qt components.

### **Non-Functional Requirements**

* **NFR-1 (Developer Experience)**: The process of adding a new test to the CMakeLists.txt should be simple and follow a clear, repeatable pattern.  
* **NFR-2 (Build Performance)**: The test suite should build efficiently. Build times will be monitored for significant regressions compared to the old system (if benchmarks are available).

### **Conclusion**

Sprint 3 is the validation phase of the CMake rebuild. By successfully re-integrating the entire test suite, we confirm that the new build system is not only capable of compiling the application but is doing so correctly. At the end of this sprint, the project will have a fully buildable, runnable, and verifiable codebase, paving the way for the final modularization and optimization phase.