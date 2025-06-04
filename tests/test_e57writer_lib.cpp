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
    qDebug() << "File size:" << fileInfo.size() << "bytes";
    EXPECT_GT(fileInfo.size(), 0) << "E57 file is empty - size: " << fileInfo.size();
    
    // Verify file can be opened by libE57Format for reading
    // User Story 2: Enhanced exception handling for libE57Format API calls
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        EXPECT_TRUE(testFile.isOpen()) << "Created file cannot be opened by libE57Format";

        // Verify E57Root structure with defensive access
        e57::StructureNode root = testFile.root();
        EXPECT_TRUE(root.isDefined("formatName")) << "formatName not found in E57Root";
        EXPECT_TRUE(root.isDefined("guid")) << "guid not found in E57Root";

        // Verify formatName value with constructor-based conversion
        if (root.isDefined("formatName")) {
            e57::StringNode formatName(root.get("formatName"));
            EXPECT_EQ(formatName.value(), "ASTM E57 3D Imaging Data File") << "Incorrect formatName value";
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when reading created file: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when reading created file: " << ex.what();
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
        
        // Verify /data3D exists with defensive checking
        EXPECT_TRUE(root.isDefined("data3D")) << "/data3D VectorNode not found";

        if (root.isDefined("data3D")) {
            // User Story 1: Replace static_cast with constructor-based conversion
            e57::VectorNode data3D(root.get("data3D"));
            EXPECT_EQ(data3D.childCount(), 1) << "/data3D should contain exactly one scan";

            if (data3D.childCount() > 0) {
                // Verify scan structure - use constructor-based conversion
                e57::StructureNode scan(data3D.get(0));
                EXPECT_TRUE(scan.isDefined("guid")) << "Scan should have guid";
                EXPECT_TRUE(scan.isDefined("name")) << "Scan should have name";

                // Verify scan name - use constructor-based conversion with defensive access
                if (scan.isDefined("name")) {
                    e57::StringNode scanName(scan.get("name"));
                    EXPECT_EQ(scanName.value(), "Test Scan 001") << "Incorrect scan name";
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying scan structure: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying scan structure: " << ex.what();
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
    // User Story 2: Enhanced exception handling for libE57Format API calls
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        // Navigate to scan with defensive checking
        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));

            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify the CompressedVectorNode exists
                EXPECT_TRUE(scan.isDefined("points")) << "Scan should have points CompressedVectorNode";

                if (scan.isDefined("points")) {
                    // Verify points is a CompressedVectorNode
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 0) << "Points node should be empty (0 points)";

                    // Verify the prototype structure within the CompressedVectorNode
                    e57::StructureNode prototype(pointsNode.prototype());
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";

                    // Verify coordinate fields are FloatNodes with double precision - defensive access
                    if (prototype.isDefined("cartesianX") && prototype.isDefined("cartesianY") && prototype.isDefined("cartesianZ")) {
                        e57::FloatNode xNode(prototype.get("cartesianX"));
                        e57::FloatNode yNode(prototype.get("cartesianY"));
                        e57::FloatNode zNode(prototype.get("cartesianZ"));

                        EXPECT_EQ(xNode.precision(), e57::PrecisionDouble) << "cartesianX should have double precision";
                        EXPECT_EQ(yNode.precision(), e57::PrecisionDouble) << "cartesianY should have double precision";
                        EXPECT_EQ(zNode.precision(), e57::PrecisionDouble) << "cartesianZ should have double precision";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying XYZ prototype: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying XYZ prototype: " << ex.what();
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
    // User Story 2: Enhanced exception handling for libE57Format API calls
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            EXPECT_EQ(data3D.childCount(), 2) << "/data3D should contain two scans";

            if (data3D.childCount() >= 2) {
                // Verify first scan with defensive access
                e57::StructureNode scan1(data3D.get(0));
                if (scan1.isDefined("name")) {
                    e57::StringNode name1(scan1.get("name"));
                    EXPECT_EQ(name1.value(), "Scan 001") << "First scan name incorrect";
                }
                EXPECT_TRUE(scan1.isDefined("points")) << "First scan should have points CompressedVectorNode";

                // Verify second scan with defensive access
                e57::StructureNode scan2(data3D.get(1));
                if (scan2.isDefined("name")) {
                    e57::StringNode name2(scan2.get("name"));
                    EXPECT_EQ(name2.value(), "Scan 002") << "Second scan name incorrect";
                }
                EXPECT_TRUE(scan2.isDefined("points")) << "Second scan should have points CompressedVectorNode";
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying multiple scans: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying multiple scans: " << ex.what();
    }
}

// Sprint W2 Tests: Point Writing and Cartesian Bounds

/**
 * Test Case W2.1.1: Write a small set of XYZ points with known coordinates
 * Expected Result: Points written correctly and cartesian bounds calculated accurately
 */
TEST_F(E57WriterLibTest, WriteSmallSetOfXYZPoints) {
    // Create file, add scan, and define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Test Scan with Points")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Create test points with known coordinates
    std::vector<E57WriterLib::Point3D> testPoints = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };

    // Write points
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_TRUE(writer->writePoints(testPoints, xyzOnlyOptions)) << "Failed to write points: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            EXPECT_EQ(data3D.childCount(), 1) << "Should have one scan";

            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify points CompressedVectorNode
                EXPECT_TRUE(scan.isDefined("points")) << "Scan should have points";
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 3) << "Should have 3 points";
                }

                // Verify cartesian bounds
                EXPECT_TRUE(scan.isDefined("cartesianBounds")) << "Scan should have cartesian bounds";
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));

                    // Check all six bounds values
                    EXPECT_TRUE(bounds.isDefined("xMinimum")) << "Should have xMinimum";
                    EXPECT_TRUE(bounds.isDefined("xMaximum")) << "Should have xMaximum";
                    EXPECT_TRUE(bounds.isDefined("yMinimum")) << "Should have yMinimum";
                    EXPECT_TRUE(bounds.isDefined("yMaximum")) << "Should have yMaximum";
                    EXPECT_TRUE(bounds.isDefined("zMinimum")) << "Should have zMinimum";
                    EXPECT_TRUE(bounds.isDefined("zMaximum")) << "Should have zMaximum";

                    // Verify bounds values
                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        EXPECT_DOUBLE_EQ(xMin.value(), 1.0) << "xMinimum should be 1.0";
                        EXPECT_DOUBLE_EQ(xMax.value(), 7.0) << "xMaximum should be 7.0";
                    }

                    if (bounds.isDefined("yMinimum") && bounds.isDefined("yMaximum")) {
                        e57::FloatNode yMin(bounds.get("yMinimum"));
                        e57::FloatNode yMax(bounds.get("yMaximum"));
                        EXPECT_DOUBLE_EQ(yMin.value(), 2.0) << "yMinimum should be 2.0";
                        EXPECT_DOUBLE_EQ(yMax.value(), 8.0) << "yMaximum should be 8.0";
                    }

                    if (bounds.isDefined("zMinimum") && bounds.isDefined("zMaximum")) {
                        e57::FloatNode zMin(bounds.get("zMinimum"));
                        e57::FloatNode zMax(bounds.get("zMaximum"));
                        EXPECT_DOUBLE_EQ(zMin.value(), 3.0) << "zMinimum should be 3.0";
                        EXPECT_DOUBLE_EQ(zMax.value(), 9.0) << "zMaximum should be 9.0";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying written points: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying written points: " << ex.what();
    }
}

