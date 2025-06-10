# **Project Backlog: CMake Rebuild**

### **Introduction**

This document provides a detailed project backlog for Sprint 5 of the CMake rebuild. With a fully functional, tested, and modular build system now in place, this sprint focuses on finalizing the developer experience and preparing the project for production deployment and automated workflows. The primary goals are to enforce code style consistency, create distributable installers, and prepare the project for Continuous Integration (CI).

### **User Stories**

* **User Story 7**: Automate Code Formatting  
  * **Description**: As a developer, I want to integrate an automatic code formatter (like clang-format) into the build system so that all code adheres to a single, consistent style guide, reducing merge conflicts and improving readability.  
  * **Goal**: To eliminate manual formatting and ensure code style consistency across the entire codebase.  
  * **Outcome**: New CMake targets (format and check-format) that can automatically format or validate the source code.  
* **User Story 8**: Generate Application Installers  
  * **Description**: As a user or release manager, I want the build system to generate a native installer for my platform (e.g., NSIS on Windows, DEB on Linux, DMG on macOS) so I can easily distribute and install the application.  
  * **Goal**: To create a reliable, one-command process for packaging the application and all its dependencies into a distributable installer.  
  * **Outcome**: A runnable cpack command that produces a professional, platform-native installer.  
* **User Story 9**: Prepare for Continuous Integration (CI)  
  * **Description**: As a developer, I want a CI pipeline script (e.g., for GitHub Actions) that automatically builds and tests the application on every commit, so that I can catch integration errors early and ensure the main branch is always stable.  
  * **Goal**: To automate the build and test process, improving code quality and development velocity.  
  * **Outcome**: A CI workflow configuration file that can be used by platforms like GitHub Actions to build and test the project from a clean environment.

### **Actions to Undertake: Sprint 5 (Atomic Steps)**

This sprint is broken down into atomic tasks to ensure a meticulous and verifiable process.

1. **Code Formatting Integration (clang-format)**:  
   * **Task 1.1**: Create a .clang-format configuration file in the project root. Base it on a standard style like "Google" or "Microsoft" and adjust as needed.  
   * **Task 1.2**: In CMakeLists.txt, find the clang-format executable using find\_program().  
   * **Task 1.3**: Create a variable containing all C++ source and header files to be formatted (file(GLOB\_RECURSE ...)).  
   * **Task 1.4**: Add a custom target to reformat code: add\_custom\_target(format COMMAND clang-format \-i ${ALL\_CXX\_FILES}).  
   * **Task 1.5**: Add a custom target to check formatting: add\_custom\_target(check-format COMMAND clang-format \--dry-run \--Werror ${ALL\_CXX\_FILES}). This target is crucial for CI pipelines as it will exit with an error code if files are not formatted correctly.  
   * **Task 1.6**: Run format on the entire codebase once to establish a consistent baseline.  
2. **Installer Generation (CPack)**:  
   * **Task 2.1**: At the end of CMakeLists.txt, add include(CPack).  
   * **Task 2.2**: Define CPack variables: CPACK\_PACKAGE\_NAME, CPACK\_PACKAGE\_VERSION, CPACK\_PACKAGE\_VENDOR, CPACK\_PACKAGE\_DESCRIPTION\_SUMMARY, etc.  
   * **Task 2.3**: Write an install(TARGETS CloudRegistration ...) command to specify that the main executable should be included in the package. Specify the DESTINATION as bin.  
   * **Task 2.4**: Use install(FILES ...) to package resources like shaders, icons, and documentation. Specify their DESTINATION (e.g., share/shaders).  
   * **Task 2.5**: **(Crucial)** Use install(CODE ...) with the $\<TARGET\_FILE:Qt6::Core\> generator expression to automatically find and package the required Qt DLLs/shared libraries. This is a complex but robust method for handling Qt deployment.  
   * **Task 2.6**: Configure platform-specific CPack generators (e.g., set(CPACK\_GENERATOR "NSIS") for Windows).  
   * **Task 2.7**: Run cpack from the build directory to generate a test installer.  
3. **CI Pipeline Preparation (GitHub Actions)**:  
   * **Task 3.1**: Create the directory structure .github/workflows/.  
   * **Task 3.2**: Create a new file build.yml inside the workflows directory.  
   * **Task 3.3**: Define the CI workflow trigger (e.g., on: \[push, pull\_request\]).  
   * **Task 3.4**: Define a build job for a specific platform (e.g., ubuntu-latest).  
   * **Task 3.5**: Write the steps for the job:  
     1. actions/checkout@v3  
     2. A step to install dependencies (e.g., sudo apt-get install \-y ninja-build clang-format).  
     3. A step to set up vcpkg.  
     4. A step to run CMake configure: cmake \-B build \-S . \-DCMAKE\_TOOLCHAIN\_FILE=...  
     5. A step to run the build: cmake \--build build  
     6. A step to run tests: ctest \--test-dir build \--output-on-failure

