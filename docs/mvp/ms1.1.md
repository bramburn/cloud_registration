Sprint 1.1 Backlog: E57 MVP Foundation
Introduction

This document outlines the detailed backlog for Sprint 1.1 of the FARO Scene Registration MVP Completion and E57 File Handling Enhancement project. The primary focus of this sprint is to establish the foundational elements for E57 file processing, which represents a critical path for the project's success. This involves two parallel objectives: implementing the low-level parsing of E57 file headers as per the ASTM E2807 standard, and setting up the crucial libE57Format dependency within the project's build system. Successfully completing this sprint is an essential prerequisite for all subsequent E57 data handling, as it ensures that the application can reliably identify and begin to interpret E57 files, thereby mitigating significant technical risks early in the development cycle.

This backlog is derived from the "Product Requirements Document: FARO Scene Registration MVP Completion and E57 File Handling Enhancement" (referred to as PRD_E57_MVP from document ID 34f38214-6df6-4618-aa27-60ac5d3c8c80) and the "Technical Audit of E57 File Handling and Project Setup Functionality" (referred to as AUDIT_E57_MVP from MVP and E57 File Loading_.pdf).
User Stories
User Story 1: Implement E57 File Header Parsing

    User Story 1: As a developer, I want to implement robust E57 file header parsing so that the application can correctly identify valid E57 files and extract essential initial metadata needed for further processing, ensuring basic compliance with the ASTM E2807 standard.

        Description: This user story focuses on creating the capability to read and validate the header section of an E57 file. The E57 file header serves as a "magic number" and a file roadmap, containing critical information such as the file signature (fileSignature), format version (majorVersion, minorVersion), and the exact location and size of the XML section (xmlPayloadOffset, xmlPayloadLength) which describes the rest of the file's content. Correctly parsing this header is the non-negotiable first step in reliably handling E57 files. A failure at this stage could lead to attempts to process invalid files or a complete misinterpretation of the file's internal structure. This functionality is critical for the project as per PRD_E57_MVP (Phase 1, Sprint 1.1) and directly addresses the identified gap E57-C-002 in AUDIT_E57_MVP.

        Priority: Critical

        Estimated Effort: 5 Days (as per PRD_E57_MVP)

        Source Document Reference: PRD_E57_MVP (Sprint 1.1 Plan), AUDIT_E57_MVP (Task ID: E57-C-002)

        Actions to Undertake:

            Thoroughly review the ASTM E2807 standard documentation, focusing on the section defining the precise byte-level layout of the E57 file header structure (fields: fileSignature, majorVersion, minorVersion, fileLength, xmlPayloadOffset, xmlPayloadLength).

            Design a well-encapsulated C++ class (e.g., E57HeaderParser) with methods to read an E57 file stream and populate a dedicated, plain-old-data structure (e.g., E57HeaderData) with the extracted information.

            Implement file I/O logic using std::ifstream in binary mode to read the necessary bytes (typically the first 48 bytes) from the beginning of a given E57 file path.

            Implement strict validation logic to compare the read fileSignature against the required constant string "ASTM E57 3D Image File Format Std. V1.0". This check must be exact.

            Implement logic to correctly parse and store the majorVersion, minorVersion, xmlPayloadOffset, and xmlPayloadLength values from the header, paying close attention to the specified little-endian byte order for all multi-byte integer fields.

            Implement comprehensive and robust error handling for a variety of failure scenarios, such as:

                File not found or inaccessible due to permissions.

                File is smaller than the required header size (48 bytes).

                The file signature does not match the ASTM standard, indicating an invalid file type.

                Header fields contain illogical values (e.g., an xmlPayloadOffset that points beyond the end of the file as indicated by fileLength).

            Develop a detailed suite of unit tests using the gtest framework to cover all success and failure pathways, ensuring the parser is reliable and predictable.

        References between Files:

            E57HeaderParser.h will declare the E57HeaderParser class and the E57HeaderData data structure.

            E57HeaderParser.cpp will contain the implementation of the parsing and validation logic.

            TestE57HeaderParser.cpp will be the dedicated unit test file that includes E57HeaderParser.h and links against its object file to verify its functionality.

            A future E57Reader or E57DataManager module (planned for Sprint 2.1) will be the primary consumer of this component, using E57HeaderParser as the initial gatekeeper before handing off to libE57Format.

        Acceptance Criteria:

            The system can successfully read and parse the header of a valid E57 file conforming to the ASTM E2807 standard, correctly mapping the first 48 bytes into the E57HeaderData struct.

            The extracted fileSignature field exactly matches the string "ASTM E57 3D Image File Format Std. V1.0".

            The extracted majorVersion and minorVersion numbers are correctly parsed and stored.

            The xmlPayloadOffset and xmlPayloadLength (both 64-bit unsigned integers) are correctly parsed, and their values are numerically valid within the context of the file's total fileLength.

            The system correctly identifies, rejects, and reports a specific error for files that do not possess the valid E57 signature.

            The system gracefully handles and reports I/O or parsing errors for files that are too short to contain a complete header, preventing buffer overruns.

            The system reports appropriate, distinct errors if header fields contain inconsistent values (e.g., xmlPayloadOffset + xmlPayloadLength > fileLength).

            All header parsing logic, including every validation check and error path, is covered by unit tests, with a target code coverage of >= 90%.

            The parsing logic is proven to correctly handle the little-endian byte order for all numerical fields (uint32_t and uint64_t).

        Testing Plan:

            Unit Tests:

                Test Case 1.1: Parse a valid E57 v1.0 file header.

                Test Case 1.2: Attempt to parse a non-E57 file (e.g., a .txt or .jpg) and verify a specific "Invalid Signature" error is returned.

                Test Case 1.3: Attempt to parse a truncated file (e.g., a 20-byte file) and verify a "File Too Short" error is returned.

                Test Case 1.4: Attempt to parse a header with an invalid XML offset/length and verify an "Invalid Header" error is returned.

