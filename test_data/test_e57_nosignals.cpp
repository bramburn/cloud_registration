#include <iostream>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDebug>
#include <QUuid>
#include <QDateTime>
#include <E57Format.h>
#include <cfloat>

/**
 * @brief Test E57WriterLib functionality without Qt signals
 * 
 * This test replicates E57WriterLib functionality without inheriting from QObject
 * to isolate if the issue is with Qt signals/slots.
 */

class SimpleE57Writer {
public:
    SimpleE57Writer() : m_fileOpen(false), m_scanCount(0) {}
    
    bool createFile(const QString& filePath) {
        try {
            std::cout << "Creating file: " << filePath.toStdString() << std::endl;
            
            // Validate file path
            QFileInfo fileInfo(filePath);
            QDir parentDir = fileInfo.dir();
            if (!parentDir.exists()) {
                std::cout << "ERROR: Directory does not exist: " << parentDir.absolutePath().toStdString() << std::endl;
                return false;
            }

            // Create ImageFile in write mode
            std::cout << "Creating e57::ImageFile..." << std::endl;
            m_imageFile = std::make_unique<e57::ImageFile>(filePath.toStdString(), "w");
            
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

            std::cout << "Generating GUID..." << std::endl;
            QUuid uuid = QUuid::createUuid();
            e57::StringNode guidNode(*m_imageFile, uuid.toString().toStdString());
            rootNode.set("guid", guidNode);

            std::cout << "Setting version info..." << std::endl;
            e57::IntegerNode versionMajorNode(*m_imageFile, 1, 0, 255);
            rootNode.set("versionMajor", versionMajorNode);

            e57::IntegerNode versionMinorNode(*m_imageFile, 0, 0, 255);
            rootNode.set("versionMinor", versionMinorNode);

            std::cout << "Setting creation date..." << std::endl;
            QString creationDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
            e57::StringNode dateTimeNode(*m_imageFile, creationDateTime.toStdString());
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
    QString m_currentFilePath;
    bool m_fileOpen;
    int m_scanCount;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    std::cout << "=== E57WriterLib No Signals Test ===" << std::endl;
    
    try {
        // Create temporary directory
        QTemporaryDir tempDir;
        if (!tempDir.isValid()) {
            std::cout << "ERROR: Failed to create temporary directory" << std::endl;
            return 1;
        }
        
        QString testFilePath = tempDir.path() + "/nosignals_test.e57";
        std::cout << "Test file path: " << testFilePath.toStdString() << std::endl;
        
        // Test file creation
        std::cout << "\n--- Creating E57 file ---" << std::endl;
        SimpleE57Writer writer;
        
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
        
        // Verify file exists
        QFileInfo fileInfo(testFilePath);
        std::cout << "File exists: " << (fileInfo.exists() ? "YES" : "NO") << std::endl;
        std::cout << "File size: " << fileInfo.size() << " bytes" << std::endl;
        
        std::cout << "\n=== Test completed successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& ex) {
        std::cout << "Unexpected exception: " << ex.what() << std::endl;
        return 1;
    }
}
