# Sprint 1: Parsers Library - Placeholder CMakeLists.txt
# This file defines the Parsers static library containing file format parsers

# Define source files for the Parsers library
# Files have been moved to src/parsers/src/ as part of restructuring
set(PARSERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/e57parserlib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/E57ParserCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lasparser.cpp
)

set(PARSERS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers/e57parserlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers/E57ParserCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parsers/lasparser.h
)

# Create Parsers static library
add_library(Parsers STATIC ${PARSERS_SOURCES} ${PARSERS_HEADERS})

target_include_directories(Parsers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration>
)

# Parsers library dependencies: Core, Qt components, and optionally E57Format, XercesC
target_link_libraries(Parsers PUBLIC
    Core
    Qt6::Core
)

# Link external libraries only if available
if(E57FORMAT_AVAILABLE)
    target_link_libraries(Parsers PUBLIC E57Format)
endif()

if(XERCES_AVAILABLE)
    target_link_libraries(Parsers PUBLIC XercesC::XercesC)
endif()

message(STATUS "Configuring Parsers library...")
