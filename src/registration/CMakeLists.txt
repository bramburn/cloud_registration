# Sprint 6: Registration Library - Complete Implementation
# This file defines the Registration static library containing registration workflow and target management

# Define source files for the Registration library
set(REGISTRATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Target.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TargetCorrespondence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TargetManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlignmentEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ErrorAnalysis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FeatureBasedRegistration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PoseGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PoseGraphBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RegistrationProject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RegistrationWorkflowWidget.cpp

    # Files moved from detection/ to registration/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TargetDetectionBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NaturalPointSelector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SphereDetector.cpp
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