/**
 * Test Case W2.1.2: Write a larger dataset to test block-wise writing
 * Expected Result: Large dataset written correctly with proper point count
 */
TEST_F(E57WriterLibTest, WriteLargeDatasetBlockWise) {
    // Create file, add scan, and define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Large Dataset Scan")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Create a larger dataset (15,000 points to test multiple blocks)
    const size_t numPoints = 15000;
    std::vector<E57WriterLib::Point3D> testPoints;
    testPoints.reserve(numPoints);

    // Generate points in a known pattern
    for (size_t i = 0; i < numPoints; ++i) {
        double x = static_cast<double>(i) * 0.1;
        double y = static_cast<double>(i) * 0.2;
        double z = static_cast<double>(i) * 0.3;
        testPoints.emplace_back(x, y, z);
    }

    // Write points
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_TRUE(writer->writePoints(testPoints, xyzOnlyOptions)) << "Failed to write large dataset: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify point count
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), numPoints) << "Should have " << numPoints << " points";
                }

                // Verify bounds for the generated pattern
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));
                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        EXPECT_DOUBLE_EQ(xMin.value(), 0.0) << "xMinimum should be 0.0";
                        EXPECT_NEAR(xMax.value(), (numPoints - 1) * 0.1, 1e-10) << "xMaximum should match pattern";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying large dataset: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying large dataset: " << ex.what();
    }
}

/**
 * Test Case W2.1.3: Test writing zero points
 * Expected Result: Empty scan with valid cartesian bounds (all zeros)
 */
TEST_F(E57WriterLibTest, WriteZeroPoints) {
    // Create file, add scan, and define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Empty Scan")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Write empty point set
    std::vector<E57WriterLib::Point3D> emptyPoints;
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_TRUE(writer->writePoints(emptyPoints, xyzOnlyOptions)) << "Failed to write empty points: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify empty scan
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify zero points
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 0) << "Should have 0 points";
                }

                // Verify cartesian bounds exist with default values
                EXPECT_TRUE(scan.isDefined("cartesianBounds")) << "Should have cartesian bounds even for empty scan";
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));

                    // All bounds should be 0.0 for empty point set
                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        EXPECT_DOUBLE_EQ(xMin.value(), 0.0) << "xMinimum should be 0.0 for empty set";
                        EXPECT_DOUBLE_EQ(xMax.value(), 0.0) << "xMaximum should be 0.0 for empty set";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying empty scan: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying empty scan: " << ex.what();
    }
}

/**
 * Test Case W2.2.2: Test cartesian bounds with negative coordinates
 * Expected Result: Bounds correctly calculated for negative coordinate ranges
 */
TEST_F(E57WriterLibTest, CartesianBoundsWithNegativeCoordinates) {
    // Create file, add scan, and define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Negative Coords Scan")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Create test points with negative coordinates
    std::vector<E57WriterLib::Point3D> testPoints = {
        {-10.5, -20.5, -30.5},
        {-5.0, -15.0, -25.0},
        {-0.5, -1.5, -2.5}
    };

    // Write points
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_TRUE(writer->writePoints(testPoints, xyzOnlyOptions)) << "Failed to write points: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify bounds with negative values
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));

                    // Verify negative bounds
                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        EXPECT_DOUBLE_EQ(xMin.value(), -10.5) << "xMinimum should be -10.5";
                        EXPECT_DOUBLE_EQ(xMax.value(), -0.5) << "xMaximum should be -0.5";
                    }

                    if (bounds.isDefined("yMinimum") && bounds.isDefined("yMaximum")) {
                        e57::FloatNode yMin(bounds.get("yMinimum"));
                        e57::FloatNode yMax(bounds.get("yMaximum"));
                        EXPECT_DOUBLE_EQ(yMin.value(), -20.5) << "yMinimum should be -20.5";
                        EXPECT_DOUBLE_EQ(yMax.value(), -1.5) << "yMaximum should be -1.5";
                    }

                    if (bounds.isDefined("zMinimum") && bounds.isDefined("zMaximum")) {
                        e57::FloatNode zMin(bounds.get("zMinimum"));
                        e57::FloatNode zMax(bounds.get("zMaximum"));
                        EXPECT_DOUBLE_EQ(zMin.value(), -30.5) << "zMinimum should be -30.5";
                        EXPECT_DOUBLE_EQ(zMax.value(), -2.5) << "zMaximum should be -2.5";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying negative bounds: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying negative bounds: " << ex.what();
    }
}

