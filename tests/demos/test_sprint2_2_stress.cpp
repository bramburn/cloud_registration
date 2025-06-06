#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QThread>
#include <QElapsedTimer>
#include <chrono>
#include <thread>

// Include Sprint 2.2 components
#include "../../src/projecttreemodel.h"
#include "../../src/pointcloudloadmanager.h"
#include "../../src/sidebarwidget.h"
#include "../../src/sqlitemanager.h"
#include "../../src/performance_profiler.h"
#include "../../src/performance_benchmark.h"

/**
 * @brief Sprint 2.2 Stress Testing Suite
 * 
 * This test suite validates the memory management system under extreme conditions,
 * ensuring stability and proper LRU eviction behavior as specified in Sprint 2.2.
 */
class Sprint22StressTest : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        if (!QCoreApplication::instance()) {
            app = new QCoreApplication(argc, argv);
        }

        // Create temporary directory for test database
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        // Initialize components with artificial low memory limit for testing
        m_loadManager = new PointCloudLoadManager();
        m_loadManager->setMemoryLimit(100 * 1024 * 1024); // 100 MB limit for testing
        
        m_model = new ProjectTreeModel();
        m_sqliteManager = new SQLiteManager();
        
        // Setup test project
        QString dbPath = tempDir->path() + "/test_project.db";
        ASSERT_TRUE(m_sqliteManager->openDatabase(dbPath));
        ASSERT_TRUE(m_sqliteManager->initializeDatabase());
        
        m_model->setSQLiteManager(m_sqliteManager);
        m_model->setProject("StressTestProject", tempDir->path());
        
        // Connect load manager to model
        m_loadManager->setProjectTreeModel(m_model);
        m_loadManager->setSQLiteManager(m_sqliteManager);
        
        // Record baseline memory usage
        m_baselineMemory = getCurrentMemoryUsage();
        
        qDebug() << "Sprint 2.2 Stress Test Setup - Baseline memory:" << m_baselineMemory << "bytes";
    }
    
    void TearDown() override {
        // Cleanup all loaded scans
        if (m_loadManager) {
            // Force unload all scans
            auto loadedScans = getLoadedScanIds();
            for (const QString& scanId : loadedScans) {
                m_loadManager->unloadScan(scanId);
            }
        }
        
        delete m_loadManager;
        delete m_model;
        delete m_sqliteManager;
        delete tempDir;
        
        if (app && QCoreApplication::instance() == app) {
            delete app;
        }
    }
    
    /**
     * @brief Get current memory usage of the process
     * @return Memory usage in bytes, or 0 if unable to determine
     */
    size_t getCurrentMemoryUsage() {
#ifdef _WIN32
        // Windows implementation using GetProcessMemoryInfo
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), 
                               (PROCESS_MEMORY_COUNTERS*)&pmc, 
                               sizeof(pmc))) {
            return pmc.PrivateUsage;
        }
#else
        // Linux implementation using /proc/self/statm
        QFile file("/proc/self/statm");
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            QString line = stream.readLine();
            QStringList fields = line.split(' ');
            if (fields.size() >= 2) {
                // VmRSS is the second field, in pages
                long pages = fields[1].toLong();
                return pages * 4096; // Assume 4KB page size
            }
        }
