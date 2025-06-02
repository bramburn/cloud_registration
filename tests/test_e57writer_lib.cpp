#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDebug>
#include <E57Format.h>
#include "../src/e57writer_lib.h"

/**
 * @brief Test fixture for E57WriterLib tests
 * 
 * Implements Sprint W1 testing requirements for E57 file creation,
 * header writing, prototype definition, and file validity verification.
 */
class E57WriterLibTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid()) << "Failed to create temporary directory";
        
        writer = std::make_unique<E57WriterLib>();
        
        // Generate test file path
        testFilePath = tempDir->path() + "/test_output.e57";
    }

    void TearDown() override {
        writer.reset();
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<E57WriterLib> writer;
    QString testFilePath;
};

/**
 * Test Case W1.1.1: Attempt to create an E57 file in a writable directory
 * Expected Result: A small E57 file is created with correct signature and version
 */
TEST_F(E57WriterLibTest, CreateE57FileInWritableDirectory) {
    // Test file creation
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file: " << writer->getLastError().toStdString();

    // Verify writer state
    EXPECT_TRUE(writer->isFileOpen()) << "Writer should report file as open";
    EXPECT_EQ(writer->getCurrentFilePath(), testFilePath) << "Writer should track current file path";
    EXPECT_TRUE(writer->getLastError().isEmpty()) << "No error should be reported";

    // Close file
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";
    EXPECT_FALSE(writer->isFileOpen()) << "Writer should report file as closed";

    // Verify file exists and has content after closing
    QFileInfo fileInfo(testFilePath);
    EXPECT_TRUE(fileInfo.exists()) << "E57 file was not created";
    EXPECT_GT(fileInfo.size(), 0) << "E57 file is empty";
    
    // Verify file can be opened by libE57Format for reading
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        EXPECT_TRUE(testFile.isOpen()) << "Created file cannot be opened by libE57Format";
        
        // Verify E57Root structure
        e57::StructureNode root = testFile.root();
        EXPECT_TRUE(root.isDefined("formatName")) << "formatName not found in E57Root";
        EXPECT_TRUE(root.isDefined("guid")) << "guid not found in E57Root";
        
        // Verify formatName value
        e57::StringNode formatName = static_cast<e57::StringNode>(root.get("formatName"));
        EXPECT_EQ(formatName.value(), "ASTM E57 3D Imaging Data File") << "Incorrect formatName value";
        
        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when reading created file: " << ex.what();
    }
}

/**
 * Test Case W1.1.2: Attempt to create an E57 file in a non-writable directory
 * Expected Result: File creation fails with appropriate error message
 */
TEST_F(E57WriterLibTest, CreateE57FileInNonWritableDirectory) {
    QString invalidPath = "/invalid/nonexistent/path/test.e57";
    
    EXPECT_FALSE(writer->createFile(invalidPath)) << "File creation should fail for invalid path";
    EXPECT_FALSE(writer->getLastError().isEmpty()) << "Error message should be set";
    EXPECT_FALSE(writer->isFileOpen()) << "Writer should not report file as open";
    
    qDebug() << "Expected error message:" << writer->getLastError();
}

/**
 * Test Case W1.2.1: Generate an E57 file and verify its XML structure
 * Expected Result: /data3D vector with one child StructureNode containing guid and name
 */
TEST_F(E57WriterLibTest, CreateE57FileWithScanStructure) {
    // Create file and add scan
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Test Scan 001")) << "Failed to add scan";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";
    
    // Verify XML structure by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();
        
        // Verify /data3D exists
        EXPECT_TRUE(root.isDefined("data3D")) << "/data3D VectorNode not found";

        e57::VectorNode data3D = static_cast<e57::VectorNode>(root.get("data3D"));
        EXPECT_EQ(data3D.childCount(), 1) << "/data3D should contain exactly one scan";

        // Verify scan structure
        e57::StructureNode scan = static_cast<e57::StructureNode>(data3D.get(0));
        EXPECT_TRUE(scan.isDefined("guid")) << "Scan should have guid";
        EXPECT_TRUE(scan.isDefined("name")) << "Scan should have name";

        // Verify scan name
        e57::StringNode scanName = static_cast<e57::StringNode>(scan.get("name"));
        EXPECT_EQ(scanName.value(), "Test Scan 001") << "Incorrect scan name";
        
        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying scan structure: " << ex.what();
    }
}

