Sprint 1.2 Backlog: E57 Data Integrity and XML Parsing
1. Introduction

This document outlines the detailed backlog for Sprint 1.2 of the FARO Scene Registration MVP project. Building upon the foundational work of Sprint 1.1, which focused on E57 header parsing and dependency setup, this sprint is dedicated to two critical areas: ensuring data integrity through CRC-32 validation and enabling structural understanding of E57 files via robust XML parsing.

The successful completion of this sprint is paramount, as it directly addresses the most significant gaps identified in the initial technical audit: the application's inability to detect corrupted data and its failure to parse the full structure of E57 files. By tackling these challenges now, we will establish a reliable data-handling pipeline, which is an essential prerequisite for all subsequent development, including point cloud processing and registration.

This backlog is derived from the "Product Requirements Document" (prd.md) and the "Technical Audit of E57 File Handling" (ms1.1.md).
2. User Stories
User Story 1: Implement CRC-32 Validation for E57 Binary Data Integrity

    User Story 1: As a developer, I want the application to perform CRC-32 checksum validation on all binary sections of an E57 file during the loading process, so that data corruption can be reliably detected and the application does not process invalid or incomplete point cloud data.

    Description: This user story addresses a critical flaw identified in the technical audit (E57-C-001), where the application would load corrupted data without warning. The E57 standard specifies a CRC-32 checksum for each page in a binary section to ensure data integrity. Implementing this validation is a non-negotiable requirement for building a robust and reliable application. This functionality will be encapsulated within a new E57BinaryReader component.

    Priority: Critical

    Estimated Effort: 7 Days

    Source Document Reference: prd.md (Section 3.1, 5), ms1.1.md (Gap E57-C-001)

    Actions to Undertake:

        Design and implement an E57BinaryReader class responsible for reading and validating binary sections of E57 files.

        This class will take the file path and the XML-derived offsets for binary sections as input.

        Implement logic to read binary data in 1024-byte pages, where each page consists of a 4-byte CRC checksum and a 1020-byte payload.

        For each page, calculate the CRC-32 checksum of the 1020-byte payload.

        Compare the calculated checksum with the checksum stored in the page's header.

        Implement robust error handling to manage checksum mismatches, throwing specific exceptions (e.g., E57DataCorruptionError).

        If a checksum fails, the application should halt the loading process for that scan and report a clear error to the user.

        Develop a comprehensive suite of unit tests for the E57BinaryReader, including tests with valid data, intentionally corrupted data, and edge cases (e.g., empty binary sections).

    References between Files:

        E57BinaryReader.h: Declares the E57BinaryReader class.

        E57BinaryReader.cpp: Implements the binary reading and CRC-32 validation logic.

        E57XmlParser.cpp (from User Story 2): Will use the E57BinaryReader to read and process binary data sections based on information parsed from the XML section.

        TestE57BinaryReader.cpp: Unit tests for the E57BinaryReader.

    Acceptance Criteria:

        The system must correctly read and validate the CRC-32 checksum for every page in an E57 binary section.

        The system must successfully load point cloud data from a valid E57 file with correct checksums.

        The application must detect and reject E57 files with CRC checksum mismatches in any binary section page.

        When a checksum mismatch is detected, a specific, user-friendly error message must be generated, indicating data corruption.

        The application's memory usage should not significantly increase due to the CRC-32 validation process.

    Testing Plan:

        Test Case 1.1: Load a valid E57 file and verify that all data is read correctly without any CRC errors.

        Test Case 1.2: Create a test E57 file with a single corrupted binary page and verify that the application detects the CRC error and fails to load the file.

        Test Case 1.3: Create a test E57 file with multiple corrupted pages and ensure the application reports the first error it encounters.

        Test Case 1.4: Test with an E57 file that has an empty binary section to ensure no errors are thrown.

