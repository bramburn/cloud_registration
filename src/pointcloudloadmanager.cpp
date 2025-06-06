#include "pointcloudloadmanager.h"
#include "sqlitemanager.h"
#include "projecttreemodel.h"
#include "projectmanager.h"
#include "lasparser.h"
#include "e57parserlib.h"
#include "E57DataManager.h"
#include "loadingsettings.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>
#include <QApplication>
#include <QMessageBox>

PointCloudLoadManager::PointCloudLoadManager(QObject *parent)
    : QObject(parent)
    , m_sqliteManager(nullptr)
    , m_treeModel(nullptr)
    , m_memoryLimitMB(DEFAULT_MEMORY_LIMIT_MB)
    , m_currentMemoryUsage(0)
    , m_memoryCheckTimer(new QTimer(this))
{
    // Setup memory monitoring timer
    m_memoryCheckTimer->setInterval(MEMORY_CHECK_INTERVAL_MS);
    connect(m_memoryCheckTimer, &QTimer::timeout, this, &PointCloudLoadManager::onMemoryCheckTimer);
    m_memoryCheckTimer->start();
    
    qDebug() << "PointCloudLoadManager initialized with memory limit:" << m_memoryLimitMB << "MB";
}

PointCloudLoadManager::~PointCloudLoadManager()
{
    // Unload all scans to free memory
    QStringList loadedScans = getLoadedScans();
    for (const QString &scanId : loadedScans) {
        unloadScan(scanId);
    }
    
    qDebug() << "PointCloudLoadManager destroyed, freed memory for" << loadedScans.size() << "scans";
}

void PointCloudLoadManager::setSQLiteManager(SQLiteManager *manager)
{
    m_sqliteManager = manager;
}

void PointCloudLoadManager::setProjectTreeModel(ProjectTreeModel *model)
{
    m_treeModel = model;
}

bool PointCloudLoadManager::loadScan(const QString &scanId)
{
    QMutexLocker locker(&m_stateMutex);
    
    if (scanId.isEmpty()) {
        m_lastError = "Invalid scan ID";
        return false;
    }
    
    // Check if already loaded
    if (isScanLoaded(scanId)) {
        qDebug() << "Scan already loaded:" << scanId;
        return true;
    }
    
    // Update state to loading
    updateScanState(scanId, LoadedState::Loading);
    
    // Perform the actual loading
    bool success = loadScanData(scanId);
    
    if (success) {
        updateScanState(scanId, LoadedState::Loaded);
        emit scanLoaded(scanId);
        qDebug() << "Successfully loaded scan:" << scanId;
    } else {
        updateScanState(scanId, LoadedState::Error, m_lastError);
        emit scanLoadFailed(scanId, m_lastError);
        qDebug() << "Failed to load scan:" << scanId << "Error:" << m_lastError;
    }
    
    return success;
}

bool PointCloudLoadManager::unloadScan(const QString &scanId)
{
    QMutexLocker locker(&m_stateMutex);
    
    if (scanId.isEmpty()) {
        m_lastError = "Invalid scan ID";
        return false;
    }
    
    bool success = unloadScanData(scanId);
    
    if (success) {
        updateScanState(scanId, LoadedState::Unloaded);
        emit scanUnloaded(scanId);
        qDebug() << "Successfully unloaded scan:" << scanId;
    }
    
    return success;
}

LoadedState PointCloudLoadManager::getScanLoadedState(const QString &scanId) const
{
    QMutexLocker locker(&m_stateMutex);
    
    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end()) {
        return it.value()->state;
    }
    
    return LoadedState::Unloaded;
}

bool PointCloudLoadManager::loadCluster(const QString &clusterId)
{
    if (clusterId.isEmpty()) {
        m_lastError = "Invalid cluster ID";
        return false;
    }
    
    QStringList scanIds = getClusterScanIds(clusterId);
    if (scanIds.isEmpty()) {
        qDebug() << "No scans found in cluster:" << clusterId;
        return true; // Not an error, just empty cluster
    }
    
    bool allSuccess = true;
    int loadedCount = 0;
    
    for (const QString &scanId : scanIds) {
        if (loadScan(scanId)) {
            loadedCount++;
        } else {
            allSuccess = false;
            qDebug() << "Failed to load scan in cluster:" << scanId;
        }
    }
    
    if (allSuccess) {
        emit clusterLoaded(clusterId);
        qDebug() << "Successfully loaded all scans in cluster:" << clusterId << "(" << loadedCount << "scans)";
    } else {
        qDebug() << "Partially loaded cluster:" << clusterId << "(" << loadedCount << "of" << scanIds.size() << "scans)";
    }
    
    return allSuccess;
}

