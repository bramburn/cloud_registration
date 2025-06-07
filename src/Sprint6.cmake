# Sprint 6 - Export, Quality Assessment, and Coordinate System Management
# This file defines the Sprint 6 components for the FARO Scene Registration MVP

# Export functionality
set(EXPORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/export/IFormatWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/PointCloudExporter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/PointCloudExporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/E57Writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/E57Writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/LASWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/LASWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/PLYWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/PLYWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/XYZWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/FormatWriters/XYZWriter.cpp
)

# Quality assessment functionality
set(QUALITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/QualityAssessment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/QualityAssessment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/PDFReportGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/PDFReportGenerator.cpp
)

# Coordinate system management
set(CRS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/crs/CoordinateSystemManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/crs/CoordinateSystemManager.cpp
)

# UI components
set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportDialog.cpp
)

# Combine all Sprint 6 sources
set(SPRINT6_SOURCES
    ${EXPORT_SOURCES}
    ${QUALITY_SOURCES}
    ${CRS_SOURCES}
    ${UI_SOURCES}
)

# Add Sprint 6 sources to the main target
target_sources(${PROJECT_NAME} PRIVATE ${SPRINT6_SOURCES})

# Include directories for Sprint 6
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/export
    ${CMAKE_CURRENT_SOURCE_DIR}/quality
    ${CMAKE_CURRENT_SOURCE_DIR}/crs
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# Find required packages for Sprint 6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui PrintSupport)

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::PrintSupport
)

# Find E57 library (if available)
find_library(E57_LIBRARY NAMES E57RefImpl e57 PATHS /usr/local/lib /usr/lib)
if(E57_LIBRARY)
    message(STATUS "Found E57 library: ${E57_LIBRARY}")
    target_link_libraries(${PROJECT_NAME} ${E57_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_E57_LIBRARY)
else()
    message(WARNING "E57 library not found. E57 export will use stub implementation.")
endif()

# Find LAS library (if available)
find_library(LAS_LIBRARY NAMES las PATHS /usr/local/lib /usr/lib)
if(LAS_LIBRARY)
    message(STATUS "Found LAS library: ${LAS_LIBRARY}")
    target_link_libraries(${PROJECT_NAME} ${LAS_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_LAS_LIBRARY)
else()
    message(WARNING "LAS library not found. LAS export will use custom implementation.")
endif()

# Add compiler definitions for Sprint 6
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SPRINT6_ENABLED
    EXPORT_FUNCTIONALITY_ENABLED
    QUALITY_ASSESSMENT_ENABLED
    CRS_MANAGEMENT_ENABLED
)

# Set C++ standard
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Add MOC for Qt classes
qt6_wrap_cpp(SPRINT6_MOC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/export/PointCloudExporter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/QualityAssessment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/PDFReportGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/crs/CoordinateSystemManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportDialog.h
)

target_sources(${PROJECT_NAME} PRIVATE ${SPRINT6_MOC_SOURCES})

# Create Sprint 6 test executable
if(BUILD_TESTING)
    add_executable(Sprint6Test
        ${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sprint6Test.cpp
        ${SPRINT6_SOURCES}
        ${SPRINT6_MOC_SOURCES}
    )
    
    target_link_libraries(Sprint6Test
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::PrintSupport
        Qt6::Test
    )
    
    target_include_directories(Sprint6Test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/export
        ${CMAKE_CURRENT_SOURCE_DIR}/quality
        ${CMAKE_CURRENT_SOURCE_DIR}/crs
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
    )
    
    if(E57_LIBRARY)
        target_link_libraries(Sprint6Test ${E57_LIBRARY})
        target_compile_definitions(Sprint6Test PRIVATE HAVE_E57_LIBRARY)
    endif()
    
    if(LAS_LIBRARY)
        target_link_libraries(Sprint6Test ${LAS_LIBRARY})
        target_compile_definitions(Sprint6Test PRIVATE HAVE_LAS_LIBRARY)
    endif()
    
    target_compile_definitions(Sprint6Test PRIVATE
        SPRINT6_ENABLED
        EXPORT_FUNCTIONALITY_ENABLED
        QUALITY_ASSESSMENT_ENABLED
        CRS_MANAGEMENT_ENABLED
    )
    
    add_test(NAME Sprint6Test COMMAND Sprint6Test)
endif()

# Install Sprint 6 headers (for development)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/export/IFormatWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/export/PointCloudExporter.h
    DESTINATION include/export
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/QualityAssessment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quality/PDFReportGenerator.h
    DESTINATION include/quality
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/crs/CoordinateSystemManager.h
    DESTINATION include/crs
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/ExportDialog.h
    DESTINATION include/ui
)

message(STATUS "Sprint 6 components configured successfully")
message(STATUS "  - Export functionality: ENABLED")
message(STATUS "  - Quality assessment: ENABLED") 
message(STATUS "  - Coordinate system management: ENABLED")
message(STATUS "  - UI components: ENABLED")
