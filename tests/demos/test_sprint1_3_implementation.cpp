#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QUuid>
#include <QDateTime>

// Include our Sprint 1.3 implementation
#include "../../src/projectmanager.h"
#include "../../src/sqlitemanager.h"
#include "../../src/projecttreemodel.h"
#include "../../src/sidebarwidget.h"

class TestWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        testSprint13Implementation();
    }

private:
    void setupUI()
    {
        auto *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        auto *layout = new QVBoxLayout(centralWidget);
        
        m_sidebar = new SidebarWidget(this);
        layout->addWidget(m_sidebar);
        
        setWindowTitle("Sprint 1.3 Test - Cluster Creation & Scan Organization");
        resize(400, 600);
    }
    
    void testSprint13Implementation()
    {
        qDebug() << "=== Testing Sprint 1.3 Implementation ===";
        
        // Test 1: Create ProjectManager and SQLiteManager
        m_projectManager = new ProjectManager(this);
        
        // Test 2: Create a test project
        QString testProjectPath;
        try {
            QString basePath = QDir::tempPath();
            QString projectName = "TestProject_Sprint13";
            testProjectPath = m_projectManager->createProject(projectName, basePath);
            qDebug() << "✓ Project created successfully:" << testProjectPath;
        } catch (const std::exception &e) {
            qDebug() << "✗ Failed to create project:" << e.what();
            return;
        }
        
        // Test 3: Load the project
        try {
            ProjectInfo projectInfo = m_projectManager->loadProject(testProjectPath);
            qDebug() << "✓ Project loaded successfully:" << projectInfo.projectName;
            
            // Set up sidebar with project
            m_sidebar->setSQLiteManager(m_projectManager->getSQLiteManager());
            m_sidebar->setProjectManager(m_projectManager);
            m_sidebar->setProject(projectInfo.projectName, testProjectPath);
            
        } catch (const std::exception &e) {
            qDebug() << "✗ Failed to load project:" << e.what();
            return;
        }
        
        // Test 4: Create clusters
        testClusterCreation();
        
        // Test 5: Create test scans
        testScanCreation();
        
        // Test 6: Test cluster hierarchy
        testClusterHierarchy();
        
        qDebug() << "=== Sprint 1.3 Test Completed ===";
    }
    
    void testClusterCreation()
    {
        qDebug() << "\n--- Testing Cluster Creation ---";
        
        // Create top-level clusters
        QString cluster1Id = m_projectManager->createCluster("Building A");
        QString cluster2Id = m_projectManager->createCluster("Building B");
        
        if (!cluster1Id.isEmpty() && !cluster2Id.isEmpty()) {
            qDebug() << "✓ Top-level clusters created successfully";
            m_testCluster1Id = cluster1Id;
            m_testCluster2Id = cluster2Id;
        } else {
            qDebug() << "✗ Failed to create top-level clusters";
            return;
        }
        
        // Create sub-clusters
        QString subCluster1Id = m_projectManager->createCluster("Floor 1", cluster1Id);
        QString subCluster2Id = m_projectManager->createCluster("Floor 2", cluster1Id);
        
        if (!subCluster1Id.isEmpty() && !subCluster2Id.isEmpty()) {
            qDebug() << "✓ Sub-clusters created successfully";
            m_testSubCluster1Id = subCluster1Id;
            m_testSubCluster2Id = subCluster2Id;
        } else {
            qDebug() << "✗ Failed to create sub-clusters";
        }
    }
    
    void testScanCreation()
    {
        qDebug() << "\n--- Testing Scan Creation ---";
        
        // Create test scans
        ScanInfo scan1;
        scan1.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        scan1.projectId = "test-project";
        scan1.scanName = "Scan_001";
        scan1.filePathRelative = "Scans/scan_001.las";
        scan1.importType = "COPIED";
        scan1.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);
        scan1.parentClusterId = QString(); // At project root initially
        
        ScanInfo scan2;
        scan2.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        scan2.projectId = "test-project";
        scan2.scanName = "Scan_002";
        scan2.filePathRelative = "Scans/scan_002.e57";
        scan2.importType = "COPIED";
        scan2.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);
        scan2.parentClusterId = m_testCluster1Id; // In Building A
        
        if (m_projectManager->getSQLiteManager()->insertScan(scan1) &&
            m_projectManager->getSQLiteManager()->insertScan(scan2)) {
            qDebug() << "✓ Test scans created successfully";
            m_testScan1Id = scan1.scanId;
            m_testScan2Id = scan2.scanId;
        } else {
            qDebug() << "✗ Failed to create test scans";
        }
    }
    
    void testClusterHierarchy()
    {
        qDebug() << "\n--- Testing Cluster Hierarchy ---";
        
        // Test moving scan between clusters
        if (!m_testScan1Id.isEmpty() && !m_testSubCluster1Id.isEmpty()) {
            if (m_projectManager->moveScanToCluster(m_testScan1Id, m_testSubCluster1Id)) {
                qDebug() << "✓ Scan moved to sub-cluster successfully";
            } else {
                qDebug() << "✗ Failed to move scan to sub-cluster";
            }
        }
        
        // Test cluster retrieval
        QList<ClusterInfo> allClusters = m_projectManager->getProjectClusters();
        qDebug() << "✓ Retrieved" << allClusters.size() << "clusters from database";
        
        QList<ClusterInfo> topLevelClusters = m_projectManager->getChildClusters(QString());
        qDebug() << "✓ Retrieved" << topLevelClusters.size() << "top-level clusters";
        
        // Refresh sidebar to show updated hierarchy
        m_sidebar->refreshFromDatabase();
        qDebug() << "✓ Sidebar refreshed with cluster hierarchy";
    }

private:
    SidebarWidget *m_sidebar;
    ProjectManager *m_projectManager;
    
    // Test data IDs
    QString m_testCluster1Id;
    QString m_testCluster2Id;
    QString m_testSubCluster1Id;
    QString m_testSubCluster2Id;
    QString m_testScan1Id;
    QString m_testScan2Id;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TestWindow window;
    window.show();
    
    return app.exec();
}

#include "test_sprint1_3_implementation.moc"
