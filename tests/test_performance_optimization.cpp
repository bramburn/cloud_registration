#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QThread>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <memory>
#include <vector>
#include <chrono>

#include "../src/performance/MemoryManager.h"
#include "../src/performance/ParallelProcessing.h"
#include "../src/pointdata.h"

/**
 * @brief Test suite for Sprint 7 performance optimization components
 * 
 * Tests cover:
 * - MemoryManager functionality and performance
 * - ParallelProcessing capabilities and scalability
 * - Memory leak detection and stress testing
 * - Performance benchmarks and validation
 */
class PerformanceOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application if not already done
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = std::make_unique<QCoreApplication>(argc, argv);
        }
        
        memoryManager = &MemoryManager::instance();
        parallelProcessing = &ParallelProcessing::instance();
    }
    
    void TearDown() override {
        // Clean up memory manager
        memoryManager->clearPools();
        memoryManager->finalizeStreaming();
        
        // Cancel all parallel tasks
        parallelProcessing->cancelAll();
        parallelProcessing->waitForAll();
    }
    
    std::unique_ptr<QCoreApplication> app;
    MemoryManager* memoryManager;
    ParallelProcessing* parallelProcessing;
};

// MemoryManager Tests
TEST_F(PerformanceOptimizationTest, MemoryManagerBasicAllocation) {
    // Test basic point allocation and deallocation
    PointFullData* point1 = memoryManager->allocatePoint();
    ASSERT_NE(point1, nullptr);
    
    PointFullData* point2 = memoryManager->allocatePoint();
    ASSERT_NE(point2, nullptr);
    ASSERT_NE(point1, point2);
    
    // Test deallocation
    memoryManager->deallocatePoint(point1);
    memoryManager->deallocatePoint(point2);
    
    // Verify memory stats
    auto stats = memoryManager->getMemoryStats();
    EXPECT_EQ(stats.activeObjects, 0);
}

TEST_F(PerformanceOptimizationTest, MemoryManagerPoolConfiguration) {
    MemoryManager::PoolConfig config;
    config.initialSize = 100;
    config.maxSize = 1000;
    config.chunkSize = 50;
    config.autoGrow = true;
    
    memoryManager->configurePool(config);
    
    // Allocate more than initial size to test growth
    std::vector<PointFullData*> points;
    for (int i = 0; i < 150; ++i) {
        PointFullData* point = memoryManager->allocatePoint();
        ASSERT_NE(point, nullptr);
        points.push_back(point);
    }
    
    auto stats = memoryManager->getMemoryStats();
    EXPECT_EQ(stats.activeObjects, 150);
    EXPECT_GT(stats.poolMemory, 0);
    
    // Clean up
    for (auto* point : points) {
        memoryManager->deallocatePoint(point);
    }
}

TEST_F(PerformanceOptimizationTest, MemoryManagerStreaming) {
    const size_t totalPoints = 10000;
    const size_t chunkSize = 1000;
    
    bool success = memoryManager->initializeStreaming(totalPoints, chunkSize);
    ASSERT_TRUE(success);
    
    size_t processedPoints = 0;
    while (memoryManager->hasMoreChunks()) {
        auto chunk = memoryManager->getNextChunk();
        EXPECT_LE(chunk.points.size(), chunkSize);
        EXPECT_EQ(chunk.totalChunks, (totalPoints + chunkSize - 1) / chunkSize);
        
        processedPoints += chunk.points.size();
        
        if (chunk.isLastChunk) {
            break;
        }
    }
    
    EXPECT_EQ(processedPoints, totalPoints);
    memoryManager->finalizeStreaming();
}

TEST_F(PerformanceOptimizationTest, MemoryManagerGarbageCollection) {
    QSignalSpy gcSpy(memoryManager, &MemoryManager::garbageCollectionCompleted);
    
    // Enable auto GC with short interval for testing
    memoryManager->enableAutoGC(true, 100); // 100ms interval
    
    // Allocate and deallocate many points to trigger GC
    for (int i = 0; i < 1000; ++i) {
        PointFullData* point = memoryManager->allocatePoint();
        memoryManager->deallocatePoint(point);
    }
    
    // Manually trigger GC
    memoryManager->triggerGarbageCollection();
    
    // Wait for GC signal
    EXPECT_TRUE(gcSpy.wait(1000));
    EXPECT_GE(gcSpy.count(), 1);
    
    memoryManager->enableAutoGC(false);
}

