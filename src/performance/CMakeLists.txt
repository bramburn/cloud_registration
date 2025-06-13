# Sprint 7: Performance Library - Complete Implementation
# This file defines the Performance static library containing performance monitoring components

# Define source files for the Performance library
set(PERFORMANCE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ParallelProcessing.cpp # Assuming ParallelProcessing also moves here
)
set(PERFORMANCE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/performance/MemoryManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/performance/ParallelProcessing.h # Assuming ParallelProcessing also moves here
)
add_library(Performance STATIC ${PERFORMANCE_SOURCES} ${PERFORMANCE_HEADERS})
target_include_directories(Performance PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/performance>
)
target_link_libraries(Performance PUBLIC Core Qt6::Core Qt6::Gui) # Core for PointFullData
message(STATUS "Configuring Performance library...")