/**
 * Test Case W2.2.3: Test cartesian bounds with single point
 * Expected Result: Min and max bounds should be equal for single point
 */
TEST_F(E57WriterLibTest, CartesianBoundsWithSinglePoint) {
    // Create file, add scan, and define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Single Point Scan")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Create single test point
    std::vector<E57WriterLib::Point3D> testPoints = {
        {7.7, 8.8, 9.9}
    };

    // Write points
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_TRUE(writer->writePoints(testPoints, xyzOnlyOptions)) << "Failed to write points: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify bounds for single point
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));

                    // For single point, min and max should be equal
                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        EXPECT_DOUBLE_EQ(xMin.value(), 7.7) << "xMinimum should be 7.7";
                        EXPECT_DOUBLE_EQ(xMax.value(), 7.7) << "xMaximum should be 7.7";
                    }

                    if (bounds.isDefined("yMinimum") && bounds.isDefined("yMaximum")) {
                        e57::FloatNode yMin(bounds.get("yMinimum"));
                        e57::FloatNode yMax(bounds.get("yMaximum"));
                        EXPECT_DOUBLE_EQ(yMin.value(), 8.8) << "yMinimum should be 8.8";
                        EXPECT_DOUBLE_EQ(yMax.value(), 8.8) << "yMaximum should be 8.8";
                    }

                    if (bounds.isDefined("zMinimum") && bounds.isDefined("zMaximum")) {
                        e57::FloatNode zMin(bounds.get("zMinimum"));
                        e57::FloatNode zMax(bounds.get("zMaximum"));
                        EXPECT_DOUBLE_EQ(zMin.value(), 9.9) << "zMinimum should be 9.9";
                        EXPECT_DOUBLE_EQ(zMax.value(), 9.9) << "zMaximum should be 9.9";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying single point bounds: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying single point bounds: " << ex.what();
    }
}

/**
 * Test error handling for writing points without prototype
 */
TEST_F(E57WriterLibTest, ErrorHandlingWritePointsWithoutPrototype) {
    // Create file and add scan but don't define prototype
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Scan Without Prototype")) << "Failed to add scan";

    // Try to write points without defining prototype first
    std::vector<E57WriterLib::Point3D> testPoints = {{1.0, 2.0, 3.0}};
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_FALSE(writer->writePoints(testPoints, xyzOnlyOptions)) << "writePoints should fail without prototype";
    EXPECT_FALSE(writer->getLastError().isEmpty()) << "Error message should be set";
}

/**
 * Test error handling for writing points to invalid scan index
 */
TEST_F(E57WriterLibTest, ErrorHandlingWritePointsInvalidScanIndex) {
    // Create file with one scan
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Valid Scan")) << "Failed to add scan";
    EXPECT_TRUE(writer->defineXYZPrototype()) << "Failed to define XYZ prototype";

    // Try to write to invalid scan index
    std::vector<E57WriterLib::Point3D> testPoints = {{1.0, 2.0, 3.0}};
    E57WriterLib::ExportOptions xyzOnlyOptions(false, false); // XYZ only for backward compatibility
    EXPECT_FALSE(writer->writePoints(1, testPoints, xyzOnlyOptions)) << "writePoints should fail for invalid scan index";
    EXPECT_FALSE(writer->getLastError().isEmpty()) << "Error message should be set";
}

// Sprint W3 Tests: Intensity and Color Data Support

/**
 * Test Case W3.1.1: Generate an E57 file with intensity export enabled
 * Expected Result: XML dump shows intensity field in prototype as FloatNode
 */
TEST_F(E57WriterLibTest, DefinePrototypeWithIntensityEnabled) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Intensity Test Scan")) << "Failed to add scan";

    // Define prototype with intensity enabled
    E57WriterLib::ExportOptions options(true, false); // intensity=true, color=false
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define prototype with intensity";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify prototype structure
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    e57::StructureNode prototype(pointsNode.prototype());

                    // Verify XYZ fields exist
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";

                    // Verify intensity field exists and is FloatNode
                    EXPECT_TRUE(prototype.isDefined("intensity")) << "Prototype should have intensity field";
                    if (prototype.isDefined("intensity")) {
                        e57::FloatNode intensityNode(prototype.get("intensity"));
                        EXPECT_EQ(intensityNode.precision(), e57::PrecisionSingle) << "Intensity should be single precision";
                        EXPECT_DOUBLE_EQ(intensityNode.minimum(), 0.0) << "Intensity minimum should be 0.0";
                        EXPECT_DOUBLE_EQ(intensityNode.maximum(), 1.0) << "Intensity maximum should be 1.0";
                    }

                    // Verify color fields do NOT exist
                    EXPECT_FALSE(prototype.isDefined("colorRed")) << "Prototype should not have colorRed";
                    EXPECT_FALSE(prototype.isDefined("colorGreen")) << "Prototype should not have colorGreen";
                    EXPECT_FALSE(prototype.isDefined("colorBlue")) << "Prototype should not have colorBlue";
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying intensity prototype: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying intensity prototype: " << ex.what();
    }
}

/**
 * Test Case W3.2.1: Generate an E57 file with color export enabled
 * Expected Result: XML dump shows color fields in prototype as IntegerNodes
 */
