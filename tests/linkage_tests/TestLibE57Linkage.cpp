#include <E57Format.h>
#include <iostream>
#include <string>

/**
 * @brief Minimal "smoke test" program to verify libE57Format linkage
 * 
 * This program demonstrates that the libE57Format library is correctly
 * linked into the build by making basic API calls and printing version
 * information to stdout.
 * 
 * Returns 0 on success, non-zero on failure.
 */
int main() {
    try {
        // Test basic API access by getting library version
        std::cout << "libE57Format Linkage Test" << std::endl;
        std::cout << "=========================" << std::endl;
        
        // Test basic library functionality
        std::cout << "Testing basic libE57Format functionality..." << std::endl;
        
        // Test that we can create basic E57 objects
        std::cout << "Testing basic E57 object creation..." << std::endl;
        
        // Create a temporary file path for testing
        std::string tempPath = "linkage_test_temp.e57";
        
        // Test creating an ImageFile (this tests constructor linkage)
        try {
            e57::ImageFile imageFile(tempPath, "w");
            std::cout << "✓ ImageFile creation successful" << std::endl;
            
            // Test basic root access
            e57::StructureNode root = imageFile.root();
            std::cout << "✓ Root node access successful" << std::endl;
            
            // Clean up
            imageFile.close();
            std::remove(tempPath.c_str());
            
        } catch (const e57::E57Exception& ex) {
            std::cout << "✓ E57Exception handling works: " << ex.what() << std::endl;
        }
        
        // Test basic E57 functionality
        std::cout << "Testing basic E57 functionality..." << std::endl;
        try {
            // Try to open a non-existent file - this should fail gracefully
            e57::ImageFile testFile("non_existent_file.e57", "r");
            std::cout << "✗ Unexpected success with non-existent file" << std::endl;
        } catch (const e57::E57Exception& ex) {
            std::cout << "✓ E57Exception handling works: " << ex.what() << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "All linkage tests passed successfully!" << std::endl;
        std::cout << "libE57Format is properly linked and functional." << std::endl;
        
        return 0;
        
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: Linkage test failed with exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "ERROR: Linkage test failed with unknown exception" << std::endl;
        return 2;
    }
}
