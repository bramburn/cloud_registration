# Sprint 8: Parsers Tests - Complete CMakeLists.txt
# This file defines test executables for the parsers library

# E57Parser Tests
add_executable(E57ParserTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_e57parser.cpp
)
target_link_libraries(E57ParserTests PRIVATE
    Parsers             # Link against the Parsers library
    Core                # Indirect dependency through Parsers
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(E57ParserTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include    # Core headers
)
add_test(NAME E57ParserTests COMMAND E57ParserTests)

# E57ParserCore Tests
add_executable(E57ParserCoreTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_e57parsercore.cpp
)
target_link_libraries(E57ParserCoreTests PRIVATE
    Parsers
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(E57ParserCoreTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
)
add_test(NAME E57ParserCoreTests COMMAND E57ParserCoreTests)

# E57ParserLib Tests
add_executable(E57ParserLibTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_e57parserlib.cpp
)
target_link_libraries(E57ParserLibTests PRIVATE
    Parsers
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(E57ParserLibTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
)
add_test(NAME E57ParserLibTests COMMAND E57ParserLibTests)

# E57 Comprehensive Tests
add_executable(E57ComprehensiveTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_e57parser_sprint4_comprehensive.cpp
)
target_link_libraries(E57ComprehensiveTests PRIVATE
    Parsers
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(E57ComprehensiveTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
)
add_test(NAME E57ComprehensiveTests COMMAND E57ComprehensiveTests)

# LAS Parser Tests
add_executable(LasParserTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_lasparser.cpp
)
target_link_libraries(LasParserTests PRIVATE
    Parsers
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(LasParserTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
)
add_test(NAME LasParserTests COMMAND LasParserTests)

# E57 Test Framework
add_library(E57TestFramework STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/E57TestFramework.cpp
)
target_link_libraries(E57TestFramework PUBLIC
    Parsers
    Core
    Qt6::Core
    Qt6::Gui
)
target_include_directories(E57TestFramework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
)

# E57 Linkage Test (from test_parsers subdirectory)
add_executable(E57LinkageTest
    ${CMAKE_CURRENT_SOURCE_DIR}/test_e57_linkage.cpp
)
target_link_libraries(E57LinkageTest PRIVATE
    Parsers
    GTest::gtest_main
    E57Format
)
target_include_directories(E57LinkageTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include
)
add_test(NAME E57LinkageTest COMMAND E57LinkageTest)

message(STATUS "Configuring parsers tests...")