bool PointCloudLoadManager::unloadCluster(const QString &clusterId)
{
    if (clusterId.isEmpty()) {
        m_lastError = "Invalid cluster ID";
        return false;
    }
    
    QStringList scanIds = getClusterScanIds(clusterId);
    if (scanIds.isEmpty()) {
        return true; // Empty cluster, nothing to unload
    }
    
    bool allSuccess = true;
    int unloadedCount = 0;
    
    for (const QString &scanId : scanIds) {
        if (unloadScan(scanId)) {
            unloadedCount++;
        } else {
            allSuccess = false;
        }
    }
    
    if (allSuccess) {
        emit clusterUnloaded(clusterId);
        qDebug() << "Successfully unloaded all scans in cluster:" << clusterId << "(" << unloadedCount << "scans)";
    }
    
    return allSuccess;
}

QStringList PointCloudLoadManager::getClusterScanIds(const QString &clusterId) const
{
    QStringList scanIds;
    
    if (!m_sqliteManager) {
        return scanIds;
    }
    
    // Get all scans and filter by cluster
    QList<ScanInfo> allScans = m_sqliteManager->getAllScans();
    for (const ScanInfo &scan : allScans) {
        if (scan.parentClusterId == clusterId) {
            scanIds.append(scan.scanId);
        }
    }
    
    // TODO: Add recursive support for sub-clusters
    
    return scanIds;
}

bool PointCloudLoadManager::viewPointCloud(const QString &itemId, const QString &itemType)
{
    if (itemType == "scan") {
        return viewScan(itemId);
    } else if (itemType == "cluster") {
        return viewCluster(itemId);
    }

    m_lastError = "Invalid item type for viewing: " + itemType;
    emit pointCloudViewFailed(m_lastError);
    return false;
}

bool PointCloudLoadManager::viewScan(const QString &scanId)
{
    // Ensure scan is loaded
    if (!loadScan(scanId)) {
        emit pointCloudViewFailed(m_lastError);
        return false;
    }

    // Get point cloud data for rendering
    std::vector<float> points = getScanPointCloudData(scanId);
    if (points.empty()) {
        m_lastError = "No point cloud data available for scan: " + scanId;
        emit pointCloudViewFailed(m_lastError);
        return false;
    }

    QString sourceInfo = QString("Scan: %1 (%2 points)").arg(scanId).arg(points.size() / 3);
    emit pointCloudDataReady(points, sourceInfo);
    return true;
}

bool PointCloudLoadManager::viewCluster(const QString &clusterId)
{
    // Get all scan IDs in cluster
    QStringList scanIds = getClusterScanIds(clusterId);
    if (scanIds.isEmpty()) {
        m_lastError = "No scans found in cluster: " + clusterId;
        emit pointCloudViewFailed(m_lastError);
        return false;
    }

    // Load all scans in cluster
    bool allLoaded = true;
    for (const QString &scanId : scanIds) {
        if (!loadScan(scanId)) {
            allLoaded = false;
            qDebug() << "Failed to load scan in cluster:" << scanId;
        }
    }

    if (!allLoaded) {
        m_lastError = "Failed to load some scans in cluster: " + clusterId;
        emit pointCloudViewFailed(m_lastError);
        return false;
    }

    // Get aggregated point cloud data
    std::vector<float> points = getAggregatedPointCloudData(scanIds);
    if (points.empty()) {
        m_lastError = "No point cloud data available for cluster: " + clusterId;
        emit pointCloudViewFailed(m_lastError);
        return false;
    }

    QString sourceInfo = QString("Cluster: %1 (%2 scans, %3 points)")
                        .arg(clusterId).arg(scanIds.size()).arg(points.size() / 3);
    emit pointCloudDataReady(points, sourceInfo);
    return true;
}

