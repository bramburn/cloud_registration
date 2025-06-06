#include <iostream>
#include <QCoreApplication>
#include <QDateTime>
#include <QVector3D>
#include <QQuaternion>
#include <QFileInfo>
#include "src/e57writer_lib.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "=== W4 Sprint Simple Test ===" << std::endl;
    
    try {
        // Test 1: Basic file creation with metadata
        std::cout << "\n1. Testing basic file creation with W4 metadata..." << std::endl;
        
        E57WriterLib writer;
        QString testFile = "test_w4_simple.e57";
        
        if (!writer.createFile(testFile)) {
            std::cout << "FAILED: Could not create file: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        std::cout << "SUCCESS: File created" << std::endl;
        
        // Test 2: Add scan with pose metadata
        std::cout << "\n2. Testing scan with pose metadata..." << std::endl;
        
        E57WriterLib::ScanMetadata metadata;
        metadata.name = "Test Scan W4";
        metadata.description = "W4 Sprint test scan";
        metadata.sensorModel = "Test Scanner v1.0";
        metadata.pose.translation = QVector3D(1.0f, 2.0f, 3.0f);
        metadata.pose.rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 45.0f);
        metadata.acquisitionStart = QDateTime::currentDateTime();
        
        if (!writer.addScan(metadata)) {
            std::cout << "FAILED: Could not add scan: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        std::cout << "SUCCESS: Scan with metadata added" << std::endl;
        
        // Test 3: Define prototype and write points
        std::cout << "\n3. Testing point writing..." << std::endl;
        
        E57WriterLib::ExportOptions options(true, true); // Include intensity and color
        if (!writer.definePointPrototype(options)) {
            std::cout << "FAILED: Could not define prototype: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        
        std::vector<E57WriterLib::Point3D> points = {
            {1.0, 2.0, 3.0, 0.5f, 255, 128, 64},
            {4.0, 5.0, 6.0, 0.7f, 128, 255, 32}
        };
        
        if (!writer.writePoints(points, options)) {
            std::cout << "FAILED: Could not write points: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        std::cout << "SUCCESS: Points written" << std::endl;
        
        // Test 4: Close file
        std::cout << "\n4. Testing file closure..." << std::endl;
        
        if (!writer.closeFile()) {
            std::cout << "FAILED: Could not close file: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        std::cout << "SUCCESS: File closed" << std::endl;
        
        // Test 5: Verify file exists and has content
        std::cout << "\n5. Testing file verification..." << std::endl;
        
        QFileInfo fileInfo(testFile);
        if (!fileInfo.exists()) {
            std::cout << "FAILED: File does not exist" << std::endl;
            return 1;
        }
        
        if (fileInfo.size() == 0) {
            std::cout << "FAILED: File is empty" << std::endl;
            return 1;
        }
        
        std::cout << "SUCCESS: File exists with size " << fileInfo.size() << " bytes" << std::endl;
        
        // Test 6: Test multiple scans
        std::cout << "\n6. Testing multiple scans..." << std::endl;
        
        std::vector<E57WriterLib::ScanData> scansData;
        
        // Scan 1
        E57WriterLib::ScanMetadata meta1;
        meta1.name = "Scan 001";
        meta1.pose.translation = QVector3D(0, 0, 0);
        meta1.pose.rotation = QQuaternion();
        meta1.acquisitionStart = QDateTime::currentDateTime();
        
        std::vector<E57WriterLib::Point3D> points1 = {{1.0, 2.0, 3.0}};
        E57WriterLib::ExportOptions opts1(false, false);
        scansData.emplace_back(meta1, points1, opts1);
        
        // Scan 2
        E57WriterLib::ScanMetadata meta2;
        meta2.name = "Scan 002";
        meta2.pose.translation = QVector3D(10, 0, 0);
        meta2.pose.rotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f);
        meta2.acquisitionStart = QDateTime::currentDateTime();
        
        std::vector<E57WriterLib::Point3D> points2 = {{4.0, 5.0, 6.0}};
        E57WriterLib::ExportOptions opts2(false, false);
        scansData.emplace_back(meta2, points2, opts2);
        
        QString multiScanFile = "test_w4_multiscan.e57";
        if (!writer.createFile(multiScanFile)) {
            std::cout << "FAILED: Could not create multi-scan file: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        
        if (!writer.writeMultipleScans(scansData)) {
            std::cout << "FAILED: Could not write multiple scans: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        
        if (!writer.closeFile()) {
            std::cout << "FAILED: Could not close multi-scan file: " << writer.getLastError().toStdString() << std::endl;
            return 1;
        }
        
        std::cout << "SUCCESS: Multiple scans written" << std::endl;
        
        std::cout << "\n=== ALL W4 TESTS PASSED ===" << std::endl;
        return 0;
        
    } catch (const std::exception& ex) {
        std::cout << "EXCEPTION: " << ex.what() << std::endl;
        return 1;
    }
}
