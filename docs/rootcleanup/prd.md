Product Requirements Document: Repository Cleanup and Organization
1. Introduction

This document outlines the plan to clean up and reorganize the Cloud Registration project repository. Currently, the root directory contains a large number of .ps1, .md, and .cpp files that are primarily related to testing, documentation, or specific development sprints. This clutter hinders navigability, maintainability, and the overall understanding of the project's core structure.

The goal is to move these files into appropriate subdirectories, enhancing the repository's clarity, improving developer experience, and establishing a consistent file organization standard.
2. Goals

    Improve Repository Clarity: Make it easier for new and existing contributors to understand the project's structure and quickly locate relevant files.

    Enhance Maintainability: Centralize related files, simplifying updates and reducing the risk of orphaned or duplicated content.

    Streamline Development Workflow: Promote better practices for file placement, reducing friction in future development.

    Reduce Root Directory Clutter: Move non-essential files out of the top-level directory.

3. Scope

This PRD focuses on reorganizing existing .ps1, .md, and .cpp files currently located in the root directory. It does not involve:

    Refactoring existing code logic within the moved files (unless strictly necessary for relocation).

    Adding new features or functionality to the application.

    Changing the core build system (CMakeLists.txt) beyond necessary path updates.

4. Target Audience

    All developers contributing to the Cloud Registration project.

    New contributors onboarding to the project.

    Anyone reviewing the project's codebase.

5. Functional Requirements

    FR1: Relocate PowerShell Scripts: All .ps1 files that are test-related or sprint-specific will be moved to a scripts/tests/ subdirectory. General utility scripts (like build-clean.ps1, run-tests.ps1, setup_libe57format.ps1, validate-phase1*.ps1) will remain in scripts/.

    FR2: Relocate Markdown Documentation: All .md files that are sprint summaries, implementation reports, or specific verification reports will be moved to a docs/sprints/ subdirectory. General project documentation (like README.md, build-instructions.md) will remain in docs/.

    FR3: Relocate C++ Test/Demo Files: All .cpp files that are standalone tests or small demonstration programs will be moved to the tests/ directory (if they are unit/integration tests) or a new tests/demos/ subdirectory (if they are simple demos).

    FR4: Update References: All internal file paths and references within the codebase (e.g., in .ps1 scripts, CMakeLists.txt, C++ source files, Markdown links) must be updated to reflect the new file locations.

    FR5: Maintain Functionality: The project must continue to build, run, and pass all existing tests successfully after the reorganization.

6. Non-Functional Requirements

    NFR1: Consistency: The new directory structure must be logical and consistently applied to all relevant file types.

    NFR2: Maintainability: The new structure should simplify future additions of similar files.

    NFR3: Performance: File relocation should not negatively impact build times or application performance.

    NFR4: Backward Compatibility: While file paths change, the overall build and test execution commands should remain as similar as possible (e.g., .\scripts\run-tests.ps1 should still work).

7. Proposed Directory Structure

