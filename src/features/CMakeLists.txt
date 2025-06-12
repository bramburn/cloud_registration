# Sprint 1: Features Library - Placeholder CMakeLists.txt
# This file defines the Features static library containing feature extraction components

# Define source files for the Features library
set(FEATURES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FeatureExtractor.cpp
)

set(FEATURES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/FeatureExtractor.h
)

# Create Features static library
add_library(Features STATIC ${FEATURES_SOURCES} ${FEATURES_HEADERS})

target_include_directories(Features PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/features>
)

# Features library dependencies: Core, Qt components
target_link_libraries(Features PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Features library...")
