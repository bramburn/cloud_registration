#include <iostream>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDebug>
#include <E57Format.h>
#include <QUuid>
#include <QDateTime>
#include <cfloat>

/**
 * @brief Simple test to isolate E57WriterLib hanging issues
 *
 * This test creates a minimal E57 file without using E57WriterLib class
 * to identify if the issue is with Qt integration or libE57Format.
 */

bool createMinimalE57File(const QString& filePath) {
    try {
        std::cout << "Creating E57 file: " << filePath.toStdString() << std::endl;

        // Create ImageFile in write mode
        e57::ImageFile imageFile(filePath.toStdString(), "w");

        if (!imageFile.isOpen()) {
            std::cout << "ERROR: Failed to open file handle" << std::endl;
            return false;
        }

        // Setup basic E57Root elements
        e57::StructureNode rootNode = imageFile.root();

        e57::StringNode formatNameNode(imageFile, "ASTM E57 3D Imaging Data File");
        rootNode.set("formatName", formatNameNode);

        QString guid = QUuid::createUuid().toString();
        e57::StringNode guidNode(imageFile, guid.toStdString());
        rootNode.set("guid", guidNode);

        // Add version information
        e57::IntegerNode versionMajorNode(imageFile, 1, 0, 255);
        rootNode.set("versionMajor", versionMajorNode);

        e57::IntegerNode versionMinorNode(imageFile, 0, 0, 255);
        rootNode.set("versionMinor", versionMinorNode);

        // Add creation date/time
        QString creationDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        e57::StringNode dateTimeNode(imageFile, creationDateTime.toStdString());
        rootNode.set("creationDateTime", dateTimeNode);

        // Add coordinate metadata
        e57::StringNode coordinateMetadataNode(imageFile, "");
        rootNode.set("coordinateMetadata", coordinateMetadataNode);

        // Create the data3D vector
        e57::VectorNode data3DVector(imageFile, false);
        rootNode.set("data3D", data3DVector);

        std::cout << "Closing file..." << std::endl;
        imageFile.close();

        std::cout << "File created successfully" << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        std::cout << "E57 Exception: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cout << "Standard exception: " << ex.what() << std::endl;
        return false;
    }
}

bool readAndVerifyE57File(const QString& filePath) {
    try {
        std::cout << "Reading E57 file: " << filePath.toStdString() << std::endl;

        e57::ImageFile testFile(filePath.toStdString(), "r");

        if (!testFile.isOpen()) {
            std::cout << "ERROR: Cannot open file for reading" << std::endl;
            return false;
        }

        std::cout << "File opened for reading" << std::endl;

        // Verify E57Root structure
        e57::StructureNode root = testFile.root();
        std::cout << "Got root node" << std::endl;

        if (!root.isDefined("formatName")) {
            std::cout << "ERROR: formatName not found" << std::endl;
            return false;
        }

        if (!root.isDefined("guid")) {
            std::cout << "ERROR: guid not found" << std::endl;
            return false;
        }

        // Verify formatName value
        e57::StringNode formatName(root.get("formatName"));
        std::string formatNameValue = formatName.value();
        std::cout << "formatName: " << formatNameValue << std::endl;

        if (formatNameValue != "ASTM E57 3D Imaging Data File") {
            std::cout << "ERROR: Incorrect formatName value" << std::endl;
            return false;
        }

        std::cout << "Closing read file..." << std::endl;
        testFile.close();

        std::cout << "File verification successful" << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        std::cout << "E57 Exception during read: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cout << "Standard exception during read: " << ex.what() << std::endl;
        return false;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    std::cout << "=== E57WriterLib Simple Test (No E57WriterLib Class) ===" << std::endl;

    try {
        // Create temporary directory
        QTemporaryDir tempDir;
        if (!tempDir.isValid()) {
            std::cout << "ERROR: Failed to create temporary directory" << std::endl;
            return 1;
        }
        
        QString testFilePath = tempDir.path() + "/simple_test.e57";
        std::cout << "Test file path: " << testFilePath.toStdString() << std::endl;

        // Test file creation
        std::cout << "\n--- Creating E57 file ---" << std::endl;
        if (!createMinimalE57File(testFilePath)) {
            std::cout << "ERROR: File creation failed" << std::endl;
            return 1;
        }

        // Check file size
        QFileInfo fileInfo(testFilePath);
        if (!fileInfo.exists()) {
            std::cout << "ERROR: File does not exist after creation" << std::endl;
            return 1;
        }

        std::cout << "File size: " << fileInfo.size() << " bytes" << std::endl;

        // Test file reading
        std::cout << "\n--- Reading and verifying E57 file ---" << std::endl;
        if (!readAndVerifyE57File(testFilePath)) {
            std::cout << "ERROR: File verification failed" << std::endl;
            return 1;
        }

        std::cout << "\n=== Test completed successfully ===" << std::endl;
        return 0;

    } catch (const std::exception& ex) {
        std::cout << "Unexpected exception: " << ex.what() << std::endl;
        return 1;
    }
}
