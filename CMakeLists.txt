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
find_package(Qt6 6.9.0 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)

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
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/pointcloudviewerwidget.h
    src/e57parser.h
    src/lasparser.h
)

# Shader files
set(SHADERS
    shaders/point.vert
    shaders/point.frag
)

# Create the executable
add_executable(CloudRegistration ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(CloudRegistration
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
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
find_package(GTest QUIET)
if(GTest_FOUND)
    # E57 Parser Tests
    add_executable(E57ParserTests
        tests/test_e57parser.cpp
        src/e57parser.cpp
    )

    target_link_libraries(E57ParserTests
        GTest::gtest_main
        Qt6::Core
    )

    target_include_directories(E57ParserTests PRIVATE src)

    add_test(NAME E57ParserTests COMMAND E57ParserTests)

    # LAS Parser Tests
    add_executable(LasParserTests
        tests/test_lasparser.cpp
        src/lasparser.cpp
    )

    target_link_libraries(LasParserTests
        GTest::gtest_main
        Qt6::Core
    )

    target_include_directories(LasParserTests PRIVATE src)

    add_test(NAME LasParserTests COMMAND LasParserTests)
endif()