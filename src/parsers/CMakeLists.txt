# Sprint 1: Parsers Library - Placeholder CMakeLists.txt
# This file defines the Parsers static library containing file format parsers

# Define source files for the Parsers library
# Note: These files are currently at src/ root but will be moved to src/parsers/src/ in future sprints
set(PARSERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../e57parserlib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../E57ParserCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../IE57Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lasparser.cpp
)

set(PARSERS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../e57parserlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../E57ParserCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../IE57Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../IE57Writer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../lasparser.h
)

# Create Parsers static library
add_library(Parsers STATIC ${PARSERS_SOURCES} ${PARSERS_HEADERS})

target_include_directories(Parsers PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # Temporary for files at src/ root
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/parsers>
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