User Story 2: Implement Robust E57 XML Section Parsing

    User Story 2: As a developer, I want to implement a robust XML parser for the E57 file format that can navigate the E57 element tree, extract essential metadata, and identify the structure and location of all binary data chunks, so that the application can fully understand and process complex E57 files.

    Description: This user story addresses the deficiency in the current implementation where XML parsing is rudimentary and fails to extract key information (E57-C-003). A complete XML parser is necessary to handle multi-scan files, extract point attributes correctly, and access crucial metadata like GUIDs and coordinate systems. This functionality will be encapsulated in a new E57XmlParser class, which will leverage the libE57Format library.

    Priority: Critical

    Estimated Effort: 7 Days

    Source Document Reference: prd.md (Section 3.1, 5), ms1.1.md (Gap E57-C-003)

    Actions to Undertake:

        Design and implement an E57XmlParser class to handle the parsing of the XML section of an E57 file.

        Integrate the libE57Format library (set up in Sprint 1.1) to perform the low-level XML parsing.

        Implement logic to navigate the E57 XML DOM (Document Object Model) to locate key elements such as /e57Root, /data3D, and /images2D.

        For each /data3D section found, extract its GUID and the prototype for its point records (/data3D/points/prototype).

        Parse the PointRecord prototype to identify all available point attributes (e.g., cartesianX, cartesianY, cartesianZ, colorRed, colorGreen, colorBlue, intensity).

        Extract metadata such as CoordinateMetadata, IntensityLimits, and ColorLimits to enable correct data interpretation.

        Implement robust error handling for malformed or incomplete XML sections.

        Develop unit tests for the E57XmlParser using various E57 files with different structures (single scan, multiple scans, with/without color and intensity).

    References between Files:

        E57XmlParser.h: Declares the E57XmlParser class and associated data structures for holding parsed metadata.

        E57XmlParser.cpp: Implements the XML parsing logic using libE57Format.

        E57HeaderParser.cpp: The header parser will provide the offset and length of the XML section to the E57XmlParser.

        E57BinaryReader.cpp: The XML parser will provide the binary section details to the binary reader.

        TestE57XmlParser.cpp: Unit tests for the E57XmlParser.

    Acceptance Criteria:

        The system can successfully parse the XML section of a valid E57 file.

        The parser must correctly identify all /data3D sections in a multi-scan E57 file.

        The parser must accurately extract the PointRecord prototype for each scan, identifying all available point attributes.

        The parser must extract key metadata, including the file's GUID and CoordinateMetadata.

        The system must gracefully handle malformed XML and report a specific error.

    Testing Plan:

        Test Case 2.1: Parse a single-scan E57 file and verify that the GUID and point prototype are correctly extracted.

        Test Case 2.2: Parse a multi-scan E57 file and confirm that the parser identifies all /data3D sections.

        Test Case 2.3: Parse an E57 file with color and intensity attributes and verify that the prototype parsing correctly identifies these fields.

        Test Case 2.4: Attempt to parse an E57 file with a corrupted XML section and confirm that a descriptive error is thrown.

3. List of Files being Created

    File 1: src/e57_parser/E57BinaryReader.h

        Purpose: Declares the E57BinaryReader class for reading binary data and performing CRC-32 validation.

        Contents: Class definition for E57BinaryReader, including methods for reading binary pages and validating checksums.

        Relationships: Will be used by E57XmlParser to access binary data.

    File 2: src/e57_parser/E57BinaryReader.cpp

        Purpose: Implements the logic for the E57BinaryReader class.

        Contents: Implementation of the CRC-32 validation algorithm and binary data reading logic.

        Relationships: Includes E57BinaryReader.h.

    File 3: src/e57_parser/E57XmlParser.h

        Purpose: Declares the E57XmlParser class for parsing the E57 XML section.

        Contents: Class definition for E57XmlParser, including data structures to store parsed metadata and scan information.

        Relationships: Uses libE57Format and will be used by higher-level data management components.

    File 4: src/e57_parser/E57XmlParser.cpp

        Purpose: Implements the logic for the E57XmlParser class.

        Contents: Implementation of XML DOM traversal and metadata extraction using libE57Format.

        Relationships: Includes E57XmlParser.h.

    File 5: tests/e57_parser/TestE57BinaryReader.cpp

        Purpose: Contains unit tests for the E57BinaryReader class.

        Contents: gtest test cases for CRC-32 validation with valid and corrupted data.

        Relationships: Includes E57BinaryReader.h and links against its object file.

    File 6: tests/e57_parser/TestE57XmlParser.cpp

        Purpose: Contains unit tests for the E57XmlParser class.

        Contents: gtest test cases for parsing various E57 XML structures.

        Relationships: Includes E57XmlParser.h and links against its object file.

4. Assumptions and Dependencies

    libE57Format Integration: It is assumed that the libE57Format library, integrated in Sprint 1.1, is stable and functions correctly on the target Windows platform.

    ASTM E2807 Standard: Development will be based on the public documentation of the ASTM E2807 standard for the E57 file format.

    Sequential Development: The successful completion of Sprint 1.1 (header parsing) is a hard dependency for this sprint.

5. Non-Functional Requirements

    Performance: The CRC-32 validation and XML parsing should not introduce significant performance bottlenecks. The target is to maintain I/O performance within a 5% degradation tolerance compared to a non-validating reader.

    Error Handling: The application must provide clear, user-friendly error messages when encountering corrupted data or malformed XML. It should never crash on invalid input.

    Memory Usage: The new components should be memory-efficient, especially when handling large E57 files. Data should be processed in chunks where possible to minimize memory footprint.

6. Conclusion

Sprint 1.2 is a critical step in building a reliable and robust FARO Scene registration application. By focusing on data integrity and structural parsing, this sprint will address major technical risks and lay a solid foundation for the implementation of core application features, such as point cloud processing and registration algorithms, in subsequent sprints. The successful completion of these user stories will ensure that the application can confidently and correctly handle a wide range of E57 files.