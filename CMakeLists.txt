# Sprint 1: Project Skeleton and Build System Foundation - Modular CMake Structure
cmake_minimum_required(VERSION 3.16)

# F1-S2-01: Set Qt installation path to use existing Qt 6.9.0
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64" ${CMAKE_PREFIX_PATH})

# F1-S2-01: Crucial - Integrate vcpkg toolchain for non-Qt dependencies only
# Set CMAKE_TOOLCHAIN_FILE to vcpkg.cmake path
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON)

project(CloudRegistration VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# F1-S2-04: Enable Qt's Meta-Object Compiler (MOC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configure MSVC
if(MSVC)
    # Warning level 4
    add_compile_options(/W4)
    # UTF-8 encoding
    add_compile_options(/utf-8)
endif()

# Configure GCC/Clang compilers (Linux/macOS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # UTF-8 encoding
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    # High warning level
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# F1-S2-03: Find and link Qt6 components (Sprint 3: Added Sql for database support, Sprint 6: Added PrintSupport for PDF generation)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets Test Sql PrintSupport)

# Sprint 2: Task 4.2 - Find third-party libraries (optional for Sprint 1 scaffolding)
find_package(Eigen3 CONFIG QUIET)
find_package(E57Format CONFIG QUIET)
find_package(XercesC QUIET)

# Set flags for optional dependencies
set(EIGEN3_AVAILABLE ${Eigen3_FOUND})
set(E57FORMAT_AVAILABLE ${E57Format_FOUND})
set(XERCES_AVAILABLE ${XercesC_FOUND})

if(EIGEN3_AVAILABLE)
    message(STATUS "Eigen3 found - algorithms will be fully functional")
else()
    message(WARNING "Eigen3 not found - algorithms library will be placeholder only")
endif()

if(E57FORMAT_AVAILABLE)
    message(STATUS "E57Format found - E57 parsing will be fully functional")
else()
    message(WARNING "E57Format not found - parsers library will be placeholder only")
endif()

if(XERCES_AVAILABLE)
    message(STATUS "XercesC found - XML parsing will be fully functional")
else()
    message(WARNING "XercesC not found - XML parsing will be placeholder only")
endif()

# Sprint 3: Task 1.1 - Enable testing in CMake
enable_testing()

# Sprint 3: Task 2.3 - Find Google Test framework (optional for Sprint 1)
find_package(GTest CONFIG QUIET)

# Sprint 3: Task 2.4 - Confirm GTest and GMock were found
if(TARGET GTest::gtest_main AND TARGET GTest::gmock_main)
    message(STATUS "Google Test and Google Mock found.")
    set(GMOCK_AVAILABLE TRUE)
    set(GTEST_AVAILABLE TRUE)
elseif(TARGET GTest::gtest_main)
    message(STATUS "Google Test found (without GMock).")
    set(GMOCK_AVAILABLE FALSE)
    set(GTEST_AVAILABLE TRUE)
else()
    message(WARNING "Google Test not found - tests will be placeholder only.")
    set(GMOCK_AVAILABLE FALSE)
    set(GTEST_AVAILABLE FALSE)
endif()

# ============================================================================
# CLANG-FORMAT AND CLANG-TIDY INTEGRATION
# ============================================================================

# Find clang-format executable
find_program(CLANG_FORMAT_EXE clang-format)

if(CLANG_FORMAT_EXE)
    message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")

    # Define files to format (all C++ source and header files)
    file(GLOB_RECURSE PROJECT_FORMAT_FILES
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/src/*.hpp"
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.cxx"
        "${CMAKE_SOURCE_DIR}/src/*.cc"
        "${CMAKE_SOURCE_DIR}/tests/*.h"
        "${CMAKE_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_SOURCE_DIR}/tests/*.cxx"
        "${CMAKE_SOURCE_DIR}/tests/*.cc"
    )

    # Create format_all target
    add_custom_target(format_all
        COMMAND ${CLANG_FORMAT_EXE} -i ${PROJECT_FORMAT_FILES}
        COMMENT "Formatting all C++ source files with clang-format"
        VERBATIM
    )

    # Create format_check target for CI/CD (dry-run)
    add_custom_target(format_check
        COMMAND ${CLANG_FORMAT_EXE} --Werror --dry-run ${PROJECT_FORMAT_FILES}
        COMMENT "Checking C++ source files formatting with clang-format"
        VERBATIM
    )
else()
    message(WARNING "clang-format not found - formatting targets will not be available")
endif()

# Enable compilation database generation for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find clang-tidy executable
find_program(CLANG_TIDY_EXE clang-tidy)

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

    # Create run_clang_tidy target
    add_custom_target(run_clang_tidy
        COMMAND ${CLANG_TIDY_EXE} ${PROJECT_FORMAT_FILES} -- -p=${CMAKE_BINARY_DIR} -std=c++17
        COMMENT "Running clang-tidy analysis on C++ source files"
        VERBATIM
    )

    # Ensure the main application is built first to generate up-to-date compile_commands.json
    if(TARGET CloudRegistration)
        add_dependencies(run_clang_tidy CloudRegistration)
    endif()
else()
    message(WARNING "clang-tidy not found - static analysis targets will not be available")
endif()

# Sprint 1: Add subdirectories for all top-level logical components
# Order is important for dependencies
add_subdirectory(src/core)
add_subdirectory(src/algorithms)
add_subdirectory(src/analysis)
add_subdirectory(src/crs)
# src/detection moved to src/registration in Sprint 6
add_subdirectory(src/export)
add_subdirectory(src/features)
add_subdirectory(src/implementations)
add_subdirectory(src/interfaces)
add_subdirectory(src/optimization)
add_subdirectory(src/parsers)
add_subdirectory(src/performance)
add_subdirectory(src/quality)
add_subdirectory(src/registration)
add_subdirectory(src/rendering)
add_subdirectory(src/ui)
add_subdirectory(src/app) # The main application executable

# Add test subdirectories
add_subdirectory(tests) # This will in turn add other test subdirectories

# Packaging integration (CPack)
include(CPack)

# ============================================================================
# Sprint 1: MODULAR LIBRARY ARCHITECTURE FOUNDATION
# ============================================================================
#
# SPRINT 1 IMPLEMENTATION SUMMARY:
# ================================
# ✅ Task 1: Created feature/modular-restructure Git branch
# ✅ Task 2: Created complete directory structure for all modules
# ✅ Task 3: Created new root CMakeLists.txt with modular add_subdirectory() calls
# ✅ Task 4: Created placeholder CMakeLists.txt files for all modules
# ✅ Task 5: Created placeholder CMakeLists.txt files for tests
#
# DIRECTORY STRUCTURE CREATED:
# ===========================
# src/core/include/core/ and src/core/src/
# src/algorithms/include/algorithms/ and src/algorithms/src/
# src/analysis/include/analysis/ and src/analysis/src/
# src/camera/ - MOVED TO src/rendering/ in Sprint 5
# src/crs/include/crs/ and src/crs/src/
# src/detection/include/detection/ and src/detection/src/
# src/export/include/export/ and src/export/src/
# src/features/include/features/ and src/features/src/
# src/implementations/include/implementations/ and src/implementations/src/
# src/interfaces/include/interfaces/ and src/interfaces/src/
# src/optimization/include/optimization/ and src/optimization/src/
# src/parsers/include/parsers/ and src/parsers/src/
# src/performance/include/performance/ and src/performance/src/
# src/quality/include/quality/ and src/quality/src/
# src/registration/include/registration/ and src/registration/src/
# src/rendering/include/rendering/ and src/rendering/src/
# src/ui/include/ui/ and src/ui/src/
# src/app/
# tests/[module_name]/ for all modules
#
# NEXT STEPS (Subsequent Sprints):
# ================================
# Sprint 2-6: Implement individual module CMakeLists.txt files
# Sprint 7: Application Integration
# Sprint 8: Test Suite Integration
#
# SPRINT 1 STATUS: ✅ COMPLETED SUCCESSFULLY
# ==========================================
# The project skeleton and build system foundation is now established.
# All directory structures and placeholder CMakeLists.txt files are in place.
# The build system is ready for modular development in subsequent sprints.