TEST_F(E57WriterLibTest, DefinePrototypeWithColorEnabled) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Color Test Scan")) << "Failed to add scan";

    // Define prototype with color enabled
    E57WriterLib::ExportOptions options(false, true); // intensity=false, color=true
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define prototype with color";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify prototype structure
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    e57::StructureNode prototype(pointsNode.prototype());

                    // Verify XYZ fields exist
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";

                    // Verify color fields exist and are IntegerNodes
                    EXPECT_TRUE(prototype.isDefined("colorRed")) << "Prototype should have colorRed field";
                    EXPECT_TRUE(prototype.isDefined("colorGreen")) << "Prototype should have colorGreen field";
                    EXPECT_TRUE(prototype.isDefined("colorBlue")) << "Prototype should have colorBlue field";

                    if (prototype.isDefined("colorRed") && prototype.isDefined("colorGreen") && prototype.isDefined("colorBlue")) {
                        e57::IntegerNode redNode(prototype.get("colorRed"));
                        e57::IntegerNode greenNode(prototype.get("colorGreen"));
                        e57::IntegerNode blueNode(prototype.get("colorBlue"));

                        EXPECT_EQ(redNode.minimum(), 0) << "colorRed minimum should be 0";
                        EXPECT_EQ(redNode.maximum(), 255) << "colorRed maximum should be 255";
                        EXPECT_EQ(greenNode.minimum(), 0) << "colorGreen minimum should be 0";
                        EXPECT_EQ(greenNode.maximum(), 255) << "colorGreen maximum should be 255";
                        EXPECT_EQ(blueNode.minimum(), 0) << "colorBlue minimum should be 0";
                        EXPECT_EQ(blueNode.maximum(), 255) << "colorBlue maximum should be 255";
                    }

                    // Verify intensity field does NOT exist
                    EXPECT_FALSE(prototype.isDefined("intensity")) << "Prototype should not have intensity";
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying color prototype: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying color prototype: " << ex.what();
    }
}

/**
 * Test Case W3.3.1: Write points with XYZ and Intensity (no color)
 * Expected Result: E57 file contains XYZ and Intensity with correct limits
 */
TEST_F(E57WriterLibTest, WritePointsWithIntensityOnly) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Intensity Points Scan")) << "Failed to add scan";

    // Define prototype with intensity enabled
    E57WriterLib::ExportOptions options(true, false); // intensity=true, color=false
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define prototype with intensity";

    // Create test points with intensity data
    std::vector<E57WriterLib::Point3D> testPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 0.1f),  // XYZ + intensity
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0.5f),  // XYZ + intensity
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0.9f)   // XYZ + intensity
    };

    // Write points with intensity
    EXPECT_TRUE(writer->writePoints(testPoints, options)) << "Failed to write points with intensity: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify points count
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 3) << "Should have 3 points";
                }

                // Verify intensity limits
                EXPECT_TRUE(scan.isDefined("intensityLimits")) << "Scan should have intensity limits";
                if (scan.isDefined("intensityLimits")) {
                    e57::StructureNode intensityLimits(scan.get("intensityLimits"));
                    EXPECT_TRUE(intensityLimits.isDefined("intensityMinimum")) << "Should have intensityMinimum";
                    EXPECT_TRUE(intensityLimits.isDefined("intensityMaximum")) << "Should have intensityMaximum";

                    if (intensityLimits.isDefined("intensityMinimum") && intensityLimits.isDefined("intensityMaximum")) {
                        e57::FloatNode minNode(intensityLimits.get("intensityMinimum"));
                        e57::FloatNode maxNode(intensityLimits.get("intensityMaximum"));
                        EXPECT_FLOAT_EQ(minNode.value(), 0.1f) << "intensityMinimum should be 0.1";
                        EXPECT_FLOAT_EQ(maxNode.value(), 0.9f) << "intensityMaximum should be 0.9";
                    }
                }

                // Verify color limits do NOT exist
                EXPECT_FALSE(scan.isDefined("colorLimits")) << "Scan should not have color limits";
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying intensity points: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying intensity points: " << ex.what();
    }
}

/**
 * Test Case W3.3.2: Write points with XYZ and RGB Color (no intensity)
 * Expected Result: E57 file contains XYZ and RGB with correct limits
 */
TEST_F(E57WriterLibTest, WritePointsWithColorOnly) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Color Points Scan")) << "Failed to add scan";

    // Define prototype with color enabled
    E57WriterLib::ExportOptions options(false, true); // intensity=false, color=true
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define prototype with color";

    // Create test points with color data
    std::vector<E57WriterLib::Point3D> testPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 255, 0, 0),    // XYZ + red
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0, 255, 0),    // XYZ + green
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0, 0, 255)     // XYZ + blue
    };

    // Write points with color
    EXPECT_TRUE(writer->writePoints(testPoints, options)) << "Failed to write points with color: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify points count
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 3) << "Should have 3 points";
                }

                // Verify color limits
                EXPECT_TRUE(scan.isDefined("colorLimits")) << "Scan should have color limits";
                if (scan.isDefined("colorLimits")) {
                    e57::StructureNode colorLimits(scan.get("colorLimits"));

                    // Check all color channel limits
                    EXPECT_TRUE(colorLimits.isDefined("colorRedMinimum")) << "Should have colorRedMinimum";
                    EXPECT_TRUE(colorLimits.isDefined("colorRedMaximum")) << "Should have colorRedMaximum";
                    EXPECT_TRUE(colorLimits.isDefined("colorGreenMinimum")) << "Should have colorGreenMinimum";
                    EXPECT_TRUE(colorLimits.isDefined("colorGreenMaximum")) << "Should have colorGreenMaximum";
                    EXPECT_TRUE(colorLimits.isDefined("colorBlueMinimum")) << "Should have colorBlueMinimum";
                    EXPECT_TRUE(colorLimits.isDefined("colorBlueMaximum")) << "Should have colorBlueMaximum";

                    // Verify red channel limits (0-255)
                    if (colorLimits.isDefined("colorRedMinimum") && colorLimits.isDefined("colorRedMaximum")) {
                        e57::IntegerNode redMin(colorLimits.get("colorRedMinimum"));
                        e57::IntegerNode redMax(colorLimits.get("colorRedMaximum"));
                        EXPECT_EQ(redMin.value(), 0) << "colorRedMinimum should be 0";
                        EXPECT_EQ(redMax.value(), 255) << "colorRedMaximum should be 255";
                    }

                    // Verify green channel limits (0-255)
                    if (colorLimits.isDefined("colorGreenMinimum") && colorLimits.isDefined("colorGreenMaximum")) {
                        e57::IntegerNode greenMin(colorLimits.get("colorGreenMinimum"));
                        e57::IntegerNode greenMax(colorLimits.get("colorGreenMaximum"));
                        EXPECT_EQ(greenMin.value(), 0) << "colorGreenMinimum should be 0";
                        EXPECT_EQ(greenMax.value(), 255) << "colorGreenMaximum should be 255";
                    }

                    // Verify blue channel limits (0-255)
                    if (colorLimits.isDefined("colorBlueMinimum") && colorLimits.isDefined("colorBlueMaximum")) {
                        e57::IntegerNode blueMin(colorLimits.get("colorBlueMinimum"));
                        e57::IntegerNode blueMax(colorLimits.get("colorBlueMaximum"));
                        EXPECT_EQ(blueMin.value(), 0) << "colorBlueMinimum should be 0";
                        EXPECT_EQ(blueMax.value(), 255) << "colorBlueMaximum should be 255";
                    }
                }

                // Verify intensity limits do NOT exist
                EXPECT_FALSE(scan.isDefined("intensityLimits")) << "Scan should not have intensity limits";
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying color points: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying color points: " << ex.what();
    }
}

