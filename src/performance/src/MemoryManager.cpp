#include "MemoryManager.h"
#include <QDebug>
#include <QThread>
#include <algorithm>
#include <cstring>

// PointDataPool Implementation
MemoryManager::PointDataPool::PointDataPool(const PoolConfig& config)
    : m_config(config), m_totalAllocated(0), m_activeCount(0) {
    // Pre-allocate initial pool
    growPool();
}

MemoryManager::PointDataPool::~PointDataPool() {
    clear();
}

PointFullData* MemoryManager::PointDataPool::allocate() {
    QMutexLocker locker(&m_mutex);
    
    if (m_freeObjects.empty()) {
        if (m_config.autoGrow && m_chunks.size() * m_config.chunkSize < m_config.maxSize) {
            growPool();
        }
        
        if (m_freeObjects.empty()) {
            // Pool exhausted, fall back to regular allocation
            auto* point = new PointFullData();
            m_activeObjects[point] = m_chunks.size(); // Mark as non-pool allocation
            m_activeCount++;
            return point;
        }
    }
    
    PointFullData* point = m_freeObjects.front();
    m_freeObjects.pop();
    
    // Reset the point data
    std::memset(point, 0, sizeof(PointFullData));
    
    m_activeObjects[point] = 0; // Mark as pool allocation
    m_activeCount++;
    
    return point;
}

void MemoryManager::PointDataPool::deallocate(PointFullData* ptr) {
    if (!ptr) return;
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_activeObjects.find(ptr);
    if (it == m_activeObjects.end()) {
        qWarning() << "MemoryManager: Attempted to deallocate unknown pointer";
        return;
    }
    
    if (it->second == m_chunks.size()) {
        // Non-pool allocation, delete directly
        delete ptr;
    } else {
        // Pool allocation, return to pool
        m_freeObjects.push(ptr);
    }
    
    m_activeObjects.erase(it);
    m_activeCount--;
}

void MemoryManager::PointDataPool::clear() {
    QMutexLocker locker(&m_mutex);
    
    // Delete any non-pool allocations
    for (const auto& pair : m_activeObjects) {
        if (pair.second == m_chunks.size()) {
            delete pair.first;
        }
    }
    
    m_activeObjects.clear();
    m_chunks.clear();
    std::queue<PointFullData*> empty;
    m_freeObjects.swap(empty);
    m_totalAllocated = 0;
    m_activeCount = 0;
}

void MemoryManager::PointDataPool::growPool() {
    auto chunk = std::make_unique<PointFullData[]>(m_config.chunkSize);
    PointFullData* chunkPtr = chunk.get();
    
    // Add all objects in chunk to free list
    for (size_t i = 0; i < m_config.chunkSize; ++i) {
        m_freeObjects.push(&chunkPtr[i]);
    }
    
    m_chunks.push_back(std::move(chunk));
    m_totalAllocated += m_config.chunkSize * sizeof(PointFullData);
    
    qDebug() << "MemoryManager: Pool grown to" << m_chunks.size() << "chunks,"
             << m_totalAllocated << "bytes allocated";
}

size_t MemoryManager::PointDataPool::getMemoryUsage() const {
    QMutexLocker locker(&m_mutex);
    return m_totalAllocated;
}

size_t MemoryManager::PointDataPool::getActiveCount() const {
    QMutexLocker locker(&m_mutex);
    return m_activeCount;
}

bool MemoryManager::PointDataPool::isValidPointer(PointFullData* ptr) const {
    return m_activeObjects.find(ptr) != m_activeObjects.end();
}

// MemoryManager Implementation
MemoryManager& MemoryManager::instance() {
    static MemoryManager instance;
    return instance;
}

MemoryManager::MemoryManager(QObject* parent)
    : QObject(parent)
    , m_streamingActive(false)
    , m_totalStreamingPoints(0)
    , m_streamingChunkSize(0)
    , m_currentStreamingIndex(0)
    , m_monitoringEnabled(false)
    , m_memoryThreshold(2ULL * 1024 * 1024 * 1024) // 2GB default
    , m_autoGCEnabled(false) {
    
    // Initialize with default configuration
    m_poolConfig = PoolConfig();
    m_pointPool = std::make_unique<PointDataPool>(m_poolConfig);
    
    // Setup monitoring timer
    m_monitoringTimer = new QTimer(this);
    connect(m_monitoringTimer, &QTimer::timeout, this, &MemoryManager::updateMemoryStats);
    
    // Setup GC timer
    m_gcTimer = new QTimer(this);
    connect(m_gcTimer, &QTimer::timeout, this, &MemoryManager::performGarbageCollection);
    
    qDebug() << "MemoryManager initialized with pool size:" << m_poolConfig.initialSize;
}

MemoryManager::~MemoryManager() {
    finalizeStreaming();
    clearPools();
}

void MemoryManager::configurePool(const PoolConfig& config) {
    m_poolConfig = config;
    m_pointPool = std::make_unique<PointDataPool>(config);
    qDebug() << "MemoryManager: Pool reconfigured";
}

PointFullData* MemoryManager::allocatePoint() {
    PointFullData* point = m_pointPool->allocate();
    
    QMutexLocker locker(&m_statsMutex);
    m_stats.activeObjects++;
    if (point) {
        m_stats.poolHits++;
    } else {
        m_stats.poolMisses++;
    }
    m_stats.hitRatio = static_cast<double>(m_stats.poolHits) / 
                       (m_stats.poolHits + m_stats.poolMisses);
    
    return point;
}