TEST_F(PerformanceOptimizationTest, MemoryManagerMemoryThreshold) {
    QSignalSpy thresholdSpy(memoryManager, &MemoryManager::memoryThresholdExceeded);
    
    // Set a low threshold for testing
    memoryManager->setMemoryThreshold(1024); // 1KB
    memoryManager->enableMemoryMonitoring(true);
    
    // Allocate enough points to exceed threshold
    std::vector<PointFullData*> points;
    for (int i = 0; i < 100; ++i) {
        points.push_back(memoryManager->allocatePoint());
    }
    
    // Wait for threshold signal
    QThread::msleep(1100); // Wait for monitoring timer
    
    // Clean up
    for (auto* point : points) {
        memoryManager->deallocatePoint(point);
    }
    
    memoryManager->enableMemoryMonitoring(false);
}

// ParallelProcessing Tests
TEST_F(PerformanceOptimizationTest, ParallelProcessingBasicExecution) {
    std::atomic<int> counter(0);
    
    auto future = parallelProcessing->executeAsync([&counter]() {
        counter.fetch_add(1);
    });
    
    future.waitForFinished();
    EXPECT_EQ(counter.load(), 1);
}

TEST_F(PerformanceOptimizationTest, ParallelProcessingMultipleTasksExecution) {
    const int numTasks = 10;
    std::atomic<int> counter(0);
    
    std::vector<std::function<void()>> tasks;
    for (int i = 0; i < numTasks; ++i) {
        tasks.push_back([&counter]() {
            counter.fetch_add(1);
        });
    }
    
    parallelProcessing->executeParallel(tasks);
    EXPECT_EQ(counter.load(), numTasks);
}

TEST_F(PerformanceOptimizationTest, ParallelProcessingPerformanceStats) {
    QSignalSpy statsSpy(parallelProcessing, &ParallelProcessing::performanceStatsUpdated);
    
    parallelProcessing->enableProfiling(true);
    
    // Execute some tasks to generate stats
    for (int i = 0; i < 5; ++i) {
        auto future = parallelProcessing->executeAsync([]() {
            QThread::msleep(10); // Small delay
        });
        future.waitForFinished();
    }
    
    // Wait for stats update
    EXPECT_TRUE(statsSpy.wait(2000));
    
    auto stats = parallelProcessing->getPerformanceStats();
    EXPECT_GE(stats.completedTasks, 5);
    EXPECT_GE(stats.throughput, 0.0);
    
    parallelProcessing->enableProfiling(false);
}

TEST_F(PerformanceOptimizationTest, ParallelProcessingThreadConfiguration) {
    int originalThreads = parallelProcessing->getMaxThreads();
    
    // Test thread count configuration
    parallelProcessing->setMaxThreads(4);
    EXPECT_EQ(parallelProcessing->getMaxThreads(), 4);
    
    // Test with different thread counts
    parallelProcessing->setMaxThreads(8);
    EXPECT_EQ(parallelProcessing->getMaxThreads(), 8);
    
    // Restore original
    parallelProcessing->setMaxThreads(originalThreads);
}

TEST_F(PerformanceOptimizationTest, ParallelProcessingBarrier) {
    const int numThreads = 4;
    std::atomic<int> phase1Counter(0);
    std::atomic<int> phase2Counter(0);
    
    ParallelProcessing::Barrier barrier(numThreads);
    
    std::vector<QFuture<void>> futures;
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(parallelProcessing->executeAsync([&]() {
            // Phase 1
            phase1Counter.fetch_add(1);
            
            // Wait for all threads to complete phase 1
            barrier.wait();
            
            // Phase 2 - should only start after all threads complete phase 1
            phase2Counter.fetch_add(1);
        }));
    }
    
    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.waitForFinished();
    }
    
    EXPECT_EQ(phase1Counter.load(), numThreads);
    EXPECT_EQ(phase2Counter.load(), numThreads);
}

