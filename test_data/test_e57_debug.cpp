#include <iostream>
#include <memory>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QDebug>
#include <E57Format.h>
#include "src/e57writer_lib.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create temporary directory
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        std::cerr << "Failed to create temporary directory" << std::endl;
        return 1;
    }

    QString testFilePath = tempDir.path() + "/debug_test.e57";
    std::cout << "Test file path: " << testFilePath.toStdString() << std::endl;

    std::cout << "\n=== Direct libE57Format Test ===" << std::endl;
    try {
        // Test direct libE57Format usage
        e57::ImageFile imf(testFilePath.toStdString(), "w");
        std::cout << "ImageFile created in write mode: " << (imf.isOpen() ? "YES" : "NO") << std::endl;

        // Get root and add required elements
        e57::StructureNode root = imf.root();

        // Add required E57Root elements
        root.set("formatName", e57::StringNode(imf, "ASTM E57 3D Imaging Data File"));
        root.set("guid", e57::StringNode(imf, "{12345678-1234-1234-1234-123456789abc}"));
        root.set("versionMajor", e57::IntegerNode(imf, 1, 0, 255));
        root.set("versionMinor", e57::IntegerNode(imf, 0, 0, 255));

        // Add empty data3D vector
        e57::VectorNode data3D(imf, true);
        root.set("data3D", data3D);

        std::cout << "Added required elements to root" << std::endl;

        // Close the file
        imf.close();
        std::cout << "File closed" << std::endl;

        // Check file size
        QFileInfo fileInfo(testFilePath);
        std::cout << "File exists: " << (fileInfo.exists() ? "YES" : "NO") << std::endl;
        std::cout << "File size: " << fileInfo.size() << " bytes" << std::endl;

        // Try to read it back
        if (fileInfo.size() > 0) {
            e57::ImageFile readFile(testFilePath.toStdString(), "r");
            std::cout << "File can be read: " << (readFile.isOpen() ? "YES" : "NO") << std::endl;

            if (readFile.isOpen()) {
                e57::StructureNode readRoot = readFile.root();
                std::cout << "formatName: " << (readRoot.isDefined("formatName") ? "YES" : "NO") << std::endl;
                std::cout << "guid: " << (readRoot.isDefined("guid") ? "YES" : "NO") << std::endl;
                std::cout << "data3D: " << (readRoot.isDefined("data3D") ? "YES" : "NO") << std::endl;
                readFile.close();
            }
        }

    } catch (const e57::E57Exception& ex) {
        std::cout << "E57 Exception: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "Standard exception: " << ex.what() << std::endl;
    }

    std::cout << "\n=== E57WriterLib Test ===" << std::endl;
    // Now test our wrapper
    E57WriterLib writer;

    bool result = writer.createFile(testFilePath + "_wrapper");
    std::cout << "E57WriterLib createFile: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << writer.getLastError().toStdString() << std::endl;
    } else {
        result = writer.closeFile();
        std::cout << "E57WriterLib closeFile: " << (result ? "SUCCESS" : "FAILED") << std::endl;

        QFileInfo wrapperInfo(testFilePath + "_wrapper");
        std::cout << "Wrapper file size: " << wrapperInfo.size() << " bytes" << std::endl;
    }

    std::cout << "\n=== Test completed ===" << std::endl;
    return 0;
}
