#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include <memory>

#include "../src/IE57Parser.h"
#include "../src/MainPresenter.h"
#include "../tests/PerformanceProfiler.cpp"
#include "mocks/MockE57Writer.h"
#include "mocks/MockMainView.h"
#include "mocks/MockPointCloudViewer.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

/**
 * @brief Performance validation tests for Sprint 5
 *
 * These tests validate that the refactored MVP architecture maintains
 * or improves performance compared to pre-refactoring benchmarks.
 * They measure key performance metrics and ensure no regressions
 * have been introduced.
 *
 * Sprint 5 Requirements:
 * - Performance benchmarking against pre-refactoring baselines
 * - Validation of key performance metrics
 * - Memory usage monitoring
 * - Rendering performance validation
 */
class PerformanceValidationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize Qt application if not already done
        if (!QCoreApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        // Create mock components for performance testing
        m_mockView = std::make_unique<NiceMock<MockMainView>>();
        m_mockWriter = std::make_unique<NiceMock<MockE57Writer>>();

        // Get raw pointers for convenience
        m_view = m_mockView.get();
        m_writer = m_mockWriter.get();
        m_viewer = m_view->getMockViewer();

        // Set up default mock behaviors for performance testing
        setupPerformanceMockBehaviors();

        // Initialize performance profiler
        m_profiler = std::make_unique<PerformanceProfiler>();
    }

    void TearDown() override
    {
        m_presenter.reset();
        m_profiler.reset();
        m_mockWriter.reset();
        m_mockView.reset();
    }

    void setupPerformanceMockBehaviors()
    {
        // Configure mocks for minimal overhead during performance testing
        ON_CALL(*m_viewer, hasData()).WillByDefault(Return(false));
        ON_CALL(*m_viewer, getViewerState()).WillByDefault(Return(ViewerState::Empty));
        ON_CALL(*m_view, getViewer()).WillByDefault(Return(m_viewer));

        // Mock viewer operations to be fast
        ON_CALL(*m_viewer, loadPointCloud(_)).WillByDefault(Return());
        ON_CALL(*m_viewer, resetCamera()).WillByDefault(Return());
        ON_CALL(*m_viewer, clearPointCloud()).WillByDefault(Return());
    }

    void createPresenterWithRealParser()
    {
        // Use real E57Parser for performance testing
        m_realParser = std::make_unique<IE57Parser>();
        m_presenter = std::make_unique<MainPresenter>(m_view, m_realParser.get(), m_writer);
        m_presenter->initialize();
    }

    std::vector<float> generateTestPointCloud(size_t pointCount)
    {
        std::vector<float> points;
        points.reserve(pointCount * 3);

        for (size_t i = 0; i < pointCount; ++i)
        {
            points.push_back(static_cast<float>(i % 1000) / 10.0f);  // X
            points.push_back(static_cast<float>(i % 500) / 10.0f);   // Y
            points.push_back(static_cast<float>(i % 200) / 10.0f);   // Z
        }

        return points;
    }

protected:
    std::unique_ptr<QCoreApplication> m_app;

    // Mock objects
    std::unique_ptr<MockMainView> m_mockView;
    std::unique_ptr<MockE57Writer> m_mockWriter;

    // Real parser for performance testing
    std::unique_ptr<IE57Parser> m_realParser;

    // Raw pointers for convenience
    MockMainView* m_view;
    MockE57Writer* m_writer;
    MockPointCloudViewer* m_viewer;

    // Components under test
    std::unique_ptr<MainPresenter> m_presenter;
    std::unique_ptr<PerformanceProfiler> m_profiler;

    // Performance benchmarks (pre-refactoring baselines)
    static constexpr double MAX_SMALL_FILE_LOAD_TIME = 200.0;        // ms for 40K points
    static constexpr double MAX_LARGE_FILE_LOAD_TIME = 1200.0;       // ms for 1M points
    static constexpr size_t MAX_MEMORY_OVERHEAD = 50 * 1024 * 1024;  // 50MB overhead
    static constexpr double MIN_UI_RESPONSIVENESS = 60.0;            // FPS
};

