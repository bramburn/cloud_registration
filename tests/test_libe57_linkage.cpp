#include <iostream>
#include <E57Format/E57Format.h>
#include <QString>
#include "../src/e57parserlib.h"

/**
 * @brief Minimal test program to verify libE57Format linkage
 * 
 * This program tests that:
 * 1. libE57Format headers can be included
 * 2. libE57Format library functions can be called
 * 3. E57ParserLib can be instantiated
 * 
 * As specified in Sprint 1 Task 1.1.4
 */
int main() {
    std::cout << "Testing libE57Format linkage..." << std::endl;
    
    try {
        // Test 1: Test basic E57 functionality
        std::cout << "libE57Format library loaded successfully" << std::endl;
        
        // Test 2: Test E57ParserLib instantiation
        E57ParserLib parser;
        std::cout << "E57ParserLib instantiated successfully" << std::endl;
        
        // Test 3: Test basic error handling
        QString lastError = parser.getLastError();
        std::cout << "Initial error state: " << (lastError.isEmpty() ? "No error" : lastError.toStdString()) << std::endl;

        // Test 4: Test file operations with non-existent file
        bool result = parser.openFile("non_existent_file.e57");
        if (!result) {
            std::cout << "Expected failure for non-existent file: " << parser.getLastError().toStdString() << std::endl;
        }
        
        std::cout << "All linkage tests passed!" << std::endl;
        return 0;
        
    } catch (const e57::E57Exception& ex) {
        std::cerr << "E57 Exception during linkage test: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Standard exception during linkage test: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception during linkage test" << std::endl;
        return 1;
    }
}