size_t PointCloudLoadManager::getTotalMemoryUsage() const
{
    return m_currentMemoryUsage;
}

void PointCloudLoadManager::setMemoryLimit(size_t limitMB)
{
    m_memoryLimitMB = limitMB;
    qDebug() << "Memory limit set to:" << limitMB << "MB";
    
    // Check if we need to enforce the new limit immediately
    if (m_currentMemoryUsage > limitMB * 1024 * 1024) {
        enforceMemoryLimit();
    }
}

bool PointCloudLoadManager::isScanLoaded(const QString &scanId) const
{
    return getScanLoadedState(scanId) == LoadedState::Loaded;
}

QStringList PointCloudLoadManager::getLoadedScans() const
{
    QMutexLocker locker(&m_stateMutex);
    
    QStringList loadedScans;
    for (auto it = m_scanStates.begin(); it != m_scanStates.end(); ++it) {
        if (it.value()->state == LoadedState::Loaded) {
            loadedScans.append(it.key());
        }
    }
    
    return loadedScans;
}

QString PointCloudLoadManager::getLastError() const
{
    return m_lastError;
}

// Public slots
void PointCloudLoadManager::onLoadScanRequested(const QString &scanId)
{
    loadScan(scanId);
}

void PointCloudLoadManager::onUnloadScanRequested(const QString &scanId)
{
    unloadScan(scanId);
}

void PointCloudLoadManager::onLoadClusterRequested(const QString &clusterId)
{
    loadCluster(clusterId);
}

void PointCloudLoadManager::onUnloadClusterRequested(const QString &clusterId)
{
    unloadCluster(clusterId);
}

void PointCloudLoadManager::onViewPointCloudRequested(const QString &itemId, const QString &itemType)
{
    viewPointCloud(itemId, itemType);
}

void PointCloudLoadManager::onMemoryCheckTimer()
{
    updateMemoryUsage();

    if (m_currentMemoryUsage > m_memoryLimitMB * 1024 * 1024) {
        emit memoryLimitExceeded(m_currentMemoryUsage, m_memoryLimitMB * 1024 * 1024);
        enforceMemoryLimit();
    }
}

// Private methods implementation
bool PointCloudLoadManager::loadScanData(const QString &scanId)
{
    if (!m_sqliteManager) {
        m_lastError = "SQLite manager not available";
        return false;
    }

    // Get scan information from database
    ScanInfo scan = m_sqliteManager->getScanById(scanId);
    if (!scan.isValid()) {
        m_lastError = "Scan not found in database: " + scanId;
        return false;
    }

    QString filePath = getScanFilePath(scanId);
    if (filePath.isEmpty()) {
        m_lastError = "Could not determine file path for scan: " + scanId;
        return false;
    }

    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_lastError = "Scan file not found: " + filePath;
        return false;
    }

    // Parse the point cloud file
    auto pointCloudData = parsePointCloudFile(filePath);
    if (!pointCloudData || !pointCloudData->isValid()) {
        m_lastError = "Failed to parse point cloud file: " + filePath;
        return false;
    }

    // Check memory limit before adding
    size_t newMemoryUsage = m_currentMemoryUsage + pointCloudData->memoryUsage;
    if (newMemoryUsage > m_memoryLimitMB * 1024 * 1024) {
        // Try to free some memory first
        evictLeastRecentlyUsed();

        // Check again
        newMemoryUsage = m_currentMemoryUsage + pointCloudData->memoryUsage;
        if (newMemoryUsage > m_memoryLimitMB * 1024 * 1024) {
            m_lastError = QString("Memory limit exceeded. Required: %1 MB, Available: %2 MB")
                         .arg(pointCloudData->memoryUsage / (1024 * 1024))
                         .arg((m_memoryLimitMB * 1024 * 1024 - m_currentMemoryUsage) / (1024 * 1024));
            return false;
        }
    }

    // Create or update scan state
    if (!m_scanStates.contains(scanId)) {
        m_scanStates[scanId] = std::make_unique<ScanLoadState>(scanId);
    }

    auto &scanState = m_scanStates[scanId];
    scanState->data = std::move(pointCloudData);
    scanState->lastAccessed = QDateTime::currentDateTime();
    scanState->state = LoadedState::Loaded;
    scanState->errorMessage.clear();

    // Update memory usage
    m_currentMemoryUsage += scanState->data->memoryUsage;

    qDebug() << "Loaded scan data:" << scanId << "Points:" << scanState->data->pointCount
             << "Memory:" << (scanState->data->memoryUsage / (1024 * 1024)) << "MB";

    return true;
}

