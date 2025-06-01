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
find_package(Qt6 6.9.0 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets Test Xml)

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
    src/pointcloudviewerwidget.cpp
    src/e57parser.cpp
    src/lasparser.cpp
    src/loadingsettingsdialog.cpp
    src/voxelgridfilter.cpp
    src/performance_profiler.cpp
    src/performance_benchmark.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/pointcloudviewerwidget.h
    src/e57parser.h
    src/lasparser.h
    src/loadingsettingsdialog.h
    src/loadingsettings.h
    src/lasheadermetadata.h
    src/voxelgridfilter.h
    src/performance_profiler.h
    src/performance_benchmark.h
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
        src/e57parser.cpp
        src/lasparser.cpp
        src/voxelgridfilter.cpp
        src/performance_profiler.cpp
    )

    # E57 Parser Tests
    add_executable(E57ParserTests
        tests/test_e57parser.cpp
        src/e57parser.cpp
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

    # Sprint 2.1 Manual Test
    add_executable(Sprint21ManualTest
        test_sprint2_1_manual.cpp
        src/e57parser.cpp
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
        test_sprint2_1_integration.cpp
        src/e57parser.cpp
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
        src/e57parser.cpp
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

    # Sprint 1.2 Integration Tests (without E57Format dependency)
    add_executable(Sprint12IntegrationTests
        tests/test_sprint1_2_integration.cpp
        src/e57parser.cpp
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
        test_las_real_file.cpp
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
        test_sprint2_2_profiling_demo.cpp
        src/e57parser.cpp
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
        src/e57parser.cpp
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
        src/e57parser.cpp
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

    # Custom target to run all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS E57ParserTests LasParserTests VoxelGridFilterTests Sprint1FunctionalityTests Sprint14IntegrationTests Sprint24AdvancedTests
        COMMENT "Running all unit tests including Sprint 2.4 advanced testing framework"
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