cloud_registration/
├── CMakeLists.txt
├── README.md
├── src/
│   └── ... (existing source files)
├── shaders/
│   └── ...
├── scripts/
│   ├── build-clean.ps1
│   ├── run-tests-fixed.ps1
│   ├── run-tests.ps1
│   ├── setup_libe57format.ps1
│   ├── validate-phase1-clean.ps1
│   ├── validate-phase1-simple.ps1
│   └── validate-phase1.ps1
│   └── tests/                 <-- NEW: For all .ps1 test scripts
│       ├── run_sprint4_tests.ps1
│       ├── test_app.ps1
│       └── ... (all other .ps1 test scripts)
├── tests/
│   ├── advanced_test_executor.h/cpp
│   ├── advanced_test_file_generator.h/cpp
│   ├── automated_test_oracle.h/cpp
│   ├── E57TestFramework.h/cpp
│   ├── integration_test_suite.h/cpp
│   ├── intelligent_bug_manager.h/cpp
│   ├── PerformanceProfiler.h/cpp
│   ├── spectrum_based_tester.h/cpp
│   ├── test_data_manager.h/cpp
│   ├── test_e57parser_sprint4_comprehensive.cpp
│   ├── test_e57parserlib_sprint3.cpp
│   ├── test_e57parserlib.cpp
│   ├── test_libe57_linkage.cpp
│   ├── test_projectmanager.cpp
│   ├── test_recentprojectsmanager.cpp
│   ├── test_sprint1_2_compressedvector.cpp
│   ├── test_sprint1_2_integration.cpp
│   ├── test_sprint1_4_integration.cpp
│   ├── test_sprint1_functionality.cpp
│   ├── test_sprint2_4_advanced.cpp
│   ├── test_voxelgridfilter.cpp
│   ├── test_e57parser.cpp
│   ├── test_lasparser.cpp
│   └── demos/                 <-- NEW: For .cpp demo/simple test files
│       ├── test_e57_parsing.cpp
│       ├── test_e57_simple.cpp
│       ├── test_las_parser.cpp
│       ├── test_las_real_file.cpp
│       ├── test_sprint1_implementation.cpp
│       ├── test_sprint2_2_profiling_demo.cpp
│       ├── test_sprint2_2_profiling.cpp
│       ├── test_sprint2_simple.cpp
│       ├── test_sprint3_demo.cpp
│       ├── test_voxel_manual.cpp
│       └── test_voxel_simple.cpp
├── docs/
│   ├── build-instructions.md
│   ├── debugging_implementation_summary.md
│   ├── README.md (if separate from root README)
│   ├── sprints/               <-- NEW: For all sprint-specific .md files
│   │   ├── BUILD_ORGANIZATION_SUMMARY.md
│   │   ├── E57_PARSING_FIX_COMPLETE.md
│   │   ├── E57_PARSING_FIX_SUMMARY.md
│   │   ├── E57_PARSING_FIX_VERIFICATION_REPORT.md
│   │   ├── IMPLEMENTATION_COMPLETE.md
│   │   ├── SPRINT_1_1_IMPLEMENTATION_COMPLETE.md
│   │   ├── SPRINT_1_1_IMPLEMENTATION_SUMMARY.md
│   │   ├── SPRINT_1_2_IMPLEMENTATION_SUMMARY.md
│   │   ├── SPRINT_1_3_IMPLEMENTATION_SUMMARY.md
│   │   ├── SPRINT_1_4_FINAL_SUMMARY.md
│   │   ├── SPRINT_1_4_IMPLEMENTATION_SUMMARY.md
│   │   ├── SPRINT_1_IMPLEMENTATION_COMPLETE.md
│   │   ├── SPRINT3_COMPLETION_SUMMARY.md
│   │   ├── SPRINT4_IMPLEMENTATION_SUMMARY.md
│   │   ├── test_sprint1_4.md
│   │   ├── verify_e57_fix.md
│   │   └── verify_sprint1.md
│   └── ... (other general documentation)
└── .gitignore

8. Release Plan (Sprints)

This reorganization will be executed in two sprints to minimize disruption and allow for thorough verification.

Sprint 1: Initial Relocation & Core Updates (1-2 Days)

