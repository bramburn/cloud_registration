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

# Configure GCC/Clang compilers (Linux/macOS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # User Story 3: Fix encoding issues - enforce UTF-8 for source and execution
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

    # Enable high warning level
    add_compile_options(-Wall -Wextra -Wpedantic)

    # Enable debug information in all builds
    add_compile_options(-g)

    # Enable optimizations for Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
        # Enable modern CPU optimizations if supported
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
        if(COMPILER_SUPPORTS_MARCH_NATIVE)
            add_compile_options(-march=native)
        endif()
    endif()

    # Enable additional security features on Linux
    if(UNIX AND NOT APPLE)
        add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)
        add_link_options(-Wl,-z,relro -Wl,-z,now)
    endif()
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
if(NOT DEFINED Qt6_DIR)
    # First check environment variable
    if(DEFINED ENV{Qt6_DIR} AND EXISTS $ENV{Qt6_DIR})
        set(Qt6_DIR $ENV{Qt6_DIR})
        message(STATUS "Using Qt6 from environment variable: ${Qt6_DIR}")
    else()
        # Platform-specific Qt6 path detection
        if(WIN32)
            # Try common Qt6 installation paths on Windows - prioritize Qt 6.9.0
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
        elseif(UNIX AND NOT APPLE)
            # Try common Qt6 installation paths on Linux
            set(QT_POSSIBLE_PATHS
                "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
                "/usr/lib64/cmake/Qt6"
                "/usr/local/lib/cmake/Qt6"
                "/opt/Qt/6.9.0/gcc_64/lib/cmake/Qt6"
                "/opt/Qt/6.8.0/gcc_64/lib/cmake/Qt6"
                "/opt/Qt/6.7.0/gcc_64/lib/cmake/Qt6"
                "/opt/Qt/6.6.0/gcc_64/lib/cmake/Qt6"
                "/opt/Qt/6.5.3/gcc_64/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.9.0/gcc_64/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.8.0/gcc_64/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.7.0/gcc_64/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.6.0/gcc_64/lib/cmake/Qt6"
            )
        elseif(APPLE)
            # Try common Qt6 installation paths on macOS
            set(QT_POSSIBLE_PATHS
                "/usr/local/opt/qt6/lib/cmake/Qt6"
                "/opt/homebrew/opt/qt6/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.9.0/macos/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.8.0/macos/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.7.0/macos/lib/cmake/Qt6"
                "$ENV{HOME}/Qt/6.6.0/macos/lib/cmake/Qt6"
            )
        endif()

        foreach(QT_PATH ${QT_POSSIBLE_PATHS})
            if(EXISTS ${QT_PATH})
                set(Qt6_DIR ${QT_PATH})
                message(STATUS "Found Qt6 at: ${Qt6_DIR}")
                break()
            endif()
        endforeach()

        if(NOT DEFINED Qt6_DIR)
            message(WARNING "Qt6 not found in common locations. Please set Qt6_DIR manually or install Qt6.")
            if(UNIX AND NOT APPLE)
                message(STATUS "On Ubuntu/Debian: sudo apt-get install qt6-base-dev")
                message(STATUS "On Fedora/RHEL: sudo dnf install qt6-qtbase-devel")
                message(STATUS "On Arch: sudo pacman -S qt6-base")
            elseif(APPLE)
                message(STATUS "On macOS: brew install qt6")
            endif()
        endif()
    endif()
endif()

# Find Qt6 components - require minimum version 6.5.0 for better Linux compatibility
# Note: 6.9.0 is preferred on Windows, but 6.5.0+ is acceptable for Linux distributions
find_package(Qt6 6.5.0 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets Test Xml Network Sql Concurrent)

# Check Qt6 version and warn if using older version
if(Qt6_VERSION VERSION_LESS "6.9.0")
    message(WARNING "Qt6 version ${Qt6_VERSION} detected. Version 6.9.0+ is recommended for optimal performance.")
    message(STATUS "Current version should work but some features may be limited.")
