# Sprint 7: Interfaces Library - Complete Implementation
# Define Interfaces as a header-only library
add_library(Interfaces INTERFACE)
target_include_directories(Interfaces INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/interfaces>
)
# Interfaces might need to expose dependencies like Core for types it uses in its API
target_link_libraries(Interfaces INTERFACE Core Qt6::Core) # Add if core types are used directly in interfaces headers
message(STATUS "Configuring Interfaces library (Header-only)...")