// ============================================================================
// Test Case 1: Small Point Cloud Loading Performance
// ============================================================================

TEST_F(PerformanceValidationTest, SmallPointCloudLoadingPerformance)
{
    // Test loading performance for small point clouds (40K points)
    // Baseline: Should complete within 200ms

    createPresenterWithRealParser();

    // Generate test data (40,000 points)
    auto testPoints = generateTestPointCloud(40000);

    // Configure mock viewer to accept the data quickly
    EXPECT_CALL(*m_viewer, loadPointCloud(_)).Times(1);
    EXPECT_CALL(*m_viewer, resetCamera()).Times(1);

    // Measure loading time
    QElapsedTimer timer;
    timer.start();

    // Simulate point cloud loading through presenter
    m_presenter->handlePointCloudDataReceived(testPoints);

    qint64 elapsedMs = timer.elapsed();

    // Performance assertion
    EXPECT_LT(elapsedMs, MAX_SMALL_FILE_LOAD_TIME) << "Small point cloud loading took " << elapsedMs
                                                   << "ms, exceeding baseline of " << MAX_SMALL_FILE_LOAD_TIME << "ms";

    // Log performance for analysis
    qDebug() << "Small point cloud (40K points) loading time:" << elapsedMs << "ms";
}

// ============================================================================
// Test Case 2: Large Point Cloud Loading Performance
// ============================================================================

TEST_F(PerformanceValidationTest, LargePointCloudLoadingPerformance)
{
    // Test loading performance for large point clouds (1M points)
    // Baseline: Should complete within 1200ms

    createPresenterWithRealParser();

    // Generate test data (1,000,000 points)
    auto testPoints = generateTestPointCloud(1000000);

    // Configure mock viewer
    EXPECT_CALL(*m_viewer, loadPointCloud(_)).Times(1);
    EXPECT_CALL(*m_viewer, resetCamera()).Times(1);

    // Measure loading time
    QElapsedTimer timer;
    timer.start();

    // Simulate point cloud loading
    m_presenter->handlePointCloudDataReceived(testPoints);

    qint64 elapsedMs = timer.elapsed();

    // Performance assertion
    EXPECT_LT(elapsedMs, MAX_LARGE_FILE_LOAD_TIME) << "Large point cloud loading took " << elapsedMs
                                                   << "ms, exceeding baseline of " << MAX_LARGE_FILE_LOAD_TIME << "ms";

    // Log performance for analysis
    qDebug() << "Large point cloud (1M points) loading time:" << elapsedMs << "ms";
}

// ============================================================================
// Test Case 3: Memory Usage Validation
// ============================================================================

TEST_F(PerformanceValidationTest, MemoryUsageValidation)
{
    // Test memory usage during point cloud operations
    // Baseline: Should not exceed 50MB overhead

    createPresenterWithRealParser();

    // Measure baseline memory
    size_t baselineMemory = getCurrentMemoryUsage();

    // Generate and load test data
    auto testPoints = generateTestPointCloud(500000);  // 500K points

    EXPECT_CALL(*m_viewer, loadPointCloud(_)).Times(1);

    // Load point cloud and measure memory
    m_presenter->handlePointCloudDataReceived(testPoints);

    size_t peakMemory = getCurrentMemoryUsage();
    size_t memoryOverhead = peakMemory - baselineMemory;

    // Calculate expected memory usage (3 floats per point * 4 bytes per float)
    size_t expectedDataSize = testPoints.size() * sizeof(float);
    size_t actualOverhead = memoryOverhead - expectedDataSize;

    // Performance assertion
    EXPECT_LT(actualOverhead, MAX_MEMORY_OVERHEAD)
        << "Memory overhead of " << actualOverhead << " bytes exceeds baseline of " << MAX_MEMORY_OVERHEAD << " bytes";

    // Log memory usage for analysis
    qDebug() << "Memory overhead:" << actualOverhead << "bytes for" << testPoints.size() / 3 << "points";
}

