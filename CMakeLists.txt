cmake_minimum_required(VERSION 3.16)

project(CloudRegistration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)

# Enable Qt's MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/pointcloudviewerwidget.cpp
    src/e57parser.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/pointcloudviewerwidget.h
    src/e57parser.h
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

# Set Qt6 path if installed in custom location
if(WIN32 AND NOT DEFINED Qt6_DIR)
    # Try common Qt6 installation paths
    set(QT_POSSIBLE_PATHS
        "C:/Qt/6.8.0/msvc2022_64/lib/cmake/Qt6"
        "C:/Qt/6.7.0/msvc2022_64/lib/cmake/Qt6"
        "C:/Qt/6.6.0/msvc2022_64/lib/cmake/Qt6"
        "C:/Qt/6.5.3/msvc2019_64/lib/cmake/Qt6"
        "C:/Qt/6.5.0/msvc2019_64/lib/cmake/Qt6"
        "C:/Qt/6.5.0/msvc2022_64/lib/cmake/Qt6"
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
    add_executable(CloudRegistrationTests
        tests/test_e57parser.cpp
        src/e57parser.cpp
    )

    target_link_libraries(CloudRegistrationTests
        GTest::gtest_main
        Qt6::Core
    )

    target_include_directories(CloudRegistrationTests PRIVATE src)

    add_test(NAME E57ParserTests COMMAND CloudRegistrationTests)
endif()