/**
 * Test Case W1.3.1: Generate an E57 file and inspect its XML for the points CompressedVectorNode
 * Expected Result: XML shows /data3D/0/points as CompressedVectorNode with XYZ prototype
 */
TEST_F(E57WriterLibTest, CreateE57FileWithXYZPrototype) {
    // Create file, add scan, and define XYZ prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Test Scan with Points")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify points structure by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        // Navigate to scan
        e57::VectorNode data3D = static_cast<e57::VectorNode>(root.get("data3D"));
        e57::StructureNode scan = static_cast<e57::StructureNode>(data3D.get(0));

        // Verify the CompressedVectorNode exists
        EXPECT_TRUE(scan.isDefined("points")) << "Scan should have points CompressedVectorNode";

        // Verify points is a CompressedVectorNode
        e57::CompressedVectorNode pointsNode = static_cast<e57::CompressedVectorNode>(scan.get("points"));
        EXPECT_EQ(pointsNode.childCount(), 0) << "Points node should be empty (0 points)";

        // Verify the prototype structure within the CompressedVectorNode
        e57::StructureNode prototype = pointsNode.prototype();
        EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
        EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
        EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";

        // Verify coordinate fields are FloatNodes with double precision
        e57::FloatNode xNode = static_cast<e57::FloatNode>(prototype.get("cartesianX"));
        e57::FloatNode yNode = static_cast<e57::FloatNode>(prototype.get("cartesianY"));
        e57::FloatNode zNode = static_cast<e57::FloatNode>(prototype.get("cartesianZ"));

        EXPECT_EQ(xNode.precision(), e57::PrecisionDouble) << "cartesianX should have double precision";
        EXPECT_EQ(yNode.precision(), e57::PrecisionDouble) << "cartesianY should have double precision";
        EXPECT_EQ(zNode.precision(), e57::PrecisionDouble) << "cartesianZ should have double precision";

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying XYZ prototype: " << ex.what();
    }
}

/**
 * Test error handling for operations on closed file
 */
TEST_F(E57WriterLibTest, ErrorHandlingForClosedFile) {
    // Try operations without opening file
    EXPECT_FALSE(writer->addScan()) << "addScan should fail when no file is open";
    EXPECT_FALSE(writer->defineXYZPrototype()) << "defineXYZPrototype should fail when no file is open";
    EXPECT_FALSE(writer->getLastError().isEmpty()) << "Error message should be set";
}

/**
 * Test multiple scans in single file
 */
TEST_F(E57WriterLibTest, MultipleScanSupport) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    
    // Add multiple scans
    EXPECT_TRUE(writer->addScan("Scan 001")) << "Failed to add first scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define prototype for first scan";
    
    EXPECT_TRUE(writer->addScan("Scan 002")) << "Failed to add second scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define prototype for second scan";
    
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";
    
    // Verify both scans exist
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();
        e57::VectorNode data3D = static_cast<e57::VectorNode>(root.get("data3D"));

        EXPECT_EQ(data3D.childCount(), 2) << "/data3D should contain two scans";

        // Verify first scan
        e57::StructureNode scan1 = static_cast<e57::StructureNode>(data3D.get(0));
        e57::StringNode name1 = static_cast<e57::StringNode>(scan1.get("name"));
        EXPECT_EQ(name1.value(), "Scan 001") << "First scan name incorrect";
        EXPECT_TRUE(scan1.isDefined("points")) << "First scan should have points CompressedVectorNode";

        // Verify second scan
        e57::StructureNode scan2 = static_cast<e57::StructureNode>(data3D.get(1));
        e57::StringNode name2 = static_cast<e57::StringNode>(scan2.get("name"));
        EXPECT_EQ(name2.value(), "Scan 002") << "Second scan name incorrect";
        EXPECT_TRUE(scan2.isDefined("points")) << "Second scan should have points CompressedVectorNode";
        
        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying multiple scans: " << ex.what();
    }
}
