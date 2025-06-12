# Sprint 1: Registration Library - Placeholder CMakeLists.txt
# This file defines the Registration static library containing registration workflow components

# Define source files for the Registration library
set(REGISTRATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Target.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetCorrespondence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AlignmentEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ErrorAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FeatureBasedRegistration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PoseGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PoseGraphBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistrationProject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistrationWorkflowWidget.cpp
)

set(REGISTRATION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Target.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetCorrespondence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AlignmentEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ErrorAnalysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FeatureBasedRegistration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PoseGraph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PoseGraphBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistrationProject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RegistrationWorkflowWidget.h
)

# Create Registration static library
add_library(Registration STATIC ${REGISTRATION_SOURCES} ${REGISTRATION_HEADERS})

target_include_directories(Registration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/registration>
)

# Registration library dependencies: Core, Algorithms, Qt components
target_link_libraries(Registration PUBLIC
    Core
    Algorithms
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

message(STATUS "Configuring Registration library...")