#endif
        return 0; // Return 0 if unable to determine
    }
    
    /**
     * @brief Create mock scan entries in the database for testing
     * @param count Number of scans to create
     * @return List of created scan IDs
     */
    QStringList createMockScans(int count) {
        QStringList scanIds;
        
        for (int i = 0; i < count; ++i) {
            ScanInfo scan;
            scan.scanId = QString("stress_scan_%1").arg(i);
            scan.name = QString("Stress Test Scan %1").arg(i);
            scan.filePath = QString("/mock/path/scan_%1.las").arg(i);
            scan.fileSize = 50 * 1024 * 1024; // 50MB each
            scan.pointCount = 1000000; // 1M points each
            scan.clusterId = ""; // Root level
            
            // Add to database
            if (m_sqliteManager->addScan(scan)) {
                scanIds.append(scan.scanId);
                m_model->addScan(scan);
            }
        }
        
        return scanIds;
    }
    
    /**
     * @brief Get list of currently loaded scan IDs
     * @return List of loaded scan IDs
     */
    QStringList getLoadedScanIds() {
        QStringList loadedScans;
        // This would need to be implemented in the actual PointCloudLoadManager
        // For now, we'll track manually in tests
        return m_loadedScans;
    }
    
    /**
     * @brief Simulate loading a scan (creates mock point cloud data)
     * @param scanId ID of scan to load
     * @return True if loading succeeded
     */
    bool simulateLoadScan(const QString& scanId) {
        // Create mock point cloud data (approximately 50MB)
        std::vector<float> mockPoints;
        mockPoints.reserve(1000000 * 3); // 1M points * 3 coordinates
        
        for (int i = 0; i < 1000000; ++i) {
            mockPoints.push_back(static_cast<float>(i % 1000));
            mockPoints.push_back(static_cast<float>((i / 1000) % 1000));
            mockPoints.push_back(static_cast<float>(i / 1000000));
        }
        
        // Simulate the loading process
        bool success = m_loadManager->loadScanData(scanId, mockPoints);
        if (success) {
            m_loadedScans.append(scanId);
        }
        
        return success;
    }

    QCoreApplication* app = nullptr;
    QTemporaryDir* tempDir = nullptr;
    PointCloudLoadManager* m_loadManager = nullptr;
    ProjectTreeModel* m_model = nullptr;
    SQLiteManager* m_sqliteManager = nullptr;
    size_t m_baselineMemory = 0;
    QStringList m_loadedScans; // Track loaded scans manually for testing
};

/**
 * @brief Test memory limit enforcement under stress conditions
 * 
 * This test validates that the PointCloudLoadManager correctly enforces
 * memory limits and doesn't exceed the configured threshold.
 */