bool PointCloudLoadManager::unloadScanData(const QString &scanId)
{
    auto it = m_scanStates.find(scanId);
    if (it == m_scanStates.end()) {
        return true; // Already unloaded
    }

    auto &scanState = it.value();
    if (scanState->data && scanState->data->isValid()) {
        m_currentMemoryUsage -= scanState->data->memoryUsage;
        scanState->data->clear();
        qDebug() << "Unloaded scan data:" << scanId << "Remaining memory:" << (m_currentMemoryUsage / (1024 * 1024)) << "MB";
    }

    scanState->state = LoadedState::Unloaded;
    scanState->errorMessage.clear();

    return true;
}

std::unique_ptr<PointCloudData> PointCloudLoadManager::parsePointCloudFile(const QString &filePath)
{
    auto data = std::make_unique<PointCloudData>();
    data->filePath = filePath;
    data->loadTime = QDateTime::currentDateTime();

    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    try {
        if (extension == "las" || extension == "laz") {
            // Use LAS parser
            LasParser parser;
            LoadingSettings settings;
            settings.method = LoadingMethod::FullLoad; // For now, always full load

            data->points = parser.parse(filePath, settings);
            data->pointCount = data->points.size() / 3; // Assuming XYZ format

        } else if (extension == "e57") {
            // Use E57DataManager for enhanced E57 support
            E57DataManager e57Manager;

            // Import all scans from the E57 file
            QVector<QVector<PointData>> scans = e57Manager.importE57File(filePath);

            if (scans.isEmpty()) {
                qDebug() << "No scans found in E57 file:" << filePath;
                return nullptr;
            }

            // For now, combine all scans into a single point cloud
            // In the future, this could be enhanced to handle multiple scans separately
            std::vector<float> combinedPoints;
            size_t totalPoints = 0;

            // Calculate total points for efficient allocation
            for (const auto& scan : scans) {
                totalPoints += scan.size();
            }

            combinedPoints.reserve(totalPoints * 3); // XYZ per point

            // Combine all scans into a single point cloud
            for (const auto& scan : scans) {
                for (const auto& point : scan) {
                    combinedPoints.push_back(static_cast<float>(point.x));
                    combinedPoints.push_back(static_cast<float>(point.y));
                    combinedPoints.push_back(static_cast<float>(point.z));
                }
            }

            data->points = combinedPoints;
            data->pointCount = combinedPoints.size() / 3;

            qDebug() << "E57DataManager: Combined" << scans.size() << "scans into"
                     << data->pointCount << "points";

        } else {
            qDebug() << "Unsupported file format:" << extension;
            return nullptr;
        }

        // Calculate memory usage (rough estimate)
        data->memoryUsage = data->points.size() * sizeof(float) + sizeof(PointCloudData);

        qDebug() << "Parsed point cloud file:" << filePath
                 << "Points:" << data->pointCount
                 << "Memory:" << (data->memoryUsage / (1024 * 1024)) << "MB";

    } catch (const std::exception &e) {
        m_lastError = QString("Error parsing file %1: %2").arg(filePath, e.what());
        qDebug() << m_lastError;
        return nullptr;
    }

    return data;
}

void PointCloudLoadManager::updateMemoryUsage()
{
    size_t totalUsage = 0;

    QMutexLocker locker(&m_stateMutex);
    for (auto it = m_scanStates.begin(); it != m_scanStates.end(); ++it) {
        auto &scanState = it.value();
        if (scanState->data && scanState->data->isValid()) {
            // Sprint 3.4: Include LOD memory in total usage
            totalUsage += scanState->data->getTotalMemoryUsage();
        }
    }

    if (m_currentMemoryUsage != totalUsage) {
        m_currentMemoryUsage = totalUsage;
        // Sprint 3.4: Emit memory usage changed signal
        emit memoryUsageChanged(m_currentMemoryUsage);
    }
}

