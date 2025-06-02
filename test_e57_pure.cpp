#include <iostream>
#include <E57Format.h>
#include <string>
#include <memory>
#include <cfloat>

/**
 * @brief Pure C++ test without any Qt dependencies
 * 
 * This test replicates E57WriterLib functionality using only standard C++
 * to isolate if the issue is with Qt integration.
 */

class PureE57Writer {
public:
    PureE57Writer() : m_fileOpen(false), m_scanCount(0) {}
    
    bool createFile(const std::string& filePath) {
        try {
            std::cout << "Creating file: " << filePath << std::endl;

            // Create ImageFile in write mode
            std::cout << "Creating e57::ImageFile..." << std::endl;
            m_imageFile = std::make_unique<e57::ImageFile>(filePath, "w");
            
            std::cout << "Checking if file is open..." << std::endl;
            if (!m_imageFile->isOpen()) {
                std::cout << "ERROR: Failed to open file handle" << std::endl;
                return false;
            }

            m_currentFilePath = filePath;
            m_fileOpen = true;
            m_scanCount = 0;

            std::cout << "Initializing E57Root..." << std::endl;
            if (!initializeE57Root()) {
                closeFile();
                return false;
            }

            std::cout << "File created successfully" << std::endl;
            return true;

        } catch (const e57::E57Exception& ex) {
            std::cout << "E57 Exception in createFile: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
            return false;
        } catch (const std::exception& ex) {
            std::cout << "Standard exception in createFile: " << ex.what() << std::endl;
            return false;
        }
    }
    
    bool closeFile() {
        if (!m_fileOpen) {
            return true;
        }

        try {
            if (m_imageFile) {
                std::cout << "Closing e57::ImageFile..." << std::endl;
                m_imageFile->close();
                m_imageFile.reset();
            }

            m_fileOpen = false;
            std::cout << "File closed successfully" << std::endl;
            return true;

        } catch (const e57::E57Exception& ex) {
            std::cout << "E57 Exception in closeFile: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
            return false;
        } catch (const std::exception& ex) {
            std::cout << "Standard exception in closeFile: " << ex.what() << std::endl;
            return false;
        }
    }
    
private:
    bool initializeE57Root() {
        try {
            std::cout << "Getting root node..." << std::endl;
            e57::StructureNode rootNode = m_imageFile->root();

            std::cout << "Setting formatName..." << std::endl;
            e57::StringNode formatNameNode(*m_imageFile, "ASTM E57 3D Imaging Data File");
            rootNode.set("formatName", formatNameNode);

            std::cout << "Setting GUID..." << std::endl;
            e57::StringNode guidNode(*m_imageFile, "{12345678-1234-1234-1234-123456789abc}");
            rootNode.set("guid", guidNode);

            std::cout << "Setting version info..." << std::endl;
            e57::IntegerNode versionMajorNode(*m_imageFile, 1, 0, 255);
            rootNode.set("versionMajor", versionMajorNode);

            e57::IntegerNode versionMinorNode(*m_imageFile, 0, 0, 255);
            rootNode.set("versionMinor", versionMinorNode);

            std::cout << "Setting creation date..." << std::endl;
            e57::StringNode dateTimeNode(*m_imageFile, "2025-01-31T12:00:00Z");
            rootNode.set("creationDateTime", dateTimeNode);

            std::cout << "Setting coordinate metadata..." << std::endl;
            e57::StringNode coordinateMetadataNode(*m_imageFile, "");
            rootNode.set("coordinateMetadata", coordinateMetadataNode);

            std::cout << "Creating data3D vector..." << std::endl;
            e57::VectorNode data3DVector(*m_imageFile, false);
            rootNode.set("data3D", data3DVector);

            std::cout << "E57Root initialized successfully" << std::endl;
            return true;

        } catch (const e57::E57Exception& ex) {
            std::cout << "E57 Exception in initializeE57Root: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
            return false;
        }
    }
    
    std::unique_ptr<e57::ImageFile> m_imageFile;
    std::string m_currentFilePath;
    bool m_fileOpen;
    int m_scanCount;
};

int main() {
    std::cout << "=== Pure C++ E57 Test ===" << std::endl;
    
    try {
        std::string testFilePath = "pure_test.e57";
        std::cout << "Test file path: " << testFilePath << std::endl;
        
        // Test file creation
        std::cout << "\n--- Creating E57 file ---" << std::endl;
        PureE57Writer writer;
        
        bool result = writer.createFile(testFilePath);
        std::cout << "createFile result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
        if (!result) {
            return 1;
        }
        
        result = writer.closeFile();
        std::cout << "closeFile result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
        if (!result) {
            return 1;
        }
        
        std::cout << "\n=== Test completed successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& ex) {
        std::cout << "Unexpected exception: " << ex.what() << std::endl;
        return 1;
    }
}
