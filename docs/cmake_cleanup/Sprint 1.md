# **Project Backlog: CMake Rebuild**

### **Introduction**

This document provides a detailed project backlog for the incremental rebuild of the Cloud Registration application's CMake build system. This first sprint focuses exclusively on **Phase 0: The Foundation**, as outlined in the PRD. The goal is to establish a minimal, robust, and working "hello world" application that compiles successfully. This foundational step is critical for isolating dependency and configuration issues before adding the full complexity of the application.

### **User Stories**

* **User Story 1**: Establish a Foundational Build System  
  * **Description**: As a developer, I want a new, minimal CMakeLists.txt that can compile a simple main.cpp into an executable. This will serve as a stable starting point for incrementally adding the rest of the application's source code and dependencies.  
  * **Goal**: To validate the core build toolchain (CMake, C++ Compiler) and environment setup before introducing project-specific complexities.  
  * **Outcome**: A new build directory containing a simple, runnable executable, proving the foundational build configuration is correct.

### **Actions to Undertake: Sprint 1 (Atomic Steps)**

This sprint is broken down into atomic tasks to ensure a meticulous and verifiable process.

1. **Workspace Preparation**:  
   * **Task 1.1**: Create a new branch in the Git repository (e.g., feature/cmake-rebuild).  
   * **Task 1.2**: In the project root, rename the existing CMakeLists.txt to CMakeLists.txt.old to serve as a reference.  
   * **Task 1.3**: Create a new, completely empty file named CMakeLists.txt in the project root.  
2. **Initial CMake Configuration**:  
   * **Task 2.1**: Edit the new CMakeLists.txt. Add cmake\_minimum\_required(VERSION 3.16).  
   * **Task 2.2**: Add project(CloudRegistration VERSION 1.0.0 LANGUAGES CXX).  
   * **Task 2.3**: Set the C++ standard:  
     set(CMAKE\_CXX\_STANDARD 17\)  
     set(CMAKE\_CXX\_STANDARD\_REQUIRED ON)

   * **Task 2.4**: Run cmake \-B build from the root directory. This command is expected to fail because no executable target has been defined, but it validates the basic syntax.  
3. **Minimal Source File Creation**:  
   * **Task 3.1**: Create a minimal src/main.cpp file containing only a main function that prints "Hello, World\!" to the console and returns 0\. This temporarily replaces the existing main.cpp logic.  
     \#include \<iostream\>  
     int main() {  
         std::cout \<\< "Hello, CMake Rebuild\!" \<\< std::endl;  
         return 0;  
     }

4. **Creating the Executable Target**:  
   * **Task 4.1**: Add the executable target to CMakeLists.txt: add\_executable(CloudRegistration src/main.cpp).  
   * **Task 4.2**: Set properties for organized output directories:  
     set(CMAKE\_RUNTIME\_OUTPUT\_DIRECTORY ${CMAKE\_BINARY\_DIR}/bin)

   * **Task 4.3**: Run cmake \-B build again. It should now configure successfully.  
   * **Task 4.4**: Run cmake \--build build. This should compile main.cpp and create the CloudRegistration executable in the build/bin directory.  
5. **Adding Compiler Flags**:  
   * **Task 5.1**: Add the platform-specific compiler flag sections for MSVC, GCC, and Clang as defined in the old CMakeLists.txt.old. This includes warning levels (/W4, \-Wall), UTF-8 settings (/utf-8, \-finput-charset=UTF-8), and optimization flags.  
   * **Task 5.2**: Re-run the build (cmake \--build build) to ensure the flags are syntactically correct and accepted by the compiler.

### **References between Files**

* CMakeLists.txt will directly reference src/main.cpp to create the CloudRegistration executable.  
* There are no other inter-file dependencies in this initial sprint.

### **List of Files being Created**

* **File 1**: CMakeLists.txt (New Version)  
  * **Purpose**: To serve as the foundational build script for the entire project.  
  * **Contents**: Will contain cmake\_minimum\_required, project, C++ standard settings, compiler flags, and a single add\_executable command.  
  * **Relationships**: Refers to src/main.cpp.  
* **File 2**: src/main.cpp (Temporary Version)  
  * **Purpose**: To provide a minimal compilable source file to validate the toolchain.  
  * **Contents**: A standard main function with a "Hello, World\!" print statement.  
  * **Relationships**: Is referenced by CMakeLists.txt.

### **Acceptance Criteria**

* **AC-1**: The new CMakeLists.txt must successfully configure the project using the cmake \-B build command without any errors.  
* **AC-2**: The cmake \--build build command must successfully compile a minimal main.cpp into an executable named CloudRegistration.  
* **AC-3**: The resulting executable must be located in a build/bin directory.  
* **AC-4**: Running the executable from the command line must print "Hello, CMake Rebuild\!" and exit with code 0\.  
* **AC-5**: The build process must not produce any compiler warnings related to the CMake script itself.

### **Testing Plan**

* **Test Case 1**: Validate CMake Configuration  
  * **Test Data**: The new CMakeLists.txt and temporary src/main.cpp.  
  * **Actions**: From the project root, run rm \-rf build (to ensure a clean state) followed by cmake \-B build.  
  * **Expected Result**: The command completes without errors, and a build directory containing Makefiles or project files is created.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 2**: Validate Compilation  
  * **Test Data**: The configured build system from Test Case 1\.  
  * **Actions**: From the project root, run cmake \--build build.  
  * **Expected Result**: The build process completes with \[100%\] Built target CloudRegistration and no compiler errors. An executable is created in build/bin.  
  * **Testing Tool**: Command Line / Terminal.  
* **Test Case 3**: Validate Executable  
  * **Test Data**: The compiled executable from Test Case 2\.  
  * **Actions**: Run the executable (e.g., ./build/bin/CloudRegistration).  
  * **Expected Result**: The console outputs "Hello, CMake Rebuild\!". The command exits with a status code of 0\.  
  * **Testing Tool**: Command Line / Terminal.

### **Assumptions and Dependencies**

* **Assumptions**:  
  * The developer has a compatible C++17 compiler (MSVC, GCC, or Clang) installed and available in the system's PATH.  
  * CMake version 3.16 or higher is installed and available in the system's PATH.  
* **Dependencies**:  
  * There are no third-party library dependencies for this sprint.

### **Non-Functional Requirements**

* **NFR-1 (Maintainability)**: The CMakeLists.txt file must be clean, well-commented, and easy to read.  
* **NFR-2 (Portability)**: The CMake script must be platform-agnostic, correctly applying compiler flags for Windows (MSVC) and Unix-like systems (GCC/Clang).

### **Conclusion**

Upon successful completion of Sprint 1, the project will have a verified, minimal, and stable build foundation. This de-risks the rest of the rebuild process by confirming that the development environment, compiler, and basic CMake configuration are all functioning correctly. The project will be perfectly positioned to begin Sprint 2, where Qt and vcpkg integration will be introduced.