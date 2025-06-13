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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/Target.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/TargetCorrespondence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/TargetManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/AlignmentEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/ErrorAnalysis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/FeatureBasedRegistration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/PoseGraph.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/PoseGraphBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/RegistrationProject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/RegistrationWorkflowWidget.h

    # Headers moved from detection/ to registration/ (Sprint 6)
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/TargetDetectionBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/NaturalPointSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/registration/SphereDetector.h
)

# Create Registration static library
add_library(Registration STATIC ${REGISTRATION_SOURCES} ${REGISTRATION_HEADERS})

target_include_directories(Registration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/registration>
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