/**
 * Test Case W3.3.3: Write points with XYZ, Intensity, and RGB Color
 * Expected Result: E57 file contains all attributes with correct limits
 */
TEST_F(E57WriterLibTest, WritePointsWithIntensityAndColor) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Full Attributes Scan")) << "Failed to add scan";

    // Define prototype with both intensity and color enabled
    E57WriterLib::ExportOptions options(true, true); // intensity=true, color=true
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define prototype with intensity and color";

    // Create test points with both intensity and color data
    std::vector<E57WriterLib::Point3D> testPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 0.2f, 255, 128, 64),  // XYZ + intensity + color
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0.6f, 128, 255, 32),  // XYZ + intensity + color
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0.8f, 64, 32, 255)    // XYZ + intensity + color
    };

    // Write points with both attributes
    EXPECT_TRUE(writer->writePoints(testPoints, options)) << "Failed to write points with intensity and color: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify points count
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 3) << "Should have 3 points";

                    // Verify prototype has all fields
                    e57::StructureNode prototype(pointsNode.prototype());
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";
                    EXPECT_TRUE(prototype.isDefined("intensity")) << "Prototype should have intensity";
                    EXPECT_TRUE(prototype.isDefined("colorRed")) << "Prototype should have colorRed";
                    EXPECT_TRUE(prototype.isDefined("colorGreen")) << "Prototype should have colorGreen";
                    EXPECT_TRUE(prototype.isDefined("colorBlue")) << "Prototype should have colorBlue";
                }

                // Verify intensity limits
                EXPECT_TRUE(scan.isDefined("intensityLimits")) << "Scan should have intensity limits";
                if (scan.isDefined("intensityLimits")) {
                    e57::StructureNode intensityLimits(scan.get("intensityLimits"));
                    if (intensityLimits.isDefined("intensityMinimum") && intensityLimits.isDefined("intensityMaximum")) {
                        e57::FloatNode minNode(intensityLimits.get("intensityMinimum"));
                        e57::FloatNode maxNode(intensityLimits.get("intensityMaximum"));
                        EXPECT_FLOAT_EQ(minNode.value(), 0.2f) << "intensityMinimum should be 0.2";
                        EXPECT_FLOAT_EQ(maxNode.value(), 0.8f) << "intensityMaximum should be 0.8";
                    }
                }

                // Verify color limits
                EXPECT_TRUE(scan.isDefined("colorLimits")) << "Scan should have color limits";
                if (scan.isDefined("colorLimits")) {
                    e57::StructureNode colorLimits(scan.get("colorLimits"));

                    // Verify red channel limits (64-255)
                    if (colorLimits.isDefined("colorRedMinimum") && colorLimits.isDefined("colorRedMaximum")) {
                        e57::IntegerNode redMin(colorLimits.get("colorRedMinimum"));
                        e57::IntegerNode redMax(colorLimits.get("colorRedMaximum"));
                        EXPECT_EQ(redMin.value(), 64) << "colorRedMinimum should be 64";
                        EXPECT_EQ(redMax.value(), 255) << "colorRedMaximum should be 255";
                    }

                    // Verify green channel limits (32-255)
                    if (colorLimits.isDefined("colorGreenMinimum") && colorLimits.isDefined("colorGreenMaximum")) {
                        e57::IntegerNode greenMin(colorLimits.get("colorGreenMinimum"));
                        e57::IntegerNode greenMax(colorLimits.get("colorGreenMaximum"));
                        EXPECT_EQ(greenMin.value(), 32) << "colorGreenMinimum should be 32";
                        EXPECT_EQ(greenMax.value(), 255) << "colorGreenMaximum should be 255";
                    }

                    // Verify blue channel limits (32-255)
                    if (colorLimits.isDefined("colorBlueMinimum") && colorLimits.isDefined("colorBlueMaximum")) {
                        e57::IntegerNode blueMin(colorLimits.get("colorBlueMinimum"));
                        e57::IntegerNode blueMax(colorLimits.get("colorBlueMaximum"));
                        EXPECT_EQ(blueMin.value(), 32) << "colorBlueMinimum should be 32";
                        EXPECT_EQ(blueMax.value(), 255) << "colorBlueMaximum should be 255";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying full attributes points: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying full attributes points: " << ex.what();
    }
}