void PointCloudLoadManager::evictLeastRecentlyUsed()
{
    QMutexLocker locker(&m_stateMutex);

    // Find the least recently used loaded scan
    QString lruScanId;
    QDateTime oldestAccess = QDateTime::currentDateTime();

    for (auto it = m_scanStates.begin(); it != m_scanStates.end(); ++it) {
        auto &scanState = it.value();
        if (scanState->state == LoadedState::Loaded &&
            scanState->data && scanState->data->isValid()) {
            if (scanState->lastAccessed < oldestAccess) {
                oldestAccess = scanState->lastAccessed;
                lruScanId = it.key();
            }
        }
    }

    if (!lruScanId.isEmpty()) {
        qDebug() << "Evicting least recently used scan:" << lruScanId;
        unloadScanData(lruScanId);
        updateScanState(lruScanId, LoadedState::Unloaded);
    }
}

void PointCloudLoadManager::enforceMemoryLimit()
{
    while (m_currentMemoryUsage > m_memoryLimitMB * 1024 * 1024) {
        QStringList loadedScans = getLoadedScans();
        if (loadedScans.isEmpty()) {
            break; // No more scans to unload
        }

        evictLeastRecentlyUsed();
        updateMemoryUsage();
    }
}

QString PointCloudLoadManager::getScanFilePath(const QString &scanId) const
{
    if (!m_sqliteManager) {
        return QString();
    }

    ScanInfo scan = m_sqliteManager->getScanById(scanId);
    if (!scan.isValid()) {
        return QString();
    }

    // Return the absolute path if available, otherwise construct it
    if (!scan.absolutePath.isEmpty()) {
        return scan.absolutePath;
    }

    // Construct absolute path from relative path
    // This would need project path information - for now return relative path
    return scan.filePathRelative;
}

void PointCloudLoadManager::updateScanState(const QString &scanId, LoadedState state, const QString &error)
{
    // Update internal state
    if (!m_scanStates.contains(scanId)) {
        m_scanStates[scanId] = std::make_unique<ScanLoadState>(scanId);
    }

    auto &scanState = m_scanStates[scanId];
    scanState->state = state;
    scanState->errorMessage = error;

    // Update tree model if available
    if (m_treeModel) {
        m_treeModel->setScanLoadedState(scanId, state);
    }
}

void PointCloudLoadManager::logMemoryUsage() const
{
    qDebug() << "Memory usage:" << (m_currentMemoryUsage / (1024 * 1024)) << "MB /"
             << m_memoryLimitMB << "MB ("
             << (m_currentMemoryUsage * 100 / (m_memoryLimitMB * 1024 * 1024)) << "%)";
}

std::vector<float> PointCloudLoadManager::getAggregatedPointCloudData(const QStringList &scanIds)
{
    std::vector<float> aggregatedPoints;

    QMutexLocker locker(&m_stateMutex);

    // Calculate total size for efficient memory allocation
    size_t totalPoints = 0;
    for (const QString &scanId : scanIds) {
        auto it = m_scanStates.find(scanId);
        if (it != m_scanStates.end() && it.value()->data && it.value()->data->isValid()) {
            totalPoints += it.value()->data->pointCount;
        }
    }

    if (totalPoints == 0) {
        return aggregatedPoints;
    }

    // Reserve space for all points (3 floats per point)
    aggregatedPoints.reserve(totalPoints * 3);

    // Aggregate point data from all loaded scans
    for (const QString &scanId : scanIds) {
        auto it = m_scanStates.find(scanId);
        if (it != m_scanStates.end() && it.value()->data && it.value()->data->isValid()) {
            const auto &points = it.value()->data->points;
            aggregatedPoints.insert(aggregatedPoints.end(), points.begin(), points.end());

            // Update last accessed time
            it.value()->lastAccessed = QDateTime::currentDateTime();
        }
    }

    qDebug() << "Aggregated point cloud data from" << scanIds.size() << "scans:"
             << (aggregatedPoints.size() / 3) << "total points";

    return aggregatedPoints;
}