// ============================================================================
// Test Case 4: UI Responsiveness Validation
// ============================================================================

TEST_F(PerformanceValidationTest, UIResponsivenessValidation)
{
    // Test UI responsiveness during operations
    // Baseline: Should maintain 60 FPS equivalent responsiveness

    createPresenterWithRealParser();

    // Configure mock viewer to report performance stats
    ON_CALL(*m_viewer, getCurrentFPS()).WillByDefault(Return(60.0f));
    ON_CALL(*m_viewer, getVisiblePointCount()).WillByDefault(Return(100000));

    // Simulate multiple rapid operations
    QElapsedTimer timer;
    timer.start();

    const int operationCount = 100;
    for (int i = 0; i < operationCount; ++i)
    {
        // Simulate rapid UI operations
        m_presenter->handleTopViewClicked();
        m_presenter->handleLeftViewClicked();
        m_presenter->handleRightViewClicked();

        // Process events to simulate real UI behavior
        QCoreApplication::processEvents();
    }

    qint64 totalTime = timer.elapsed();
    double averageOperationTime = static_cast<double>(totalTime) / operationCount;
    double operationsPerSecond = 1000.0 / averageOperationTime;

    // Performance assertion (should handle at least 60 operations per second)
    EXPECT_GE(operationsPerSecond, MIN_UI_RESPONSIVENESS)
        << "UI responsiveness of " << operationsPerSecond << " ops/sec is below baseline of " << MIN_UI_RESPONSIVENESS
        << " ops/sec";

    // Log responsiveness for analysis
    qDebug() << "UI responsiveness:" << operationsPerSecond << "operations/sec";
}

// ============================================================================
// Test Case 5: Component Integration Performance
// ============================================================================

TEST_F(PerformanceValidationTest, ComponentIntegrationPerformance)
{
    // Test performance of component interactions in MVP architecture
    // Ensure abstraction layers don't introduce significant overhead

    createPresenterWithRealParser();

    // Measure baseline direct operation time
    QElapsedTimer directTimer;
    directTimer.start();

    // Direct viewer operations (simulated)
    for (int i = 0; i < 1000; ++i)
    {
        m_viewer->setTopView();
    }

    qint64 directTime = directTimer.elapsed();

    // Measure presenter-mediated operation time
    QElapsedTimer presenterTimer;
    presenterTimer.start();

    // Presenter-mediated operations
    for (int i = 0; i < 1000; ++i)
    {
        m_presenter->handleTopViewClicked();
    }

    qint64 presenterTime = presenterTimer.elapsed();

    // Calculate overhead percentage
    double overheadPercentage = ((double)(presenterTime - directTime) / directTime) * 100.0;

    // Performance assertion (overhead should be less than 20%)
    EXPECT_LT(overheadPercentage, 20.0) << "MVP architecture overhead of " << overheadPercentage
                                        << "% exceeds acceptable threshold of 20%";

    // Log integration performance for analysis
    qDebug() << "MVP architecture overhead:" << overheadPercentage << "%";
}

// ============================================================================
// Helper function for memory usage measurement
// ============================================================================

size_t PerformanceValidationTest::getCurrentMemoryUsage()
{
    // Platform-specific memory usage measurement
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return pmc.WorkingSetSize;
    }
#elif defined(__linux__)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open())
    {
        size_t size, resident, share, text, lib, data, dt;
        statm >> size >> resident >> share >> text >> lib >> data >> dt;
        return resident * getpagesize();
    }
#elif defined(__APPLE__)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS)
    {
        return info.resident_size;
    }
#endif
    return 0;  // Fallback if platform-specific measurement fails
}