void MemoryManager::deallocatePoint(PointFullData* point) {
    if (!point) return;
    
    m_pointPool->deallocate(point);
    
    QMutexLocker locker(&m_statsMutex);
    if (m_stats.activeObjects > 0) {
        m_stats.activeObjects--;
    }
}

void MemoryManager::clearPools() {
    m_pointPool->clear();
    
    QMutexLocker locker(&m_statsMutex);
    m_stats = MemoryStats(); // Reset stats
}

bool MemoryManager::initializeStreaming(size_t totalPoints, size_t chunkSize) {
    if (m_streamingActive) {
        qWarning() << "MemoryManager: Streaming already active";
        return false;
    }
    
    m_totalStreamingPoints = totalPoints;
    m_streamingChunkSize = chunkSize;
    m_currentStreamingIndex = 0;
    m_streamingActive = true;
    
    // Pre-calculate number of chunks
    size_t numChunks = (totalPoints + chunkSize - 1) / chunkSize;
    m_streamingChunks.reserve(numChunks);
    
    qDebug() << "MemoryManager: Streaming initialized for" << totalPoints 
             << "points in" << numChunks << "chunks";
    
    return true;
}

MemoryManager::StreamingChunk MemoryManager::getNextChunk() {
    StreamingChunk chunk;
    
    if (!m_streamingActive || !hasMoreChunks()) {
        return chunk;
    }
    
    size_t remainingPoints = m_totalStreamingPoints - m_currentStreamingIndex;
    size_t chunkPoints = std::min(m_streamingChunkSize, remainingPoints);
    
    chunk.points.reserve(chunkPoints);
    chunk.chunkIndex = m_currentStreamingIndex / m_streamingChunkSize;
    chunk.totalChunks = (m_totalStreamingPoints + m_streamingChunkSize - 1) / m_streamingChunkSize;
    chunk.isLastChunk = (m_currentStreamingIndex + chunkPoints >= m_totalStreamingPoints);
    
    // Allocate points for this chunk
    for (size_t i = 0; i < chunkPoints; ++i) {
        PointFullData point;
        chunk.points.push_back(point);
    }
    
    chunk.memoryUsage = chunkPoints * sizeof(PointFullData);
    m_currentStreamingIndex += chunkPoints;
    
    emit streamingChunkReady(chunk);
    return chunk;
}

bool MemoryManager::hasMoreChunks() const {
    return m_streamingActive && m_currentStreamingIndex < m_totalStreamingPoints;
}

void MemoryManager::finalizeStreaming() {
    m_streamingActive = false;
    m_streamingChunks.clear();
    m_currentStreamingIndex = 0;
    m_totalStreamingPoints = 0;
    
    qDebug() << "MemoryManager: Streaming finalized";
}

MemoryManager::MemoryStats MemoryManager::getMemoryStats() const {
    QMutexLocker locker(&m_statsMutex);
    MemoryStats stats = m_stats;
    stats.poolMemory = m_pointPool->getMemoryUsage();
    stats.activeObjects = m_pointPool->getActiveCount();
    return stats;
}

size_t MemoryManager::getTotalMemoryUsage() const {
    return m_pointPool->getMemoryUsage();
}

void MemoryManager::enableMemoryMonitoring(bool enabled) {
    m_monitoringEnabled = enabled;
    
    if (enabled) {
        m_monitoringTimer->start(1000); // Update every second
        qDebug() << "MemoryManager: Memory monitoring enabled";
    } else {
        m_monitoringTimer->stop();
        qDebug() << "MemoryManager: Memory monitoring disabled";
    }
}

void MemoryManager::setMemoryThreshold(size_t thresholdBytes) {
    m_memoryThreshold = thresholdBytes;
    qDebug() << "MemoryManager: Memory threshold set to" << thresholdBytes << "bytes";
}

void MemoryManager::triggerGarbageCollection() {
    performGarbageCollection();
}

void MemoryManager::enableAutoGC(bool enabled, int intervalMs) {
    m_autoGCEnabled = enabled;
    
    if (enabled) {
        m_gcTimer->start(intervalMs);
        qDebug() << "MemoryManager: Auto GC enabled with interval" << intervalMs << "ms";
    } else {
        m_gcTimer->stop();
        qDebug() << "MemoryManager: Auto GC disabled";
    }
}

void MemoryManager::performGarbageCollection() {
    size_t freedBytes = 0;
    
    // Perform garbage collection logic here
    // For now, just clear any cached data
    
    QMutexLocker locker(&m_statsMutex);
    m_stats.gcCollections++;
    m_stats.gcFreedBytes += freedBytes;
    
    emit garbageCollectionCompleted(freedBytes);
    
    qDebug() << "MemoryManager: GC completed, freed" << freedBytes << "bytes";
}

void MemoryManager::updateMemoryStats() {
    updateMemoryUsage();
    checkMemoryThreshold();
}

void MemoryManager::updateMemoryUsage() {
    size_t totalUsage = getTotalMemoryUsage();
    emit memoryUsageChanged(totalUsage);
}

void MemoryManager::checkMemoryThreshold() {
    size_t currentUsage = getTotalMemoryUsage();
    if (currentUsage > m_memoryThreshold) {
        emit memoryThresholdExceeded(currentUsage, m_memoryThreshold);
        
        if (m_autoGCEnabled) {
            performGarbageCollection();
        }
    }
}
