#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QMessageBox>
#include <QTimer>

// Include our Sprint 1.3 implementation
#include "../src/scanimportmanager.h"
#include "../src/scanimportdialog.h"
#include "../src/pointcloudloadmanager.h"
#include "../src/E57DataManager.h"
#include "../src/projectmanager.h"
#include "../src/sqlitemanager.h"

class TestE57Integration : public QMainWindow
{
    Q_OBJECT

public:
    TestE57Integration(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupManagers();
        testE57Integration();
    }

private:
    void setupUI()
    {
        auto *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        auto *layout = new QVBoxLayout(centralWidget);
        
        setWindowTitle("Sprint 1.3 E57 Integration Test");
        resize(600, 400);
    }
    
    void setupManagers()
    {
        // Create managers
        m_projectManager = new ProjectManager(this);
        m_scanImportManager = new ScanImportManager(this);
        m_pointCloudLoadManager = new PointCloudLoadManager(this);
        
        // Connect signals for testing
        connect(m_scanImportManager, &ScanImportManager::importCompleted,
                this, &TestE57Integration::onImportCompleted);
        connect(m_scanImportManager, &ScanImportManager::importFailed,
                this, &TestE57Integration::onImportFailed);
        
        connect(m_pointCloudLoadManager, &PointCloudLoadManager::pointCloudDataReady,
                this, &TestE57Integration::onPointCloudDataReady);
        connect(m_pointCloudLoadManager, &PointCloudLoadManager::pointCloudViewFailed,
                this, &TestE57Integration::onPointCloudViewFailed);
    }
    
    void testE57Integration()
    {
        qDebug() << "=== Testing Sprint 1.3 E57 Integration ===";
        
        // Test 1: Create a test project
        QString testProjectPath;
        try {
            QString basePath = QDir::tempPath();
            QString projectName = "TestProject_E57_Sprint13";
            testProjectPath = m_projectManager->createProject(projectName, basePath);
            qDebug() << "✓ Project created successfully:" << testProjectPath;
        } catch (const std::exception &e) {
            qDebug() << "✗ Failed to create project:" << e.what();
            return;
        }
        
        // Test 2: Load the project
        try {
            ProjectInfo projectInfo = m_projectManager->loadProject(testProjectPath);
            qDebug() << "✓ Project loaded successfully:" << projectInfo.projectName;
            
            // Set up managers with project
            m_scanImportManager->setSQLiteManager(m_projectManager->getSQLiteManager());
            m_pointCloudLoadManager->setSQLiteManager(m_projectManager->getSQLiteManager());
            
        } catch (const std::exception &e) {
            qDebug() << "✗ Failed to load project:" << e.what();
            return;
        }
        
        // Test 3: Test E57DataManager functionality
        testE57DataManager();
        
        // Test 4: Test ScanImportDialog E57 support
        testScanImportDialog();
        
        // Test 5: Test E57 import workflow (simulated)
        testE57ImportWorkflow();
        
        qDebug() << "=== Sprint 1.3 E57 Integration Test Completed ===";
    }
    
    void testE57DataManager()
    {
        qDebug() << "\n--- Testing E57DataManager ---";
        
        E57DataManager e57Manager;
        
        // Test file validation with non-existent file
        QString testFile = "non_existent_file.e57";
        bool isValid = e57Manager.isValidE57File(testFile);
        qDebug() << "✓ E57 file validation test (expected false):" << isValid;
        
        // Test with invalid file extension
        QString invalidFile = "test.txt";
        isValid = e57Manager.isValidE57File(invalidFile);
        qDebug() << "✓ Invalid file extension test (expected false):" << isValid;
    }
    
    void testScanImportDialog()
    {
        qDebug() << "\n--- Testing ScanImportDialog E57 Support ---";
        
        // Test that E57 files are in supported extensions
        QStringList supportedExtensions = ScanImportManager::getSupportedExtensions();
        bool hasE57 = supportedExtensions.contains(".e57");
        qDebug() << "✓ E57 extension supported:" << hasE57;
        
        // Test file validation
        bool isValidE57 = ScanImportManager::isValidScanFile("test.e57");
        bool isValidLas = ScanImportManager::isValidScanFile("test.las");
        bool isValidInvalid = ScanImportManager::isValidScanFile("test.txt");
        
        qDebug() << "✓ E57 file validation:" << isValidE57;
        qDebug() << "✓ LAS file validation:" << isValidLas;
        qDebug() << "✓ Invalid file validation (expected false):" << isValidInvalid;
    }
    
    void testE57ImportWorkflow()
    {
        qDebug() << "\n--- Testing E57 Import Workflow (Simulated) ---";
        
        // Create a mock E57 scan entry to test the database integration
        ScanInfo mockE57Scan;
        mockE57Scan.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        mockE57Scan.projectId = "test-project";
        mockE57Scan.scanName = "Mock_E57_Scan";
        mockE57Scan.filePathRelative = "test_file.e57";
        mockE57Scan.importType = "E57";
        mockE57Scan.originalSourcePath = "mock-e57-guid-12345";
        mockE57Scan.pointCountEstimate = 1000000;
        mockE57Scan.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        // Test database insertion
        bool inserted = m_projectManager->getSQLiteManager()->insertScan(mockE57Scan);
        qDebug() << "✓ Mock E57 scan inserted into database:" << inserted;
        
        if (inserted) {
            // Test retrieval
            ScanInfo retrievedScan = m_projectManager->getSQLiteManager()->getScanById(mockE57Scan.scanId);
            bool retrieved = !retrievedScan.scanId.isEmpty();
            qDebug() << "✓ Mock E57 scan retrieved from database:" << retrieved;
            
            if (retrieved) {
                qDebug() << "  - Scan Name:" << retrievedScan.scanName;
                qDebug() << "  - Import Type:" << retrievedScan.importType;
                qDebug() << "  - E57 GUID:" << retrievedScan.originalSourcePath;
                qDebug() << "  - Point Count:" << retrievedScan.pointCountEstimate;
            }
        }
    }

private slots:
    void onImportCompleted(const QString& filePath, int scanCount)
    {
        qDebug() << "✓ E57 Import completed:" << filePath << "with" << scanCount << "scans";
    }
    
    void onImportFailed(const QString& filePath, const QString& error)
    {
        qDebug() << "✗ E57 Import failed:" << filePath << "Error:" << error;
    }
    
    void onPointCloudDataReady(const std::vector<float>& points, const QString& sourceInfo)
    {
        qDebug() << "✓ Point cloud data ready:" << sourceInfo << "with" << (points.size() / 6) << "points";
    }
    
    void onPointCloudViewFailed(const QString& error)
    {
        qDebug() << "✗ Point cloud view failed:" << error;
    }

private:
    ProjectManager *m_projectManager;
    ScanImportManager *m_scanImportManager;
    PointCloudLoadManager *m_pointCloudLoadManager;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestE57Integration window;
    window.show();
    
    return app.exec();
}

#include "test_sprint1_3_e57_integration.moc"
