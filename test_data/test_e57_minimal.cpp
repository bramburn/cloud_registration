#include <iostream>
#include <E57Format.h>
#include <string>

/**
 * @brief Minimal test to check if libE57Format itself is causing hangs
 */
int main() {
    std::cout << "=== Minimal libE57Format Test ===" << std::endl;
    
    try {
        std::string testFilePath = "minimal_test.e57";
        std::cout << "Test file path: " << testFilePath << std::endl;
        
        std::cout << "Creating ImageFile..." << std::endl;
        std::cout.flush();
        
        e57::ImageFile imageFile(testFilePath, "w");
        
        std::cout << "ImageFile created" << std::endl;
        std::cout << "Checking if file is open..." << std::endl;
        
        bool isOpen = imageFile.isOpen();
        std::cout << "File isOpen: " << (isOpen ? "YES" : "NO") << std::endl;
        
        if (isOpen) {
            std::cout << "Getting root node..." << std::endl;
            e57::StructureNode root = imageFile.root();
            std::cout << "Root node obtained" << std::endl;
            
            std::cout << "Setting formatName..." << std::endl;
            e57::StringNode formatName(imageFile, "ASTM E57 3D Imaging Data File");
            root.set("formatName", formatName);
            std::cout << "formatName set" << std::endl;
            
            std::cout << "Closing file..." << std::endl;
            imageFile.close();
            std::cout << "File closed" << std::endl;
        }
        
        std::cout << "=== Test completed successfully ===" << std::endl;
        return 0;
        
    } catch (const e57::E57Exception& ex) {
        std::cout << "E57 Exception: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cout << "Standard Exception: " << ex.what() << std::endl;
        return 1;
    }
}