/**
 * Test Case W3.5.1: Test export configuration flags
 * Expected Result: Export options correctly control which attributes are included
 */
TEST_F(E57WriterLibTest, ExportConfigurationFlags) {
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan("Configuration Test Scan")) << "Failed to add scan";

    // Define prototype with both intensity and color disabled
    E57WriterLib::ExportOptions options(false, false); // intensity=false, color=false
    EXPECT_TRUE(writer->definePointPrototype(options)) << "Failed to define XYZ-only prototype";

    // Create test points with both intensity and color data (but export flags are disabled)
    std::vector<E57WriterLib::Point3D> testPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 0.5f, 255, 128, 64)  // XYZ + intensity + color (but won't be exported)
    };

    // Write points with disabled export options
    EXPECT_TRUE(writer->writePoints(testPoints, options)) << "Failed to write points with disabled options: " << writer->getLastError().toStdString();
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify only XYZ is exported
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify points exist
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), 1) << "Should have 1 point";

                    // Verify prototype has only XYZ fields
                    e57::StructureNode prototype(pointsNode.prototype());
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Prototype should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Prototype should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Prototype should have cartesianZ";
                    EXPECT_FALSE(prototype.isDefined("intensity")) << "Prototype should NOT have intensity";
                    EXPECT_FALSE(prototype.isDefined("colorRed")) << "Prototype should NOT have colorRed";
                    EXPECT_FALSE(prototype.isDefined("colorGreen")) << "Prototype should NOT have colorGreen";
                    EXPECT_FALSE(prototype.isDefined("colorBlue")) << "Prototype should NOT have colorBlue";
                }

                // Verify no intensity or color limits
                EXPECT_FALSE(scan.isDefined("intensityLimits")) << "Scan should NOT have intensity limits";
                EXPECT_FALSE(scan.isDefined("colorLimits")) << "Scan should NOT have color limits";

                // Verify cartesian bounds still exist
                EXPECT_TRUE(scan.isDefined("cartesianBounds")) << "Scan should have cartesian bounds";
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying configuration flags: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying configuration flags: " << ex.what();
    }
}

// Sprint W4 Tests: Pose Metadata, Multi-Scan Support, and Enhanced Metadata

/**
 * Test Case W4.1.1: Write scanner pose metadata to E57 Data3D header
 * Expected Result: Pose information correctly written and readable
 */
TEST_F(E57WriterLibTest, WriteScannerPoseMetadata) {
    // Create scan metadata with pose information
    E57WriterLib::ScanPose pose;
    pose.translation = QVector3D(10.5f, 20.3f, 5.7f);
    pose.rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 45.0f); // 45 degree rotation around Z-axis
    pose.acquisitionTime = QDateTime::currentDateTime();

    E57WriterLib::ScanMetadata metadata;
    metadata.name = "Pose Test Scan";
    metadata.description = "Test scan with pose metadata";
    metadata.sensorModel = "Test Scanner v1.0";
    metadata.pose = pose;
    metadata.acquisitionStart = QDateTime::currentDateTime();

    // Create file and add scan with metadata
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan(metadata)) << "Failed to add scan with metadata";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify pose metadata by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            EXPECT_EQ(data3D.childCount(), 1) << "Should have one scan";

            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));

                // Verify pose structure exists
                EXPECT_TRUE(scan.isDefined("pose")) << "Scan should have pose metadata";

                if (scan.isDefined("pose")) {
                    e57::StructureNode poseNode(scan.get("pose"));

                    // Verify translation
                    EXPECT_TRUE(poseNode.isDefined("translation")) << "Pose should have translation";
                    if (poseNode.isDefined("translation")) {
                        e57::StructureNode translation(poseNode.get("translation"));
                        EXPECT_TRUE(translation.isDefined("x")) << "Translation should have x";
                        EXPECT_TRUE(translation.isDefined("y")) << "Translation should have y";
                        EXPECT_TRUE(translation.isDefined("z")) << "Translation should have z";

                        if (translation.isDefined("x") && translation.isDefined("y") && translation.isDefined("z")) {
                            e57::FloatNode xNode(translation.get("x"));
                            e57::FloatNode yNode(translation.get("y"));
                            e57::FloatNode zNode(translation.get("z"));

                            EXPECT_DOUBLE_EQ(xNode.value(), 10.5) << "Translation X should match";
                            EXPECT_DOUBLE_EQ(yNode.value(), 20.3) << "Translation Y should match";
                            EXPECT_DOUBLE_EQ(zNode.value(), 5.7) << "Translation Z should match";
                        }
                    }

                    // Verify rotation (quaternion)
                    EXPECT_TRUE(poseNode.isDefined("rotation")) << "Pose should have rotation";
                    if (poseNode.isDefined("rotation")) {
                        e57::StructureNode rotation(poseNode.get("rotation"));
                        EXPECT_TRUE(rotation.isDefined("w")) << "Rotation should have w";
                        EXPECT_TRUE(rotation.isDefined("x")) << "Rotation should have x";
                        EXPECT_TRUE(rotation.isDefined("y")) << "Rotation should have y";
                        EXPECT_TRUE(rotation.isDefined("z")) << "Rotation should have z";

                        if (rotation.isDefined("w") && rotation.isDefined("x") &&
                            rotation.isDefined("y") && rotation.isDefined("z")) {
                            e57::FloatNode wNode(rotation.get("w"));
                            e57::FloatNode xNode(rotation.get("x"));
                            e57::FloatNode yNode(rotation.get("y"));
                            e57::FloatNode zNode(rotation.get("z"));

                            // Verify quaternion is normalized and matches expected values
                            QQuaternion readQuaternion(wNode.value(), xNode.value(), yNode.value(), zNode.value());
                            QQuaternion expectedQuaternion = pose.rotation.normalized();

                            EXPECT_NEAR(readQuaternion.scalar(), expectedQuaternion.scalar(), 1e-10) << "Quaternion W should match";
                            EXPECT_NEAR(readQuaternion.x(), expectedQuaternion.x(), 1e-10) << "Quaternion X should match";
                            EXPECT_NEAR(readQuaternion.y(), expectedQuaternion.y(), 1e-10) << "Quaternion Y should match";
                            EXPECT_NEAR(readQuaternion.z(), expectedQuaternion.z(), 1e-10) << "Quaternion Z should match";
                        }
                    }
                }

                // Verify other metadata fields
                EXPECT_TRUE(scan.isDefined("description")) << "Scan should have description";
                EXPECT_TRUE(scan.isDefined("sensorModel")) << "Scan should have sensorModel";
                EXPECT_TRUE(scan.isDefined("acquisitionStart")) << "Scan should have acquisitionStart";

                if (scan.isDefined("description")) {
                    e57::StringNode descNode(scan.get("description"));
                    EXPECT_EQ(descNode.value(), "Test scan with pose metadata") << "Description should match";
                }

                if (scan.isDefined("sensorModel")) {
                    e57::StringNode sensorNode(scan.get("sensorModel"));
                    EXPECT_EQ(sensorNode.value(), "Test Scanner v1.0") << "Sensor model should match";
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying pose metadata: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying pose metadata: " << ex.what();
    }
}