User Story 2: Setup libE57Format Dependency

    User Story 2: As a developer, I want to integrate the libE57Format library into the project using vcpkg and configure CMake so that the project can successfully compile and link against it, enabling its use for all subsequent E57 file parsing and writing operations.

        Description: This story involves the setup of libE57Format, a critical third-party library that serves as a well-regarded, open-source implementation of the ASTM E2807 standard. Using this library saves significant development time by abstracting the immense complexity of parsing the E57 XML hierarchy and binary data chunks. The integration will be managed through vcpkg for streamlined dependency acquisition and CMake for build system configuration. This ensures a clean, reproducible build process. This task is critical for the project as per PRD_E57_MVP (Phase 1, Sprint 1.1) and establishes the core engine for all future E57 interactions.

        Priority: Critical

        Estimated Effort: 3 Days (as per PRD_E57_MVP)

        Source Document Reference: PRD_E57_MVP (Sprint 1.1 Plan, Build System and Dependencies section).

        Actions to Undertake:

            Modify the vcpkg.json manifest file to explicitly add libe57format as a project dependency, locking to a specific version if necessary to ensure stability.

            Run vcpkg install and inspect the output to confirm that libe57format and its own transitive dependencies (notably the Xerces-C XML parser) are downloaded, built, and installed correctly.

            Update the root CMakeLists.txt file (and/or module-specific CMake files) to robustly locate and link the library. This involves using find_package(libe57format REQUIRED) which integrates with the vcpkg toolchain, and then linking the library to the appropriate target via target_link_libraries(${PROJECT_NAME} PRIVATE libe57format).

            Create a minimal "smoke test" program or a dedicated unit test. This program will simply include a key libE57Format header (e.g., <e57/E57Simple.h>) and call a basic, static API function (e.g., e57::GetLibraryVersionMajor(), e57::GetLibraryVersionMinor()) to provide definitive proof of successful compilation and linkage.

            Perform a full, clean compilation of the entire project on the target Windows environment to confirm there are no build, linkage, or header-not-found errors related to the new dependency.

            If a Continuous Integration (CI) system is in use, update its build scripts to ensure the vcpkg dependencies are correctly restored and utilized during automated builds.

        References between Files:

            vcpkg.json: Will be modified to declare the libe57format dependency.

            CMakeLists.txt: Will be modified to find and link libe57format.

            The linkage test program (e.g., tests/linkage_tests/TestLibE57Linkage.cpp) will demonstrate the usage of the library's headers.

            All future C++ files that perform deep interaction with E57 data (e.g., E57DataManager.cpp from Sprint 2.1) will include libe57format headers and depend on this successful linkage.

        Acceptance Criteria:

            libe57format is declared as a dependency in the vcpkg.json manifest file.

            The vcpkg install command completes successfully, installing libe57format and its dependencies (like Xerces-C) into the local vcpkg cache.

            The project's CMake configuration correctly finds the libe57format library and its include directories via the find_package command, without requiring any hardcoded paths.

            The entire project and all its sub-modules compile and link successfully without any unresolved external symbol errors related to libe57format.

            The minimal linkage test program compiles, links, runs, and successfully prints the library's version number to the console, confirming runtime accessibility.

            The build process remains stable and reproducible on the designated Windows development environment.

        Testing Plan:

            Build Verification:

                Test Case 2.1: Perform a full "clean and build" of the entire solution in MSVC. The build must succeed without any errors or warnings related to the new dependency.

            Linkage Test:

                Test Case 2.2: Compile and execute the minimal libE57Format API call program. The program must exit with a code of 0.

            CI Verification:

                Test Case 2.3: Trigger a CI build and confirm that it completes successfully, demonstrating the dependency is correctly handled in the automated environment.

