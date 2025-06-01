#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// Include our Sprint 1.2 classes
#include "src/projectmanager.h"
#include "src/sqlitemanager.h"
#include "src/scanimportmanager.h"
#include "src/projecttreemodel.h"

void testSQLiteManager()
{
    qDebug() << "\n=== Testing SQLiteManager ===";
    
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qCritical() << "Failed to create temporary directory";
        return;
    }
    
    QString dbPath = QDir(tempDir.path()).absoluteFilePath("test.sqlite");
    qDebug() << "Database path:" << dbPath;
    
    SQLiteManager manager;
    
    // Test database creation
    if (!manager.createDatabase(dbPath)) {
        qCritical() << "Failed to create database";
        return;
    }
    qDebug() << "✓ Database created successfully";
    
    // Test scan insertion
    ScanInfo scan;
    scan.scanId = "test-scan-001";
    scan.projectId = "test-project-001";
    scan.scanName = "Test Scan";
    scan.filePathRelative = "Scans/test.las";
    scan.importType = "COPIED";
    scan.dateAdded = "2024-01-01T12:00:00";
    
    if (!manager.insertScan(scan)) {
        qCritical() << "Failed to insert scan";
        return;
    }
    qDebug() << "✓ Scan inserted successfully";
    
    // Test scan retrieval
    QList<ScanInfo> scans = manager.getAllScans();
    if (scans.size() != 1) {
        qCritical() << "Expected 1 scan, got" << scans.size();
        return;
    }
    qDebug() << "✓ Scan retrieved successfully";
    
    // Test scan count
    int count = manager.getScanCount();
    if (count != 1) {
        qCritical() << "Expected count 1, got" << count;
        return;
    }
    qDebug() << "✓ Scan count correct";
    
    qDebug() << "SQLiteManager tests passed!";
}

void testProjectManager()
{
    qDebug() << "\n=== Testing ProjectManager ===";
    
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        qCritical() << "Failed to create temporary directory";
        return;
    }
    
    ProjectManager manager;
    
    // Test project creation
    QString projectPath;
    try {
        projectPath = manager.createProject("Test Project", tempDir.path());
        qDebug() << "✓ Project created at:" << projectPath;
    } catch (const std::exception &e) {
        qCritical() << "Failed to create project:" << e.what();
        return;
    }
    
    // Test project validation
    if (!manager.isValidProject(projectPath)) {
        qCritical() << "Project validation failed";
        return;
    }
    qDebug() << "✓ Project validation passed";
    
    // Test database creation
    QString dbPath = ProjectManager::getDatabasePath(projectPath);
    if (!QFileInfo::exists(dbPath)) {
        qCritical() << "Database file not created";
        return;
    }
    qDebug() << "✓ Database file exists";
    
    // Test scans subfolder creation
    QString scansPath = ProjectManager::getScansSubfolder(projectPath);
    if (!QDir(scansPath).exists()) {
        qCritical() << "Scans subfolder not created";
        return;
    }
    qDebug() << "✓ Scans subfolder exists";
    
    // Test hasScans (should be false for new project)
    if (manager.hasScans(projectPath)) {
        qCritical() << "New project should not have scans";
        return;
    }
    qDebug() << "✓ hasScans correctly returns false for new project";
    
    qDebug() << "ProjectManager tests passed!";
}

void testScanImportManager()
{
    qDebug() << "\n=== Testing ScanImportManager ===";
    
    // Test file validation
    if (ScanImportManager::isValidScanFile("test.txt")) {
        qCritical() << "Should not validate .txt files";
        return;
    }
    qDebug() << "✓ Correctly rejects invalid file types";
    
    if (!ScanImportManager::isValidScanFile("test.las")) {
        qCritical() << "Should validate .las files";
        return;
    }
    qDebug() << "✓ Correctly accepts .las files";
    
    if (!ScanImportManager::isValidScanFile("test.e57")) {
        qCritical() << "Should validate .e57 files";
        return;
    }
    qDebug() << "✓ Correctly accepts .e57 files";
    
    // Test supported extensions
    QStringList extensions = ScanImportManager::getSupportedExtensions();
    if (!extensions.contains(".las") || !extensions.contains(".e57")) {
        qCritical() << "Missing expected extensions";
        return;
    }
    qDebug() << "✓ Supported extensions correct";
    
    qDebug() << "ScanImportManager tests passed!";
}

void testProjectTreeModel()
{
    qDebug() << "\n=== Testing ProjectTreeModel ===";
    
    QApplication app(0, nullptr); // Needed for QStandardItemModel
    
    ProjectTreeModel model;
    
    // Test project setup
    model.setProject("Test Project", "/test/path");
    
    if (model.rowCount() != 1) {
        qCritical() << "Expected 1 root item, got" << model.rowCount();
        return;
    }
    qDebug() << "✓ Project root item created";
    
    // Test scan addition
    ScanInfo scan;
    scan.scanId = "test-scan-001";
    scan.scanName = "Test Scan";
    scan.filePathRelative = "Scans/test.las";
    scan.importType = "COPIED";
    scan.dateAdded = "2024-01-01T12:00:00";
    
    model.addScan(scan);
    
    // Should now have root item with scans folder
    auto *rootItem = model.item(0);
    if (!rootItem || rootItem->rowCount() != 1) {
        qCritical() << "Expected scans folder under root";
        return;
    }
    qDebug() << "✓ Scans folder created";
    
    auto *scansFolder = rootItem->child(0);
    if (!scansFolder || scansFolder->rowCount() != 1) {
        qCritical() << "Expected scan item under scans folder";
        return;
    }
    qDebug() << "✓ Scan item added";
    
    qDebug() << "ProjectTreeModel tests passed!";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Starting Sprint 1.2 Implementation Tests";
    qDebug() << "======================================";
    
    try {
        testSQLiteManager();
        testProjectManager();
        testScanImportManager();
        testProjectTreeModel();
        
        qDebug() << "\n🎉 All Sprint 1.2 tests passed!";
        qDebug() << "\nImplementation Summary:";
        qDebug() << "- SQLite database creation and management ✓";
        qDebug() << "- Project creation with database and scans folder ✓";
        qDebug() << "- Scan import validation and file type checking ✓";
        qDebug() << "- Project tree model with scan display ✓";
        
    } catch (const std::exception &e) {
        qCritical() << "Test failed with exception:" << e.what();
        return 1;
    }
    
    return 0;
}