/**
 * Test Case W4.2.1: Support multiple scans in single E57 file
 * Expected Result: Multiple scans with different poses and metadata correctly written
 */
TEST_F(E57WriterLibTest, WriteMultipleScansWithMetadata) {
    // Create multiple scan data with different poses
    std::vector<E57WriterLib::ScanData> scansData;

    // Scan 1
    E57WriterLib::ScanMetadata metadata1;
    metadata1.name = "Scan 001";
    metadata1.description = "First scan position";
    metadata1.sensorModel = "FARO Focus S350";
    metadata1.pose.translation = QVector3D(0.0f, 0.0f, 0.0f);
    metadata1.pose.rotation = QQuaternion(); // Identity rotation
    metadata1.acquisitionStart = QDateTime::currentDateTime().addSecs(-3600); // 1 hour ago

    std::vector<E57WriterLib::Point3D> points1 = {
        {1.0, 2.0, 3.0, 0.5f, 255, 128, 64},
        {4.0, 5.0, 6.0, 0.7f, 128, 255, 32}
    };

    E57WriterLib::ExportOptions options1(true, true); // Include intensity and color
    scansData.emplace_back(metadata1, points1, options1);

    // Scan 2
    E57WriterLib::ScanMetadata metadata2;
    metadata2.name = "Scan 002";
    metadata2.description = "Second scan position";
    metadata2.sensorModel = "FARO Focus S350";
    metadata2.pose.translation = QVector3D(10.0f, 5.0f, 2.0f);
    metadata2.pose.rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f); // 90 degree rotation
    metadata2.acquisitionStart = QDateTime::currentDateTime().addSecs(-1800); // 30 minutes ago

    std::vector<E57WriterLib::Point3D> points2 = {
        {7.0, 8.0, 9.0, 0.3f},  // XYZ + intensity only
        {10.0, 11.0, 12.0, 0.9f}
    };

    E57WriterLib::ExportOptions options2(true, false); // Include intensity only
    scansData.emplace_back(metadata2, points2, options2);

    // Scan 3 - XYZ only
    E57WriterLib::ScanMetadata metadata3;
    metadata3.name = "Scan 003";
    metadata3.description = "Third scan position - XYZ only";
    metadata3.sensorModel = "Leica BLK360";
    metadata3.pose.translation = QVector3D(-5.0f, 10.0f, 1.0f);
    metadata3.pose.rotation = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 30.0f); // 30 degree rotation around X
    metadata3.acquisitionStart = QDateTime::currentDateTime();

    std::vector<E57WriterLib::Point3D> points3 = {
        {13.0, 14.0, 15.0},
        {16.0, 17.0, 18.0},
        {19.0, 20.0, 21.0}
    };

    E57WriterLib::ExportOptions options3(false, false); // XYZ only
    scansData.emplace_back(metadata3, points3, options3);

    // Write multiple scans
    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->writeMultipleScans(scansData)) << "Failed to write multiple scans: " << writer->getLastError().toStdString();
    EXPECT_EQ(writer->getScanCount(), 3) << "Should have 3 scans";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify multiple scans by reading back
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            EXPECT_EQ(data3D.childCount(), 3) << "Should have 3 scans";

            // Verify each scan
            for (int i = 0; i < 3; ++i) {
                e57::StructureNode scan(data3D.get(i));

                // Verify scan name
                EXPECT_TRUE(scan.isDefined("name")) << "Scan " << i << " should have name";
                if (scan.isDefined("name")) {
                    e57::StringNode nameNode(scan.get("name"));
                    QString expectedName = QString("Scan %1").arg(i + 1, 3, 10, QChar('0'));
                    EXPECT_EQ(nameNode.value(), expectedName.toStdString()) << "Scan " << i << " name should match";
                }

                // Verify pose exists
                EXPECT_TRUE(scan.isDefined("pose")) << "Scan " << i << " should have pose";

                // Verify points exist
                EXPECT_TRUE(scan.isDefined("points")) << "Scan " << i << " should have points";
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    EXPECT_EQ(pointsNode.childCount(), scansData[i].points.size()) << "Scan " << i << " point count should match";

                    // Verify prototype based on options
                    e57::StructureNode prototype(pointsNode.prototype());
                    EXPECT_TRUE(prototype.isDefined("cartesianX")) << "Scan " << i << " should have cartesianX";
                    EXPECT_TRUE(prototype.isDefined("cartesianY")) << "Scan " << i << " should have cartesianY";
                    EXPECT_TRUE(prototype.isDefined("cartesianZ")) << "Scan " << i << " should have cartesianZ";

                    if (scansData[i].options.includeIntensity) {
                        EXPECT_TRUE(prototype.isDefined("intensity")) << "Scan " << i << " should have intensity";
                        EXPECT_TRUE(scan.isDefined("intensityLimits")) << "Scan " << i << " should have intensity limits";
                    } else {
                        EXPECT_FALSE(prototype.isDefined("intensity")) << "Scan " << i << " should not have intensity";
                        EXPECT_FALSE(scan.isDefined("intensityLimits")) << "Scan " << i << " should not have intensity limits";
                    }

                    if (scansData[i].options.includeColor) {
                        EXPECT_TRUE(prototype.isDefined("colorRed")) << "Scan " << i << " should have colorRed";
                        EXPECT_TRUE(prototype.isDefined("colorGreen")) << "Scan " << i << " should have colorGreen";
                        EXPECT_TRUE(prototype.isDefined("colorBlue")) << "Scan " << i << " should have colorBlue";
                        EXPECT_TRUE(scan.isDefined("colorLimits")) << "Scan " << i << " should have color limits";
                    } else {
                        EXPECT_FALSE(prototype.isDefined("colorRed")) << "Scan " << i << " should not have colorRed";
                        EXPECT_FALSE(scan.isDefined("colorLimits")) << "Scan " << i << " should not have color limits";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying multiple scans: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying multiple scans: " << ex.what();
    }
}

