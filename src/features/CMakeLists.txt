# Sprint 7: Features Library - Complete Implementation
# This file defines the Features static library containing feature extraction components

# Define source files for the Features library
set(FEATURES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FeatureExtractor.cpp
)
set(FEATURES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/features/FeatureExtractor.h
)
add_library(Features STATIC ${FEATURES_SOURCES} ${FEATURES_HEADERS})
target_include_directories(Features PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/features>
)
target_link_libraries(Features PUBLIC Core Qt6::Core Qt6::Gui) # Core for Point3D, Qt6::Gui for QVector
message(STATUS "Configuring Features library...")
