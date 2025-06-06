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

    # Warning level 4 but don't treat warnings as errors during build fixes
    add_compile_options(/W4)

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
find_package(Qt6 6.9.0 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets Test Xml Network Sql Concurrent)

# Find Vulkan SDK
# find_package(Vulkan REQUIRED)

# Find E57Format library for enhanced E57 parsing
find_package(E57Format CONFIG REQUIRED)

# Add compile definitions to disable deprecated APIs up to Qt 5.15
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

# Fix Windows macro conflicts
if(WIN32)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

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
    src/IE57Parser.cpp
    src/lasparser.cpp
    src/loadingsettingsdialog.cpp
    src/voxelgridfilter.cpp
    src/performance_profiler.cpp
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
    src/IE57Parser.h
    src/lasparser.h
    src/loadingsettingsdialog.h
    src/loadingsettings.h
    src/lasheadermetadata.h
    src/voxelgridfilter.h
    src/performance_profiler.h
)

# Shader files
set(SHADERS
    shaders/point.vert
    shaders/point.frag
)

# Resource files
set(RESOURCES
    resources.qrc
)

# Create the executable
add_executable(CloudRegistration ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt6 libraries and E57Format
target_link_libraries(CloudRegistration
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::Xml
    Qt6::Sql
    Qt6::Concurrent
    E57Format
)

# Ensure E57Format include directories are available
get_target_property(E57_INCLUDE_DIRS E57Format INTERFACE_INCLUDE_DIRECTORIES)
if(E57_INCLUDE_DIRS)
    target_include_directories(CloudRegistration PRIVATE ${E57_INCLUDE_DIRS})
endif()



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

    # E57 Parser Tests
    add_executable(E57ParserTests
        tests/test_e57parser.cpp
        src/e57parserlib.cpp
        src/IE57Parser.cpp
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

    # Project Management Tests
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
        DEPENDS E57ParserTests LasParserTests VoxelGridFilterTests ProjectManagerTests RecentProjectsManagerTests
        COMMENT "Running all unit tests"
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
                DEPENDS E57ParserTests LasParserTests VoxelGridFilterTests ProjectManagerTests RecentProjectsManagerTests
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