std::vector<float> PointCloudLoadManager::getScanPointCloudData(const QString &scanId)
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end() && it.value()->data && it.value()->data->isValid()) {
        // Update last accessed time
        it.value()->lastAccessed = QDateTime::currentDateTime();

        // Sprint 3.4: Return LOD data if active, otherwise return full data
        if (it.value()->data->lodActive && !it.value()->data->lodPoints.empty()) {
            return it.value()->data->lodPoints;
        }
        return it.value()->data->points;
    }

    return std::vector<float>();
}

// Sprint 3.4: LOD functionality implementation
QFuture<bool> PointCloudLoadManager::loadScanWithLOD(const QString &scanId, float subsampleRate)
{
    return QtConcurrent::run([this, scanId, subsampleRate]() {
        // First load the scan normally
        bool success = loadScan(scanId);
        if (!success) {
            return false;
        }

        // Generate LOD data
        generateLODForScan(scanId, subsampleRate);
        return true;
    });
}

std::vector<float> PointCloudLoadManager::subsamplePointCloud(const std::vector<float> &points, float rate)
{
    if (points.empty() || rate <= 0.0f || rate >= 1.0f) {
        return points;
    }

    std::vector<float> subsampled;
    QRandomGenerator rand(QDateTime::currentSecsSinceEpoch());

    // Process points in groups of 3 (x, y, z)
    for (size_t i = 0; i < points.size(); i += 3) {
        if (i + 2 < points.size()) {
            float randomValue = rand.generateDouble();
            if (randomValue < rate) {
                subsampled.push_back(points[i]);     // x
                subsampled.push_back(points[i + 1]); // y
                subsampled.push_back(points[i + 2]); // z
            }
        }
    }

    qDebug() << "Subsampled point cloud: Original" << (points.size() / 3)
             << "points, Subsampled" << (subsampled.size() / 3)
             << "points (rate:" << rate << ")";

    return subsampled;
}

void PointCloudLoadManager::generateLODForScan(const QString &scanId, float subsampleRate)
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it == m_scanStates.end() || !it.value()->data || !it.value()->data->isValid()) {
        qDebug() << "Cannot generate LOD for scan - not loaded:" << scanId;
        return;
    }

    emit lodGenerationStarted(scanId);

    auto &data = it.value()->data;
    data->lodPoints = subsamplePointCloud(data->points, subsampleRate);
    data->lodPointCount = data->lodPoints.size() / 3;
    data->lodSubsampleRate = subsampleRate;

    qDebug() << "Generated LOD for scan:" << scanId
             << "Original:" << data->pointCount
             << "LOD:" << data->lodPointCount
             << "Rate:" << subsampleRate;

    emit lodGenerationFinished(scanId, true);
}

bool PointCloudLoadManager::isLODActive(const QString &scanId) const
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end() && it.value()->data) {
        return it.value()->data->lodActive;
    }

    return false;
}

void PointCloudLoadManager::setLODActive(const QString &scanId, bool active)
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end() && it.value()->data) {
        it.value()->data->lodActive = active;
        emit lodStateChanged(scanId, active);
        qDebug() << "LOD state changed for scan:" << scanId << "Active:" << active;
    }
}

std::vector<float> PointCloudLoadManager::getLODPointCloudData(const QString &scanId)
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end() && it.value()->data && it.value()->data->isValid()) {
        // Update last accessed time
        it.value()->lastAccessed = QDateTime::currentDateTime();
        return it.value()->data->lodPoints;
    }

    return std::vector<float>();
}

// Sprint 3.4: Enhanced memory tracking
size_t PointCloudLoadManager::getScanMemoryUsage(const QString &scanId) const
{
    QMutexLocker locker(&m_stateMutex);

    auto it = m_scanStates.find(scanId);
    if (it != m_scanStates.end() && it.value()->data) {
        return it.value()->data->getTotalMemoryUsage();
    }

    return 0;
}

size_t PointCloudLoadManager::getClusterMemoryUsage(const QString &clusterId) const
{
    QStringList scanIds = getClusterScanIds(clusterId);
    size_t totalMemory = 0;

    for (const QString &scanId : scanIds) {
        totalMemory += getScanMemoryUsage(scanId);
    }

    return totalMemory;
}
