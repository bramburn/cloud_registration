cmake_minimum_required(VERSION 3.16)

# Enable IDE folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(CloudRegistration VERSION 1.0.0 LANGUAGES CXX)

# Configure MSVC
if(MSVC)
    # Use static runtime libraries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Enable multi-processor compilation
    add_compile_options(/MP)

    # Enable enhanced instruction sets
    add_compile_options(/arch:AVX2)

    # Warning level 4 and treat warnings as errors
    add_compile_options(/W4 /WX)

    # Enable debug information in all builds
    add_compile_options(/Zi)

    # User Story 3: Fix encoding issues - enforce UTF-8 for source and execution
    add_compile_options(/utf-8)
endif()

# User Story 3: Fix encoding issues for GCC/Clang compilers
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)

# Set Qt6 path if installed in custom location
if(WIN32 AND NOT DEFINED Qt6_DIR)
    # First check environment variable
    if(DEFINED ENV{Qt6_DIR} AND EXISTS $ENV{Qt6_DIR})
        set(Qt6_DIR $ENV{Qt6_DIR})
        message(STATUS "Using Qt6 from environment variable: ${Qt6_DIR}")
    else()
        # Try common Qt6 installation paths - prioritize Qt 6.9.0
        set(QT_POSSIBLE_PATHS
            "C:/Qt/6.9.0/msvc2022_64/lib/cmake/Qt6"
            "C:/Qt/6.8.0/msvc2022_64/lib/cmake/Qt6"
            "C:/Qt/6.7.0/msvc2022_64/lib/cmake/Qt6"
            "C:/Qt/6.6.0/msvc2022_64/lib/cmake/Qt6"
            "C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6"
            "C:/Qt/6.5.0/msvc2019_64/lib/cmake/Qt6"
            "C:/Qt/6.5.0/msvc2022_64/lib/cmake/Qt6"
            "$ENV{PROGRAMFILES}/Qt/6.9.0/msvc2022_64/lib/cmake/Qt6"
            "$ENV{PROGRAMFILES}/Qt/6.8.0/msvc2022_64/lib/cmake/Qt6"
            "$ENV{PROGRAMFILES}/Qt/6.7.0/msvc2022_64/lib/cmake/Qt6"
            "$ENV{PROGRAMFILES}/Qt/6.6.0/msvc2022_64/lib/cmake/Qt6"
        )

        foreach(QT_PATH ${QT_POSSIBLE_PATHS})
            if(EXISTS ${QT_PATH})
                set(Qt6_DIR ${QT_PATH})
                message(STATUS "Found Qt6 at: ${Qt6_DIR}")
                break()
            endif()
        endforeach()

        if(NOT DEFINED Qt6_DIR)
            message(WARNING "Qt6 not found in common locations. Please set Qt6_DIR manually.")
        endif()
    endif()
endif()

# Find Qt6 components - require minimum version 6.9.0
find_package(Qt6 6.9.0 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets Test Xml Network Sql)

# Find E57Format library for enhanced E57 parsing (Sprint 1.2)
find_package(E57Format CONFIG REQUIRED)

# Add compile definitions to disable deprecated APIs up to Qt 5.15
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

# Enable Qt's MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add Qt bin directory to PATH for DLL loading
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(ENV{PATH} "$ENV{PATH};${QT_BIN_DIR}")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/projecthubwidget.cpp
    src/createprojectdialog.cpp
    src/projectmanager.cpp
    src/recentprojectsmanager.cpp
    src/sidebarwidget.cpp
    src/project.cpp
    src/pointcloudviewerwidget.cpp
    src/e57parserlib.cpp
    src/lasparser.cpp
    src/loadingsettingsdialog.cpp
    src/voxelgridfilter.cpp
    src/performance_profiler.cpp
    src/performance_benchmark.cpp
    # Sprint 1.2: Scan Import & SQLite Database Setup
    src/sqlitemanager.cpp
    src/scanimportmanager.cpp
    src/scanimportdialog.cpp
    src/projecttreemodel.cpp
    # Sprint 2.1: Point Cloud Load Management
    src/pointcloudloadmanager.cpp
    # Sprint W1: E57 Writing - Basic E57 Structure Writing
    src/e57writer_lib.cpp
    # Sprint 2.3: Cluster Locking & Enhanced Context Menus
    src/confirmationdialog.cpp
    # Sprint 3.1: Robust Data Persistence & Error Handling
    src/errordialog.cpp
    src/relinkfiledialog.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/projecthubwidget.h
    src/createprojectdialog.h
    src/projectmanager.h
    src/recentprojectsmanager.h
    src/sidebarwidget.h
    src/project.h
    src/pointcloudviewerwidget.h
    src/e57parserlib.h
    src/lasparser.h
    src/loadingsettingsdialog.h
    src/loadingsettings.h
    src/lasheadermetadata.h
    src/voxelgridfilter.h
    src/performance_profiler.h
    src/performance_benchmark.h
    # Sprint 1.2: Scan Import & SQLite Database Setup
    src/sqlitemanager.h
    src/scanimportmanager.h
    src/scanimportdialog.h
    src/projecttreemodel.h
    # Sprint 2.1: Point Cloud Load Management
    src/pointcloudloadmanager.h
    # Sprint W1: E57 Writing - Basic E57 Structure Writing
    src/e57writer_lib.h
    # Sprint 2.3: Cluster Locking & Enhanced Context Menus
    src/confirmationdialog.h
    # Sprint 3.1: Robust Data Persistence & Error Handling
    src/errordialog.h
    src/relinkfiledialog.h
)

