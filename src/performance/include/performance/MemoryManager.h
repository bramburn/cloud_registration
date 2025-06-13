#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include "core/pointdata.h"

/**
 * @brief Advanced memory management system for large point cloud datasets
 * 
 * This class provides smart memory management utilities including:
 * - Memory pooling for PointFullData objects
 * - Streaming algorithms for datasets larger than RAM
 * - Garbage collection and memory monitoring
 * - Memory usage tracking and optimization
 * 
 * Sprint 7 Requirements:
 * - Handle 100M+ point datasets efficiently
 * - Reduce allocation/deallocation overhead
 * - Streaming support for large files
 * - Memory leak prevention and monitoring
 */
class MemoryManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Memory pool configuration
     */
    struct PoolConfig {
        size_t initialSize = 1024;      // Initial pool size
        size_t maxSize = 10240;         // Maximum pool size
        size_t chunkSize = 256;         // Allocation chunk size
        bool autoGrow = true;           // Allow pool growth
        bool enableGC = true;           // Enable garbage collection
    };

    /**
     * @brief Memory usage statistics
     */
    struct MemoryStats {
        size_t totalAllocated = 0;      // Total memory allocated
        size_t poolMemory = 0;          // Memory in pools
        size_t activeObjects = 0;       // Active object count
        size_t poolHits = 0;            // Pool allocation hits
        size_t poolMisses = 0;          // Pool allocation misses
        double hitRatio = 0.0;          // Pool hit ratio
        size_t gcCollections = 0;       // GC collection count
        size_t gcFreedBytes = 0;        // Bytes freed by GC
    };

    /**
     * @brief Streaming chunk for large dataset processing
     */
    struct StreamingChunk {
        std::vector<PointFullData> points;
        size_t chunkIndex = 0;
        size_t totalChunks = 0;
        bool isLastChunk = false;
        size_t memoryUsage = 0;
    };

    /**
     * @brief Memory pool for PointFullData objects
     */
    class PointDataPool {
    public:
        explicit PointDataPool(const PoolConfig& config);
        ~PointDataPool();

        PointFullData* allocate();
        void deallocate(PointFullData* ptr);
        void clear();
        size_t getMemoryUsage() const;
        size_t getActiveCount() const;

    private:
        PoolConfig m_config;
        std::queue<PointFullData*> m_freeObjects;
        std::vector<std::unique_ptr<PointFullData[]>> m_chunks;
        std::unordered_map<PointFullData*, size_t> m_activeObjects;
        mutable QMutex m_mutex;
        size_t m_totalAllocated;
        size_t m_activeCount;

        void growPool();
        bool isValidPointer(PointFullData* ptr) const;
    };

public:
    static MemoryManager& instance();
    explicit MemoryManager(QObject* parent = nullptr);
    ~MemoryManager();

    // Pool management
    void configurePool(const PoolConfig& config);
    PointFullData* allocatePoint();
    void deallocatePoint(PointFullData* point);
    void clearPools();

    // Streaming support
    bool initializeStreaming(size_t totalPoints, size_t chunkSize = 1000000);
    StreamingChunk getNextChunk();
    bool hasMoreChunks() const;
    void finalizeStreaming();

    // Memory monitoring
    MemoryStats getMemoryStats() const;
    size_t getTotalMemoryUsage() const;
    void enableMemoryMonitoring(bool enabled);
    void setMemoryThreshold(size_t thresholdBytes);

    // Garbage collection
    void triggerGarbageCollection();
    void enableAutoGC(bool enabled, int intervalMs = 30000);

signals:
    void memoryUsageChanged(size_t totalBytes);
    void memoryThresholdExceeded(size_t currentBytes, size_t threshold);
    void garbageCollectionCompleted(size_t freedBytes);
    void streamingChunkReady(const StreamingChunk& chunk);

private slots:
    void performGarbageCollection();
    void updateMemoryStats();

private:
    std::unique_ptr<PointDataPool> m_pointPool;
    PoolConfig m_poolConfig;
    
    // Streaming state
    bool m_streamingActive;
    size_t m_totalStreamingPoints;
    size_t m_streamingChunkSize;
    size_t m_currentStreamingIndex;
    std::vector<StreamingChunk> m_streamingChunks;
    
    // Memory monitoring
    bool m_monitoringEnabled;
    size_t m_memoryThreshold;
    QTimer* m_monitoringTimer;
    QTimer* m_gcTimer;
    mutable QMutex m_statsMutex;
    MemoryStats m_stats;
    
    // Garbage collection
    bool m_autoGCEnabled;
    std::vector<PointFullData*> m_gcCandidates;
    
    void updateMemoryUsage();
    void checkMemoryThreshold();
    size_t calculateObjectMemoryUsage(const PointFullData* point) const;
};

#endif // MEMORYMANAGER_H