Objective: Move the majority of files to their new locations and update core build system references.

    Task 1.1: Create New Directories:

        Create scripts/tests/

        Create tests/demos/

        Create docs/sprints/

    Task 1.2: Relocate PowerShell Test Scripts:

        Move all .ps1 files from root to scripts/tests/ except for:

            build-clean.ps1

            run-tests-fixed.ps1

            run-tests.ps1

            setup_libe57format.ps1

            validate-phase1-clean.ps1

            validate-phase1-simple.ps1

            validate-phase1.ps1

    Task 1.3: Relocate Markdown Sprint Documentation:

        Move all .md files from root to docs/sprints/ except for:

            README.md

            build-instructions.md

            debugging_implementation_summary.md (This is a summary of a fix, could go to docs/fixes/ if such a folder exists, but for now, docs/sprints/ is a good general place for sprint-specific docs).

            IMPLEMENTATION_COMPLETE.md (Similar to above, could go to docs/fixes/).

    Task 1.4: Relocate C++ Demo/Simple Test Files:

        Move the following .cpp files from root to tests/demos/:

            test_e57_parsing.cpp

            test_e57_simple.cpp

            test_las_parser.cpp

            test_las_real_file.cpp

            test_sprint1_implementation.cpp

            test_sprint2_2_profiling_demo.cpp

            test_sprint2_2_profiling.cpp

            test_sprint2_simple.cpp

            test_sprint3_demo.cpp

            test_voxel_manual.cpp

            test_voxel_simple.cpp

    Task 1.5: Update CMakeLists.txt:

        Update all add_executable, target_sources, and add_test commands to reflect the new paths for moved .cpp files.

        Ensure scripts/tests/ and tests/demos/ are correctly referenced.

    Task 1.6: Update Core PowerShell Scripts:

        Modify scripts/run-tests.ps1 and scripts/run-tests-fixed.ps1 to call test scripts from scripts/tests/.

        Update any other core scripts in scripts/ that reference moved files.

    Task 1.7: Basic Codebase Scan for References:

        Perform a quick search for hardcoded paths to moved files within src/ and tests/ (e.g., QFile::exists("test_data/...") or QFile::remove("temp_invalid.e57") if these are related to the moved .cpp files). Update as necessary.

    Verification:

        Ensure the project builds successfully.

        Run scripts/build-clean.ps1.

        Run scripts/run-tests.ps1 (or run-tests-fixed.ps1) to confirm all tests still execute and pass.

        Manually check a few moved .md files to ensure they open and links work.

Sprint 2: Internal Reference Refinement & Comprehensive Verification (1-2 Days)

Objective: Address remaining internal references, ensure all documentation links are correct, and perform a final comprehensive verification.

    Task 2.1: Refine Internal Code References:

        Thoroughly search src/, tests/, scripts/, and docs/ for any remaining hardcoded paths or relative references to the original locations of the moved files.

        Update any QFile::exists, QFile::remove, QDir::currentPath(), QFileInfo usage that might be affected by the new structure.

        Pay close attention to test_data/ paths within C++ tests – ensure they are correct relative to the new tests/ or tests/demos/ locations, or are handled dynamically.

    Task 2.2: Update Markdown Links:

        Review all .md files (both in docs/ and docs/sprints/) and update any internal links that point to moved .md or .cpp files.

    Task 2.3: Update README.md:

        Ensure the README.md accurately reflects the new project structure and any updated build/test commands.

    Task 2.4: Comprehensive Test Execution:

        Run all automated tests (scripts/run-tests.ps1 -Coverage -Verbose) to ensure 100% pass rate and no new warnings/errors related to file paths.

        Execute manual testing scenarios (e.g., loading files, creating projects) to ensure core application functionality is intact.

    Task 2.5: Documentation Review:

        Perform a final review of the docs/ directory to ensure all documentation is coherent, up-to-date, and correctly organized.

    Verification:

        All automated tests pass.

        Manual smoke tests confirm core application functionality.

        No broken links in documentation.

        The repository looks clean and organized.

9. Success Metrics

    Root directory contains only essential project files (e.g., CMakeLists.txt, README.md, src/, scripts/, tests/, docs/, .gitignore).

    All .ps1 test scripts are in scripts/tests/.

    All .md sprint-specific documentation is in docs/sprints/.

    All .cpp demo/simple test files are in tests/demos/.

    The project builds successfully without warnings or errors.

    All automated tests pass with 100% success rate.

    No broken internal links in documentation.

    Developer feedback confirms improved repository clarity.