# Shader files
set(SHADERS
    shaders/point.vert
    shaders/point.frag
)

# Create the executable
add_executable(CloudRegistration ${SOURCES} ${HEADERS})

# Link Qt6 libraries and E57Format
target_link_libraries(CloudRegistration
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::Xml
    Qt6::Sql
    E57Format
)



# Copy shader files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Set output directory
set_target_properties(CloudRegistration PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest CONFIG QUIET)
if(TARGET GTest::gtest_main)
    message(STATUS "Google Test found - building unit tests")

    # Enable code coverage if requested
    option(ENABLE_COVERAGE "Enable code coverage" OFF)
    if(ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            message(STATUS "Code coverage enabled")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        else()
            message(WARNING "Code coverage only supported with GCC or Clang")
        endif()
    endif()

    # Common test sources (excluding main.cpp)
    set(TEST_COMMON_SOURCES
        src/e57parserlib.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    # E57 Parser Tests (now using E57ParserLib)
    add_executable(E57ParserTests
        tests/test_e57parser.cpp
        src/e57parserlib.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(E57ParserTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Xml
        Qt6::Test
        E57Format
    )

    target_include_directories(E57ParserTests PRIVATE src)
    add_test(NAME E57ParserTests COMMAND E57ParserTests)

    # E57ParserLib Tests (Sprint 1)
    add_executable(E57ParserLibTests
        tests/test_e57parserlib.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(E57ParserLibTests
        GTest::gtest_main
        E57Format
        Qt6::Core
        Qt6::Test
    )

    target_include_directories(E57ParserLibTests PRIVATE src)
    add_test(NAME E57ParserLibTests COMMAND E57ParserLibTests)

    # Sprint 4: Comprehensive E57 Test Suite
    add_executable(E57Sprint4ComprehensiveTests
        tests/test_e57parser_sprint4_comprehensive.cpp
        tests/E57TestFramework.cpp
        tests/PerformanceProfiler.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(E57Sprint4ComprehensiveTests
        GTest::gtest_main
        E57Format
        Qt6::Core
        Qt6::Test
    )

    target_include_directories(E57Sprint4ComprehensiveTests PRIVATE src tests)
    add_test(NAME E57Sprint4ComprehensiveTests COMMAND E57Sprint4ComprehensiveTests)

    # Sprint 4: Performance Benchmarking Target
    add_custom_target(benchmark_e57_sprint4
        COMMAND E57Sprint4ComprehensiveTests --gtest_filter="*Performance*"
        DEPENDS E57Sprint4ComprehensiveTests
        COMMENT "Running E57 Sprint 4 performance benchmarks"
    )

    # Sprint 4: Comprehensive Testing Target
    add_custom_target(test_e57_comprehensive
        COMMAND E57Sprint4ComprehensiveTests
        DEPENDS E57Sprint4ComprehensiveTests
        COMMENT "Running E57 Sprint 4 comprehensive test suite"
    )

    # E57ParserLib Sprint 3 Tests (Intensity, Color, and CompressedVector Handling)
    add_executable(E57ParserLibSprint3Tests
        tests/test_e57parserlib_sprint3.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(E57ParserLibSprint3Tests
        GTest::gtest_main
        Qt6::Core
        E57Format
    )

    target_include_directories(E57ParserLibSprint3Tests PRIVATE src)
    add_test(NAME E57ParserLibSprint3Tests COMMAND E57ParserLibSprint3Tests)

    # Sprint W1: E57 Writer Library Tests
    add_executable(E57WriterLibTests
        tests/test_e57writer_lib.cpp
        src/e57writer_lib.cpp
    )

    target_link_libraries(E57WriterLibTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
        E57Format
    )

    target_include_directories(E57WriterLibTests PRIVATE src)
    add_test(NAME E57WriterLibTests COMMAND E57WriterLibTests)

    # E57 Writer Debug Test
    add_executable(E57WriterDebugTest
        test_e57_debug.cpp
        src/e57writer_lib.cpp
    )

    target_link_libraries(E57WriterDebugTest
        Qt6::Core
        E57Format
    )

    target_include_directories(E57WriterDebugTest PRIVATE src)

    # E57 Writer Simple Test (for debugging hanging issues)
    add_executable(E57WriterSimpleTest
        test_e57_simple.cpp
        src/e57writer_lib.cpp
    )

    target_link_libraries(E57WriterSimpleTest
        Qt6::Core
        E57Format
    )

    target_include_directories(E57WriterSimpleTest PRIVATE src)

    # E57 Minimal Test (for debugging libE57Format issues)
    add_executable(E57MinimalTest
        test_e57_minimal.cpp
    )

    target_link_libraries(E57MinimalTest
        E57Format
    )

    # E57 No Signals Test (for debugging Qt signals issues)
    add_executable(E57NoSignalsTest
        test_e57_nosignals.cpp
    )

    target_link_libraries(E57NoSignalsTest
        Qt6::Core
        E57Format
    )

    # E57 Pure C++ Test (no Qt dependencies)
    add_executable(E57PureTest
        test_e57_pure.cpp
    )

    target_link_libraries(E57PureTest
        E57Format
    )

    # E57WriterLib No-Qt Test (fixes hanging issues)
    add_executable(E57WriterNoQtTest
        test_e57writer_noqt.cpp
        src/e57writer_lib_noqt.cpp
    )

    target_link_libraries(E57WriterNoQtTest
        E57Format
    )

    target_include_directories(E57WriterNoQtTest PRIVATE src)

    # Sprint 3 Demo Program
    add_executable(Sprint3Demo
        tests/demos/test_sprint3_demo.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(Sprint3Demo
        Qt6::Core
        E57Format
    )

    target_include_directories(Sprint3Demo PRIVATE src)

    # Sprint 2 Simple Test
    add_executable(Sprint2SimpleTest
        tests/demos/test_sprint2_simple.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(Sprint2SimpleTest
        E57Format
        Qt6::Core
    )

    target_include_directories(Sprint2SimpleTest PRIVATE src)

    # libE57Format Linkage Test (Sprint 1)
    add_executable(LibE57LinkageTest
        tests/test_libe57_linkage.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(LibE57LinkageTest
        E57Format
    )

    target_include_directories(LibE57LinkageTest PRIVATE src)

    # Sprint 1 Manual Verification Test
    add_executable(Sprint1VerificationTest
        tests/demos/test_sprint1_implementation.cpp
        src/e57parserlib.cpp
    )

    target_link_libraries(Sprint1VerificationTest
        E57Format
    )

    target_include_directories(Sprint1VerificationTest PRIVATE src)

    # Sprint 2.1 Manual Test
    add_executable(Sprint21ManualTest
        tests/demos/test_sprint2_1_manual.cpp
        src/e57parserlib.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint21ManualTest
        Qt6::Core
        Qt6::Xml
        E57Format
    )

    target_include_directories(Sprint21ManualTest PRIVATE src)

    # Sprint 2.1 Integration Test
    add_executable(Sprint21IntegrationTest
        tests/demos/test_sprint2_1_integration.cpp
        src/e57parserlib.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint21IntegrationTest
        Qt6::Core
        Qt6::Xml
        Qt6::Test
        E57Format
    )

    target_include_directories(Sprint21IntegrationTest PRIVATE src)
    add_test(NAME Sprint21IntegrationTest COMMAND Sprint21IntegrationTest)

    # LAS Parser Tests
    add_executable(LasParserTests
        tests/test_lasparser.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(LasParserTests
        GTest::gtest_main
        Qt6::Core
    )

    target_include_directories(LasParserTests PRIVATE src)
    add_test(NAME LasParserTests COMMAND LasParserTests)

    # VoxelGridFilter Tests
    add_executable(VoxelGridFilterTests
        tests/test_voxelgridfilter.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(VoxelGridFilterTests
        GTest::gtest_main
        Qt6::Core
    )

    target_include_directories(VoxelGridFilterTests PRIVATE src)
    add_test(NAME VoxelGridFilterTests COMMAND VoxelGridFilterTests)

    # Sprint 2.3: Cluster Locking Tests
    add_executable(Sprint23ClusterLockingTests
        tests/test_sprint2_3_cluster_locking.cpp
        src/sqlitemanager.cpp
        src/projectmanager.cpp
        src/confirmationdialog.cpp
        src/project.cpp
        src/scanimportmanager.cpp
        src/scanimportdialog.cpp
    )

    target_link_libraries(Sprint23ClusterLockingTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Test
    )

    target_include_directories(Sprint23ClusterLockingTests PRIVATE src)
    add_test(NAME Sprint23ClusterLockingTests COMMAND Sprint23ClusterLockingTests)

    # Sprint 1 Functionality Tests
    add_executable(Sprint1FunctionalityTests
        tests/test_sprint1_functionality.cpp
        src/loadingsettingsdialog.cpp
        ${TEST_COMMON_SOURCES}
    )

    target_link_libraries(Sprint1FunctionalityTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::Xml
        Qt6::Test
        E57Format
    )

    target_include_directories(Sprint1FunctionalityTests PRIVATE src)
    add_test(NAME Sprint1FunctionalityTests COMMAND Sprint1FunctionalityTests)

    # Sprint 1.2 CompressedVector Tests (without E57Format dependency)
    add_executable(Sprint12CompressedVectorTests
        tests/test_sprint1_2_compressedvector.cpp
        src/e57parserlib.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint12CompressedVectorTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Xml
        Qt6::Test
    )

    target_include_directories(Sprint12CompressedVectorTests PRIVATE src)
    add_test(NAME Sprint12CompressedVectorTests COMMAND Sprint12CompressedVectorTests)

    # Sprint 3.1: Robust Data Persistence & Error Handling Tests
    add_executable(Sprint31PersistenceTests
        tests/test_sprint31_persistence.cpp
        src/projectmanager.cpp
        src/sqlitemanager.cpp
        src/projecttreemodel.cpp
        src/errordialog.cpp
        src/relinkfiledialog.cpp
        src/project.cpp
        src/scanimportmanager.cpp
        src/scanimportdialog.cpp
        src/confirmationdialog.cpp
    )

    target_link_libraries(Sprint31PersistenceTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Test
    )

    target_include_directories(Sprint31PersistenceTests PRIVATE src)
    add_test(NAME Sprint31PersistenceTests COMMAND Sprint31PersistenceTests)

    # Sprint 1.2 Integration Tests (without E57Format dependency)
    add_executable(Sprint12IntegrationTests
        tests/test_sprint1_2_integration.cpp
        src/e57parserlib.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint12IntegrationTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Xml
        Qt6::Test
    )

    target_include_directories(Sprint12IntegrationTests PRIVATE src)
    add_test(NAME Sprint12IntegrationTests COMMAND Sprint12IntegrationTests)

    # Simple LAS real file test (no GTest dependency)
    add_executable(TestLasRealFile
        tests/demos/test_las_real_file.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(TestLasRealFile
        Qt6::Core
    )

    target_include_directories(TestLasRealFile PRIVATE src)

    # Sprint 2.2 Profiling Demonstration
    add_executable(Sprint22ProfilingDemo
        tests/demos/test_sprint2_2_profiling_demo.cpp
        src/e57parserlib.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
        src/performance_benchmark.cpp
    )

    target_link_libraries(Sprint22ProfilingDemo
        Qt6::Core
        Qt6::Xml
        E57Format
    )

    target_include_directories(Sprint22ProfilingDemo PRIVATE src)

    # Note: AllTests target removed due to multiple main() conflicts
    # Individual test executables work fine

    # Sprint 1.4: Comprehensive Integration Testing Framework
    add_executable(Sprint14IntegrationTests
        tests/test_sprint1_4_integration.cpp
        tests/test_reporter.cpp
        tests/integration_test_suite.cpp
        src/e57parserlib.cpp
        src/lasparser.cpp
        src/loadingsettingsdialog.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint14IntegrationTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::Xml
        Qt6::Test
        E57Format
    )

    target_include_directories(Sprint14IntegrationTests PRIVATE src)
    add_test(NAME Sprint14IntegrationTests COMMAND Sprint14IntegrationTests)

    # Sprint 2.4: Advanced Testing Framework
    add_executable(Sprint24AdvancedTests
        tests/test_sprint2_4_advanced.cpp
        tests/advanced_test_executor.cpp
        tests/advanced_test_file_generator.cpp
        tests/automated_test_oracle.cpp
        tests/spectrum_based_tester.cpp
        tests/intelligent_bug_manager.cpp
        tests/test_data_manager.cpp
        src/e57parserlib.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(Sprint24AdvancedTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Widgets
        Qt6::Xml
        Qt6::Test
        Qt6::Network
        E57Format
    )

    target_include_directories(Sprint24AdvancedTests PRIVATE src tests)
    add_test(NAME Sprint24AdvancedTests COMMAND Sprint24AdvancedTests)

    # Sprint 1.1: Project Management Tests
    add_executable(ProjectManagerTests
        tests/test_projectmanager.cpp
        src/projectmanager.cpp
        src/project.cpp
    )

    target_link_libraries(ProjectManagerTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
    )

    target_include_directories(ProjectManagerTests PRIVATE src)
    add_test(NAME ProjectManagerTests COMMAND ProjectManagerTests)

    add_executable(RecentProjectsManagerTests
        tests/test_recentprojectsmanager.cpp
        src/recentprojectsmanager.cpp
    )

    target_link_libraries(RecentProjectsManagerTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
    )

    target_include_directories(RecentProjectsManagerTests PRIVATE src)
    add_test(NAME RecentProjectsManagerTests COMMAND RecentProjectsManagerTests)

    # Custom target to run all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS E57ParserTests E57ParserLibTests E57WriterLibTests LasParserTests VoxelGridFilterTests Sprint1FunctionalityTests Sprint14IntegrationTests Sprint24AdvancedTests ProjectManagerTests RecentProjectsManagerTests
        COMMENT "Running all unit tests including Sprint W1 E57WriterLib, Sprint 1.1 project management, Sprint 1 E57ParserLib, and Sprint 2.4 advanced testing framework"
)

# Sprint 1: E57ParserLib testing targets
add_custom_target(run_sprint1_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ".*E57ParserLib.*"
        DEPENDS E57ParserLibTests
        COMMENT "Running Sprint 1 E57ParserLib tests"
    )

    # Sprint W1: E57WriterLib testing targets
    add_custom_target(run_sprintw1_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ".*E57WriterLib.*"
        DEPENDS E57WriterLibTests
        COMMENT "Running Sprint W1 E57WriterLib tests"
    )

    # Sprint 2.4: Advanced testing targets
    add_custom_target(run_advanced_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R ".*Sprint24.*"
        DEPENDS Sprint24AdvancedTests
        COMMENT "Running Sprint 2.4 advanced testing suite"
    )

    add_custom_target(generate_complex_test_files
        COMMAND Sprint24AdvancedTests --generate-test-files
        DEPENDS Sprint24AdvancedTests
        COMMENT "Generating complex test files for advanced testing"
    )

    add_custom_target(run_stress_tests
        COMMAND Sprint24AdvancedTests --stress-test
        DEPENDS Sprint24AdvancedTests
        COMMENT "Running stress tests with large files"
    )

    # Code coverage target (if enabled)
    if(ENABLE_COVERAGE)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --directory . --zerocounters
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
                COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage.info.cleaned
                COMMAND ${GENHTML_PATH} -o coverage_html coverage.info.cleaned
                COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage.info.cleaned
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                DEPENDS E57ParserTests LasParserTests VoxelGridFilterTests Sprint1FunctionalityTests
                COMMENT "Generating code coverage report"
            )
            message(STATUS "Code coverage target 'coverage' available")
        else()
            message(WARNING "lcov and/or genhtml not found - coverage target not available")
        endif()
    endif()

else()
    message(STATUS "Google Test not found - unit tests will not be built")
    message(STATUS "To install Google Test on Ubuntu/Debian: sudo apt-get install libgtest-dev")
    message(STATUS "To install Google Test on Windows: vcpkg install gtest")
endif()