List of Files being Created (Sprint 1.1)

    File 1: src/e57_parser/E57HeaderParser.h

        Purpose: Declares the E57HeaderParser class and its associated data structures. This file serves as the public interface for the header parsing module.

        Contents:

            A struct E57HeaderData containing fields for all header elements: char fileSignature[32], uint32_t majorVersion, uint32_t minorVersion, uint64_t fileLength, uint64_t xmlPayloadOffset, uint64_t xmlPayloadLength.

            A class definition for E57HeaderParser with public methods like Parse(const std::string& filePath) and GetData() const.

            Standard include guards (#pragma once or #ifndef/#define) and necessary standard library headers like <string> and <cstdint>.

    File 2: src/e57_parser/E57HeaderParser.cpp

        Purpose: Implements the detailed logic for the E57HeaderParser class.

        Contents:

            Implementations of the E57HeaderParser methods declared in the header file.

            File I/O logic using std::ifstream to open files in binary mode and read the first 48 bytes into a buffer.

            Byte-level signature verification logic.

            Low-level parsing of integer fields, including any necessary byte-swapping logic to ensure correctness on little-endian systems.

            Implementation of the error handling and reporting mechanisms, likely throwing custom exceptions for different failure modes.

        Relationships: Includes E57HeaderParser.h.

    File 3: tests/e57_parser/TestE57HeaderParser.cpp

        Purpose: Contains a comprehensive suite of gtest unit tests for the E57HeaderParser class.

        Contents:

            A gtest test fixture that handles the setup and teardown of test data, which may involve creating temporary files with valid and intentionally corrupted headers.

            Multiple TEST_F blocks, each targeting a specific scenario: successful parsing, handling of files with incorrect signatures, handling of truncated files, and handling of headers with logically invalid field values.

            Liberal use of ASSERT_TRUE, ASSERT_EQ, and ASSERT_THROW to verify expected outcomes and error conditions with high precision.

        Relationships: Includes E57HeaderParser.h and gtest/gtest.h. Links against the compiled E57HeaderParser object code.

    File 4: vcpkg.json (Modification)

        Purpose: The project's manifest file for the vcpkg C++ package manager, defining all external dependencies in a declarative way.

        Contents (Relevant Change): The addition of a new entry for libe57format within the dependencies array.

        Relationships: This file is the source of truth for vcpkg, which in turn provides the libraries that CMake needs to find.

    File 5: CMakeLists.txt (Modification)

        Purpose: The primary build script for CMake, defining project targets, source files, include directories, and library linkage.

        Contents (Relevant Changes):

            The addition of a find_package(libe57format REQUIRED) call, which instructs CMake to locate the library using its vcpkg integration.

            An update to a target_link_libraries call, adding libe57format to ensure the linker can resolve its symbols for our application executable or libraries.

        Relationships: Orchestrates the entire build process.

    File 6: tests/linkage_tests/TestLibE57Linkage.cpp

        Purpose: A minimal "smoke test" program to provide a fast, definitive check that the libE57Format library is correctly linked into the build.

        Contents:

            An #include <e57/E57Simple.h> directive.

            A simple main function that makes a single, static API call, for example e57::GetLibraryVersionMajor(), and prints the result to stdout.

            Returns 0 on success.

        Relationships: This file has no purpose other than to depend on libE57Format for linkage testing.

Acceptance Criteria (Overall Sprint 1.1)

    All detailed acceptance criteria for User Story 1 (E57 File Header Parsing) are fully met and verified by unit tests.

    All detailed acceptance criteria for User Story 2 (Setup libE57Format Dependency) are fully met and verified by build system tests.

    The project builds cleanly from scratch on a Windows machine using the established CMake/vcpkg/MSVC toolchain.

    All new code is thoroughly commented, especially logic interpreting the E57 standard, and adheres strictly to project coding standards.

    Project documentation (e.g., a BUILD.md or README) is updated to reflect the new dependency and any changes to the build process.

Testing Plan (Overall Sprint 1.1)

    Unit Testing:

        Execute the entire gtest suite, ensuring all tests within TestE57HeaderParser.cpp pass.

        Review the code coverage report for E57HeaderParser to ensure the >= 90% target is met.

    Build System & Integration Testing:

        Perform multiple clean builds of the project after every significant change to CMake or vcpkg.json.

        Compile and run the TestLibE57Linkage program to confirm runtime linkage is functional.

    Code Review: A formal peer review will be conducted for all new and modified files (E57HeaderParser.h/.cpp, TestE57HeaderParser.cpp, vcpkg.json, CMakeLists.txt) to check for correctness, quality, and adherence to standards.

Assumptions and Dependencies

    Assumptions:

        The development environment (Windows, MSVC, CMake, Git, vcpkg) is fully configured and operational for all developers.

        A digital copy of the ASTM E2807 standard documentation is available to developers, and they have familiarized themselves with its core concepts.

        The libE57Format library, as provided by vcpkg, is stable and functions correctly on the target Windows platform.

        The chosen version of libE57Format is compatible with the project's C++ standard (e.g., C++17) and other key libraries like PCL and Qt.

        The existing project architecture is modular enough to accommodate a new e57_parser component without significant refactoring.

    Dependencies:

        External Libraries: libE57Format (which itself depends on Xerces-C for XML parsing), and gtest (for unit testing).

        Tools: vcpkg, CMake, a C++ Compiler (specifically MSVC), and Git.

        Documentation: ASTM E2807 E57 Standard Specification, libE57Format library documentation.

        Sprint/Phase Dependencies: Successful completion of Sprint 1.1 is a hard blocker for Sprint 1.2 (CRC Validation) and all subsequent E57 data reading and processing tasks in Phase 2.

Non-Functional Requirements

    Build Integrity: The integration of libE57Format must not destabilize the existing build process or unduly complicate the developer setup workflow. Build times should not increase excessively.

    Code Quality & Maintainability: All new C++ code must be clean, well-structured, and self-documenting where possible. Complex algorithms or logic derived directly from the standard must be accompanied by comments explaining the "why".

    Error Handling: The header parsing module must be exceptionally robust. It must never crash the application on invalid input. Instead, it must propagate errors in a controlled way (e.g., via exceptions) that can be caught by higher-level logic to display a user-friendly message.

    Platform Compatibility: All development, testing, and functionality for this sprint are exclusively targeted for the Windows platform using the MSVC compiler.

    Performance: Header parsing is expected to be a near-instantaneous operation. It involves reading a small, fixed-size block of data from disk and should not introduce any noticeable latency when a user selects a file to open, even from a network location.

Conclusion

Sprint 1.1 lays the critical groundwork for the E57 functionality of the FARO Scene Registration MVP. It is a foundational sprint focused on de-risking the project by tackling the most fundamental aspects of E57 file handling first. By successfully implementing a robust E57 header parser and cleanly integrating the core libE57Format library, the project will establish a trusted and stable foundation, ensuring that all future development on E57 data handling is built upon solid, reliable components.