endif()

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
    src/MainPresenter.cpp
    src/lasparser.cpp
    src/loadingsettingsdialog.cpp
    src/voxelgridfilter.cpp
    src/performance_profiler.cpp
    src/progressmanager.cpp
    src/pointcloudloadmanager.cpp
    src/screenspaceerror.cpp
    src/sqlitemanager.cpp
    src/projecttreemodel.cpp
    src/scanimportmanager.cpp
    src/E57ParserCore.cpp
    src/ProjectStateService.cpp
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
    src/IPointCloudViewer.h
    src/e57parserlib.h
    src/IE57Parser.h
    src/IE57Writer.h
    src/IMainView.h
    src/MainPresenter.h
    src/lasparser.h
    src/loadingsettingsdialog.h
    src/loadingsettings.h
    src/lasheadermetadata.h
    src/voxelgridfilter.h
    src/performance_profiler.h
    src/progressmanager.h
    src/pointcloudloadmanager.h
    src/screenspaceerror.h
    src/sqlitemanager.h
    src/projecttreemodel.h
    src/scanimportmanager.h
    src/E57ParserCore.h
    src/ProjectStateService.h
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

# Platform-specific linking
if(UNIX AND NOT APPLE)
    # Link additional libraries required on Linux
    target_link_libraries(CloudRegistration
        pthread
        dl
    )

    # Set RPATH for better library loading on Linux
    set_target_properties(CloudRegistration PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib"
    )
elseif(APPLE)
    # macOS-specific linking
    target_link_libraries(CloudRegistration
        "-framework OpenGL"
        "-framework Cocoa"
    )

    # Set RPATH for macOS
    set_target_properties(CloudRegistration PROPERTIES
        INSTALL_RPATH "@executable_path;@executable_path/../lib"
        BUILD_WITH_INSTALL_RPATH FALSE
    )
endif()

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

# Find Google Test and Google Mock
find_package(GTest CONFIG QUIET)
if(TARGET GTest::gtest_main)
    message(STATUS "Google Test found - building unit tests")

    # Check if Google Mock is available
    if(TARGET GTest::gmock_main)
        message(STATUS "Google Mock found - enabling mock-based tests")
        set(GMOCK_AVAILABLE TRUE)
    else()
        message(WARNING "Google Mock not found - some tests will be disabled")
        set(GMOCK_AVAILABLE FALSE)
    endif()

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

    # E57 Parser Library Tests (with interface testing)
    add_executable(E57ParserLibTests
        tests/test_e57parserlib.cpp
        src/e57parserlib.cpp
        src/IE57Parser.cpp
        src/performance_profiler.cpp
    )

    target_link_libraries(E57ParserLibTests
        GTest::gtest_main
        Qt6::Core
        Qt6::Xml
        Qt6::Test
        E57Format
    )

    target_include_directories(E57ParserLibTests PRIVATE src)
    add_test(NAME E57ParserLibTests COMMAND E57ParserLibTests)

    # E57 Parser Core Tests (Sprint 2 - Qt-independent core)
    add_executable(E57ParserCoreTests
        tests/test_e57parsercore.cpp
        src/E57ParserCore.cpp
    )

    target_link_libraries(E57ParserCoreTests
        GTest::gtest_main
        E57Format
    )

    target_include_directories(E57ParserCoreTests PRIVATE src)
    add_test(NAME E57ParserCoreTests COMMAND E57ParserCoreTests)






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

    # Sprint 5: MainPresenter Tests (requires Google Mock)
    if(GMOCK_AVAILABLE)
        add_executable(MainPresenterTests
            tests/test_mainpresenter.cpp
            src/MainPresenter.cpp
            src/IE57Parser.cpp
            src/performance_profiler.cpp
        )

        target_link_libraries(MainPresenterTests
            GTest::gtest_main
            GTest::gmock_main
            Qt6::Core
            Qt6::Test
        )

        target_include_directories(MainPresenterTests PRIVATE src)
        add_test(NAME MainPresenterTests COMMAND MainPresenterTests)

        set(ALL_TESTS E57ParserTests E57ParserLibTests E57ParserCoreTests LasParserTests VoxelGridFilterTests ProjectManagerTests RecentProjectsManagerTests MainPresenterTests)
    else()
        set(ALL_TESTS E57ParserTests E57ParserLibTests E57ParserCoreTests LasParserTests VoxelGridFilterTests ProjectManagerTests RecentProjectsManagerTests)
    endif()

    # Custom target to run all tests
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS ${ALL_TESTS}
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
                DEPENDS ${ALL_TESTS}
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