// Performance Benchmarks
TEST_F(PerformanceOptimizationTest, MemoryAllocationPerformanceBenchmark) {
    const int numAllocations = 10000;
    
    QElapsedTimer timer;
    timer.start();
    
    std::vector<PointFullData*> points;
    points.reserve(numAllocations);
    
    for (int i = 0; i < numAllocations; ++i) {
        points.push_back(memoryManager->allocatePoint());
    }
    
    qint64 allocationTime = timer.elapsed();
    
    timer.restart();
    for (auto* point : points) {
        memoryManager->deallocatePoint(point);
    }
    qint64 deallocationTime = timer.elapsed();
    
    qDebug() << "Memory allocation benchmark:";
    qDebug() << "  Allocations:" << numAllocations;
    qDebug() << "  Allocation time:" << allocationTime << "ms";
    qDebug() << "  Deallocation time:" << deallocationTime << "ms";
    qDebug() << "  Avg allocation time:" << (double)allocationTime / numAllocations << "ms";
    
    // Performance assertions (adjust based on expected performance)
    EXPECT_LT(allocationTime, 1000); // Should complete in under 1 second
    EXPECT_LT(deallocationTime, 1000);
}

TEST_F(PerformanceOptimizationTest, ParallelProcessingSpeedupBenchmark) {
    const int numTasks = 1000;
    const int taskDuration = 1; // 1ms per task
    
    // Sequential execution benchmark
    QElapsedTimer timer;
    timer.start();
    
    for (int i = 0; i < numTasks; ++i) {
        QThread::msleep(taskDuration);
    }
    
    qint64 sequentialTime = timer.elapsed();
    
    // Parallel execution benchmark
    timer.restart();
    
    std::vector<std::function<void()>> tasks;
    for (int i = 0; i < numTasks; ++i) {
        tasks.push_back([]() {
            QThread::msleep(1);
        });
    }
    
    parallelProcessing->executeParallel(tasks);
    qint64 parallelTime = timer.elapsed();
    
    double speedup = (double)sequentialTime / parallelTime;
    
    qDebug() << "Parallel processing benchmark:";
    qDebug() << "  Tasks:" << numTasks;
    qDebug() << "  Sequential time:" << sequentialTime << "ms";
    qDebug() << "  Parallel time:" << parallelTime << "ms";
    qDebug() << "  Speedup:" << speedup << "x";
    
    // Should achieve some speedup (at least 2x on multi-core systems)
    EXPECT_GT(speedup, 1.5);
}

// Memory Leak Detection Test
TEST_F(PerformanceOptimizationTest, MemoryLeakDetection) {
    auto initialStats = memoryManager->getMemoryStats();
    size_t initialMemory = memoryManager->getTotalMemoryUsage();
    
    // Perform many allocation/deallocation cycles
    for (int cycle = 0; cycle < 100; ++cycle) {
        std::vector<PointFullData*> points;
        
        // Allocate points
        for (int i = 0; i < 100; ++i) {
            points.push_back(memoryManager->allocatePoint());
        }
        
        // Deallocate all points
        for (auto* point : points) {
            memoryManager->deallocatePoint(point);
        }
    }
    
    // Force garbage collection
    memoryManager->triggerGarbageCollection();
    
    auto finalStats = memoryManager->getMemoryStats();
    size_t finalMemory = memoryManager->getTotalMemoryUsage();
    
    // Memory usage should not have grown significantly
    EXPECT_EQ(finalStats.activeObjects, initialStats.activeObjects);
    
    // Allow for some memory growth due to pool expansion, but not excessive
    double memoryGrowthRatio = (double)finalMemory / initialMemory;
    EXPECT_LT(memoryGrowthRatio, 2.0); // Less than 2x growth
    
    qDebug() << "Memory leak detection:";
    qDebug() << "  Initial memory:" << initialMemory << "bytes";
    qDebug() << "  Final memory:" << finalMemory << "bytes";
    qDebug() << "  Growth ratio:" << memoryGrowthRatio;
}