### **References between Files**

* CMakeLists.txt will now execute clang-format using the rules defined in .clang-format.  
* CMakeLists.txt will contain install() commands that define the contents of the package generated by cpack.  
* The CI script (.github/workflows/build.yml) will execute the CMake and CTest commands defined in CMakeLists.txt.

### **List of Files being Created/Modified**

* **File 1**: .clang-format (New File)  
  * **Purpose**: To define the C++ code style for the entire project.  
  * **Contents**: A YAML-formatted file with clang-format style options (e.g., BasedOnStyle: Google, IndentWidth: 4).  
  * **Relationships**: Used by the format and check-format CMake targets.  
* **File 2**: .github/workflows/build.yml (New File)  
  * **Purpose**: To define the automated build-and-test pipeline for GitHub Actions.  
  * **Contents**: A YAML file specifying jobs, steps, and commands for checking out, building, and testing the code.  
  * **Relationships**: Executes commands from the CMakeLists.txt file in a remote environment.  
* **File 3**: CMakeLists.txt (Modified)  
  * **Purpose**: To add support for code formatting, packaging, and installation.  
  * **Contents**: Will be updated with find\_program(clang-format), new custom targets (format, check-format), include(CPack), and a series of install() commands.  
  * **Relationships**: The central script that enables all of this sprint's functionality.

### **Acceptance Criteria**

* **AC-1**: Running cmake \--build build \--target check-format must exit with a non-zero error code if any C++ file is incorrectly formatted.  
* **AC-2**: Running cmake \--build build \--target format must automatically reformat all C++ source and header files to match the style defined in .clang-format.  
* **AC-3**: Running cpack from the build directory must produce a platform-native installer (e.g., .exe on Windows).  
* **AC-4**: The generated installer must successfully install the application and all its required runtime dependencies (e.g., Qt DLLs) on a clean machine.  
* **AC-5**: The CI pipeline defined in build.yml must execute successfully on every push to a pull request, running the build and all tests.

### **Testing Plan**

* **Test Case 1**: Validate Code Formatting  
  * **Actions**:  
    1. Deliberately mis-format a line in a .cpp file (e.g., add extra indentation).  
    2. Run cmake \--build build \--target check-format. It should fail.  
    3. Run cmake \--build build \--target format.  
    4. Inspect the .cpp file. It should now be correctly formatted.  
    5. Run cmake \--build build \--target check-format again. It should now pass.  
  * **Expected Result**: The format and check-format targets work as designed.  
  * **Testing Tool**: Command Line, Text Editor.  
* **Test Case 2**: Validate Installer  
  * **Test Data**: The generated installer file.  
  * **Actions**:  
    1. On a clean virtual machine (or another computer) without development tools installed, run the installer.  
    2. Follow the installation prompts.  
    3. After installation, try to launch the CloudRegistration application from the installation directory.  
  * **Expected Result**: The application installs correctly and launches without errors about missing DLLs or other dependencies.  
  * **Testing Tool**: Virtual Machine (e.g., VirtualBox, VMware), Windows/Linux/macOS OS.  
* **Test Case 3**: Validate CI Pipeline  
  * **Actions**: Create a new pull request in the Git repository.  
  * **Expected Result**: The GitHub Actions pipeline automatically triggers, runs all build and test steps, and reports a "success" (green checkmark) status on the pull request.  
  * **Testing Tool**: GitHub.

### **Assumptions and Dependencies**

* **Assumptions**:  
  * The project is hosted in a Git repository on a platform that supports CI/CD, such as GitHub.  
  * Developers will have clang-format installed or it will be installed as part of the CI process.  
* **Dependencies**:  
  * **New**: clang-format (build-time tool).  
  * **New**: CPack-supported packaging tools (e.g., NSIS for Windows).

### **Non-Functional Requirements**

* **NFR-1 (Automation)**: All build, test, and packaging steps must be executable from the command line without manual intervention, making them suitable for CI/CD.  
* **NFR-2 (Reliability)**: The installer must reliably deploy a working version of the application on target platforms.  
* **NFR-3 (Code Quality)**: The automated formatting ensures a consistent and professional level of code quality and readability across the entire project.

### **Conclusion**

Sprint 5 transitions the project from a developer-focused build system to a production-ready engineering pipeline. By implementing automated formatting, packaging, and CI readiness, this sprint significantly improves developer productivity, code consistency, and deployment reliability. The project will now be in a state where it can be professionally maintained, scaled, and distributed.