/**
 * Test Case W4.3.1: Test ScanPose matrix conversion utilities
 * Expected Result: Pose conversion between matrix and quaternion/translation works correctly
 */
TEST_F(E57WriterLibTest, ScanPoseMatrixConversion) {
    // Create a test transformation matrix
    QMatrix4x4 originalMatrix;
    originalMatrix.setToIdentity();
    originalMatrix.translate(5.0f, 10.0f, 15.0f);
    originalMatrix.rotate(45.0f, 0.0f, 0.0f, 1.0f); // 45 degrees around Z-axis

    // Convert matrix to pose
    E57WriterLib::ScanPose pose = E57WriterLib::ScanPose::fromMatrix(originalMatrix);

    // Verify translation
    EXPECT_NEAR(pose.translation.x(), 5.0f, 1e-6) << "Translation X should match";
    EXPECT_NEAR(pose.translation.y(), 10.0f, 1e-6) << "Translation Y should match";
    EXPECT_NEAR(pose.translation.z(), 15.0f, 1e-6) << "Translation Z should match";

    // Verify quaternion is normalized
    float quaternionLength = sqrt(pose.rotation.scalar() * pose.rotation.scalar() +
                                 pose.rotation.x() * pose.rotation.x() +
                                 pose.rotation.y() * pose.rotation.y() +
                                 pose.rotation.z() * pose.rotation.z());
    EXPECT_NEAR(quaternionLength, 1.0, 1e-6) << "Quaternion should be normalized";

    // Convert pose back to matrix
    QMatrix4x4 convertedMatrix = pose.toMatrix();

    // Verify round-trip conversion accuracy
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            EXPECT_NEAR(originalMatrix(row, col), convertedMatrix(row, col), 1e-5)
                << "Matrix element (" << row << "," << col << ") should match after round-trip conversion";
        }
    }

    // Test with the pose in an actual E57 file
    E57WriterLib::ScanMetadata metadata;
    metadata.name = "Matrix Conversion Test";
    metadata.pose = pose;

    EXPECT_TRUE(writer->createFile(testFilePath)) << "Failed to create E57 file";
    EXPECT_TRUE(writer->addScan(metadata)) << "Failed to add scan with pose";
    EXPECT_TRUE(writer->closeFile()) << "Failed to close file";

    // Verify the pose was written correctly
    try {
        e57::ImageFile testFile(testFilePath.toStdString(), "r");
        e57::StructureNode root = testFile.root();

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                if (scan.isDefined("pose")) {
                    e57::StructureNode poseNode(scan.get("pose"));

                    // Read back translation
                    if (poseNode.isDefined("translation")) {
                        e57::StructureNode translation(poseNode.get("translation"));
                        e57::FloatNode xNode(translation.get("x"));
                        e57::FloatNode yNode(translation.get("y"));
                        e57::FloatNode zNode(translation.get("z"));

                        EXPECT_NEAR(xNode.value(), 5.0, 1e-6) << "Stored translation X should match";
                        EXPECT_NEAR(yNode.value(), 10.0, 1e-6) << "Stored translation Y should match";
                        EXPECT_NEAR(zNode.value(), 15.0, 1e-6) << "Stored translation Z should match";
                    }

                    // Read back rotation and verify normalization
                    if (poseNode.isDefined("rotation")) {
                        e57::StructureNode rotation(poseNode.get("rotation"));
                        e57::FloatNode wNode(rotation.get("w"));
                        e57::FloatNode xNode(rotation.get("x"));
                        e57::FloatNode yNode(rotation.get("y"));
                        e57::FloatNode zNode(rotation.get("z"));

                        double storedLength = sqrt(wNode.value() * wNode.value() +
                                                  xNode.value() * xNode.value() +
                                                  yNode.value() * yNode.value() +
                                                  zNode.value() * zNode.value());
                        EXPECT_NEAR(storedLength, 1.0, 1e-10) << "Stored quaternion should be normalized";
                    }
                }
            }
        }

        testFile.close();
    } catch (const e57::E57Exception& ex) {
        FAIL() << "E57 Exception when verifying matrix conversion: " << ex.what()
               << " (Error code: " << ex.errorCode() << ")";
    } catch (const std::exception& ex) {
        FAIL() << "Standard exception when verifying matrix conversion: " << ex.what();
    }
}
