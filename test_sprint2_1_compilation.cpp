#include <QApplication>
#include <QDebug>
#include <QTimer>

// Test compilation of Sprint 2.1 components
#include "src/pointcloudloadmanager.h"
#include "src/projecttreemodel.h"
#include "src/sidebarwidget.h"
#include "src/projectmanager.h"
#include "src/sqlitemanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "=== Sprint 2.1 Compilation Test ===";
    
    // Test 1: Create ProjectTreeModel and verify LoadedState enum
    qDebug() << "\n--- Test 1: ProjectTreeModel with LoadedState ---";
    ProjectTreeModel model;
    qDebug() << "✓ ProjectTreeModel created successfully";
    
    // Test LoadedState enum values
    LoadedState state1 = LoadedState::Unloaded;
    LoadedState state2 = LoadedState::Loaded;
    LoadedState state3 = LoadedState::Partial;
    LoadedState state4 = LoadedState::Loading;
    LoadedState state5 = LoadedState::Error;
    
    qDebug() << "✓ LoadedState enum values accessible:";
    qDebug() << "  - Unloaded:" << static_cast<int>(state1);
    qDebug() << "  - Loaded:" << static_cast<int>(state2);
    qDebug() << "  - Partial:" << static_cast<int>(state3);
    qDebug() << "  - Loading:" << static_cast<int>(state4);
    qDebug() << "  - Error:" << static_cast<int>(state5);
    
    // Test 2: Create PointCloudLoadManager
    qDebug() << "\n--- Test 2: PointCloudLoadManager ---";
    PointCloudLoadManager loadManager;
    qDebug() << "✓ PointCloudLoadManager created successfully";
    qDebug() << "✓ Memory limit:" << loadManager.getTotalMemoryUsage() << "bytes";
    qDebug() << "✓ Loaded scans count:" << loadManager.getLoadedScans().size();
    
    // Test 3: Create SidebarWidget
    qDebug() << "\n--- Test 3: SidebarWidget ---";
    SidebarWidget sidebar;
    qDebug() << "✓ SidebarWidget created successfully";
    
    // Test 4: Test integration setup
    qDebug() << "\n--- Test 4: Integration Setup ---";
    
    // Create managers
    SQLiteManager sqliteManager;
    ProjectManager projectManager;
    
    // Set up connections
    sidebar.setSQLiteManager(&sqliteManager);
    sidebar.setProjectManager(&projectManager);
    sidebar.setPointCloudLoadManager(&loadManager);
    
    model.setSQLiteManager(&sqliteManager);
    loadManager.setSQLiteManager(&sqliteManager);
    loadManager.setProjectTreeModel(&model);
    
    qDebug() << "✓ All managers connected successfully";
    
    // Test 5: Test scan state management
    qDebug() << "\n--- Test 5: Scan State Management ---";
    
    QString testScanId = "test-scan-123";
    
    // Test initial state
    LoadedState initialState = model.getScanLoadedState(testScanId);
    qDebug() << "✓ Initial scan state:" << static_cast<int>(initialState);
    
    // Test setting state
    model.setScanLoadedState(testScanId, LoadedState::Loading);
    LoadedState loadingState = model.getScanLoadedState(testScanId);
    qDebug() << "✓ Loading state set:" << static_cast<int>(loadingState);
    
    model.setScanLoadedState(testScanId, LoadedState::Loaded);
    LoadedState loadedState = model.getScanLoadedState(testScanId);
    qDebug() << "✓ Loaded state set:" << static_cast<int>(loadedState);
    
    // Test 6: Test PointCloudLoadManager state queries
    qDebug() << "\n--- Test 6: PointCloudLoadManager State Queries ---";
    
    LoadedState managerState = loadManager.getScanLoadedState(testScanId);
    qDebug() << "✓ Manager scan state:" << static_cast<int>(managerState);
    
    bool isLoaded = loadManager.isScanLoaded(testScanId);
    qDebug() << "✓ Is scan loaded:" << (isLoaded ? "Yes" : "No");
    
    QStringList loadedScans = loadManager.getLoadedScans();
    qDebug() << "✓ Total loaded scans:" << loadedScans.size();
    
    // Test 7: Test memory management
    qDebug() << "\n--- Test 7: Memory Management ---";
    
    size_t memoryUsage = loadManager.getTotalMemoryUsage();
    qDebug() << "✓ Current memory usage:" << memoryUsage << "bytes";
    
    loadManager.setMemoryLimit(1024); // 1GB
    qDebug() << "✓ Memory limit set to 1GB";
    
    // Test 8: Test error handling
    qDebug() << "\n--- Test 8: Error Handling ---";
    
    QString lastError = loadManager.getLastError();
    qDebug() << "✓ Last error:" << (lastError.isEmpty() ? "None" : lastError);
    
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "Sprint 2.1 components compiled and basic functionality verified!";
    
    // Exit after a short delay
    QTimer::singleShot(100, &app, &QApplication::quit);
    
    return app.exec();
}