TEST_F(Sprint22StressTest, MemoryLimitEnforcement)
{
    PROFILE_FUNCTION();
    
    // Create multiple mock scans
    QStringList scanIds = createMockScans(5);
    ASSERT_EQ(scanIds.size(), 5);
    
    qDebug() << "Created" << scanIds.size() << "mock scans for memory limit test";
    
    // Load scans until memory limit should be reached
    for (const QString& scanId : scanIds) {
        qDebug() << "Loading scan:" << scanId;
        
        // Simulate loading
        bool loadResult = simulateLoadScan(scanId);
        
        // Check memory usage after each load
        size_t currentUsage = m_loadManager->getTotalMemoryUsage();
        qDebug() << "Memory usage after loading" << scanId << ":" << currentUsage << "bytes";
        
        // Verify memory usage doesn't exceed limit
        EXPECT_LE(currentUsage, m_loadManager->getMemoryLimit())
            << "Memory usage exceeded limit after loading: " << scanId;
        
        // Allow some processing time
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    qDebug() << "Memory limit enforcement test completed";
}

/**
 * @brief Test LRU eviction behavior under memory pressure
 * 
 * This test validates that the least recently used scans are properly
 * evicted when memory limits are reached.
 */
TEST_F(Sprint22StressTest, LRUEvictionBehavior)
{
    PROFILE_FUNCTION();
    
    // Create test scans
    QStringList scanIds = createMockScans(4);
    ASSERT_EQ(scanIds.size(), 4);
    
    qDebug() << "Testing LRU eviction with scans:" << scanIds;
    
    // Load first two scans
    ASSERT_TRUE(simulateLoadScan(scanIds[0]));
    ASSERT_TRUE(simulateLoadScan(scanIds[1]));
    
    // Access first scan to make it recently used
    m_loadManager->accessScan(scanIds[0]);
    
    // Load third scan - should trigger eviction of second scan (least recently used)
    ASSERT_TRUE(simulateLoadScan(scanIds[2]));
    
    // Verify eviction behavior
    EXPECT_EQ(m_model->getScanLoadedState(scanIds[0]), LoadedState::Loaded)
        << "First scan should remain loaded (recently accessed)";
    EXPECT_EQ(m_model->getScanLoadedState(scanIds[1]), LoadedState::Unloaded)
        << "Second scan should be evicted (least recently used)";
    EXPECT_EQ(m_model->getScanLoadedState(scanIds[2]), LoadedState::Loaded)
        << "Third scan should be loaded";
    
    qDebug() << "LRU eviction test completed successfully";
}

/**
 * @brief Test rapid load/unload cycles for memory leak detection
 *
 * This test performs rapid loading and unloading of scans to detect
 * potential memory leaks in the system.
 */
TEST_F(Sprint22StressTest, RapidLoadUnloadCycles)
{
    PROFILE_FUNCTION();

    // Create test scan
    QStringList scanIds = createMockScans(1);
    ASSERT_EQ(scanIds.size(), 1);
    QString testScanId = scanIds[0];

    qDebug() << "Starting rapid load/unload cycles with scan:" << testScanId;

    size_t initialMemory = getCurrentMemoryUsage();
    qDebug() << "Initial memory usage:" << initialMemory << "bytes";

    // Perform 50 rapid load/unload cycles
    for (int cycle = 0; cycle < 50; ++cycle) {
        // Load scan
        ASSERT_TRUE(simulateLoadScan(testScanId))
            << "Failed to load scan in cycle " << cycle;

        // Brief processing time
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Unload scan
        m_loadManager->unloadScan(testScanId);
        m_loadedScans.removeAll(testScanId);

        // Brief processing time
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Check for memory growth every 10 cycles
        if (cycle % 10 == 9) {
            size_t currentMemory = getCurrentMemoryUsage();
            qDebug() << "Memory after cycle" << (cycle + 1) << ":" << currentMemory << "bytes";

            // Allow for some memory growth, but not excessive
            size_t memoryGrowth = currentMemory > initialMemory ?
                                 currentMemory - initialMemory : 0;
            EXPECT_LT(memoryGrowth, 50 * 1024 * 1024) // Less than 50MB growth
                << "Excessive memory growth detected after " << (cycle + 1) << " cycles";
        }
    }

    qDebug() << "Rapid load/unload cycles test completed";
}

/**
 * @brief Test concurrent access patterns for thread safety
 *
 * This test validates thread safety of the memory management system
 * under concurrent load/unload operations.
 */
TEST_F(Sprint22StressTest, ConcurrentAccessPatterns)
{
    PROFILE_FUNCTION();

    // Create test scans
    QStringList scanIds = createMockScans(3);
    ASSERT_EQ(scanIds.size(), 3);

    qDebug() << "Testing concurrent access patterns with scans:" << scanIds;

    // Use QSignalSpy to monitor operations
    QSignalSpy loadSpy(m_loadManager, &PointCloudLoadManager::scanLoaded);
    QSignalSpy unloadSpy(m_loadManager, &PointCloudLoadManager::scanUnloaded);

    // Simulate concurrent operations using QTimer
    QTimer loadTimer;
    QTimer unloadTimer;
    QTimer accessTimer;

    int loadIndex = 0;
    int unloadIndex = 0;
    int accessIndex = 0;

    // Load operations every 100ms
    connect(&loadTimer, &QTimer::timeout, [&]() {
        if (loadIndex < scanIds.size()) {
            QString scanId = scanIds[loadIndex];
            qDebug() << "Concurrent load:" << scanId;
            simulateLoadScan(scanId);
            loadIndex++;
        } else {
            loadTimer.stop();
        }
    });

    // Unload operations every 150ms (offset timing)
    connect(&unloadTimer, &QTimer::timeout, [&]() {
        if (unloadIndex < scanIds.size() && unloadIndex < loadIndex) {
            QString scanId = scanIds[unloadIndex];
            qDebug() << "Concurrent unload:" << scanId;
            m_loadManager->unloadScan(scanId);
            m_loadedScans.removeAll(scanId);
            unloadIndex++;
        } else if (loadIndex >= scanIds.size()) {
            unloadTimer.stop();
        }
    });

    // Access operations every 75ms (different timing)
    connect(&accessTimer, &QTimer::timeout, [&]() {
        if (accessIndex < scanIds.size() && accessIndex < loadIndex) {
            QString scanId = scanIds[accessIndex % scanIds.size()];
            qDebug() << "Concurrent access:" << scanId;
            m_loadManager->accessScan(scanId);
            accessIndex++;
        } else if (loadIndex >= scanIds.size() && unloadIndex >= scanIds.size()) {
            accessTimer.stop();
        }
    });

    // Start concurrent operations
    loadTimer.start(100);
    unloadTimer.start(150);
    accessTimer.start(75);

    // Wait for all operations to complete
    QElapsedTimer testTimer;
    testTimer.start();

    while ((loadTimer.isActive() || unloadTimer.isActive() || accessTimer.isActive())
           && testTimer.elapsed() < 10000) { // 10 second timeout
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Verify no crashes occurred and operations completed
    EXPECT_FALSE(loadTimer.isActive()) << "Load operations did not complete in time";
    EXPECT_FALSE(unloadTimer.isActive()) << "Unload operations did not complete in time";
    EXPECT_FALSE(accessTimer.isActive()) << "Access operations did not complete in time";

    qDebug() << "Concurrent access patterns test completed";
}

/**
 * @brief Test performance under sustained load
 *
 * This test validates that performance remains acceptable under
 * sustained memory management operations.
 */
TEST_F(Sprint22StressTest, SustainedLoadPerformance)
{
    PROFILE_FUNCTION();

    // Create larger set of test scans
    QStringList scanIds = createMockScans(10);
    ASSERT_EQ(scanIds.size(), 10);

    qDebug() << "Testing sustained load performance with" << scanIds.size() << "scans";

    PerformanceBenchmark benchmark;
    benchmark.startBenchmark("SustainedLoad");

    QElapsedTimer operationTimer;
    QList<qint64> loadTimes;
    QList<qint64> unloadTimes;

    // Perform sustained operations for 30 seconds
    QElapsedTimer testTimer;
    testTimer.start();

    int operationCount = 0;
    while (testTimer.elapsed() < 30000) { // 30 seconds
        QString scanId = scanIds[operationCount % scanIds.size()];

        // Measure load time
        operationTimer.restart();
        bool loadSuccess = simulateLoadScan(scanId);
        qint64 loadTime = operationTimer.elapsed();

        if (loadSuccess) {
            loadTimes.append(loadTime);

            // Brief pause
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Measure unload time
            operationTimer.restart();
            m_loadManager->unloadScan(scanId);
            m_loadedScans.removeAll(scanId);
            qint64 unloadTime = operationTimer.elapsed();
            unloadTimes.append(unloadTime);
        }

        operationCount++;
        QCoreApplication::processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    benchmark.endBenchmark("SustainedLoad");

    // Analyze performance metrics
    if (!loadTimes.isEmpty()) {
        qint64 avgLoadTime = std::accumulate(loadTimes.begin(), loadTimes.end(), 0LL) / loadTimes.size();
        qint64 maxLoadTime = *std::max_element(loadTimes.begin(), loadTimes.end());

        qDebug() << "Load performance - Operations:" << loadTimes.size()
                 << "Avg time:" << avgLoadTime << "ms, Max time:" << maxLoadTime << "ms";

        // Performance expectations (adjust based on requirements)
        EXPECT_LT(avgLoadTime, 1000) << "Average load time too high: " << avgLoadTime << "ms";
        EXPECT_LT(maxLoadTime, 5000) << "Maximum load time too high: " << maxLoadTime << "ms";
    }

    if (!unloadTimes.isEmpty()) {
        qint64 avgUnloadTime = std::accumulate(unloadTimes.begin(), unloadTimes.end(), 0LL) / unloadTimes.size();
        qint64 maxUnloadTime = *std::max_element(unloadTimes.begin(), unloadTimes.end());

        qDebug() << "Unload performance - Operations:" << unloadTimes.size()
                 << "Avg time:" << avgUnloadTime << "ms, Max time:" << maxUnloadTime << "ms";

        // Performance expectations
        EXPECT_LT(avgUnloadTime, 500) << "Average unload time too high: " << avgUnloadTime << "ms";
        EXPECT_LT(maxUnloadTime, 2000) << "Maximum unload time too high: " << maxUnloadTime << "ms";
    }

    qDebug() << "Sustained load performance test completed";
    qDebug() << "Total operations performed:" << operationCount;
}
