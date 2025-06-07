#ifndef MOCKPROJECTMANAGER_H
#define MOCKPROJECTMANAGER_H

#include <gmock/gmock.h>
#include <QObject>
#include <QString>
#include <QStringList>
#include "../../src/projectmanager.h"

/**
 * @brief MockProjectManager - Mock implementation of ProjectManager for testing
 * 
 * This mock class provides a test double for ProjectManager, allowing unit tests
 * to verify interactions with the project management system without requiring
 * actual database operations or file system access.
 * 
 * Sprint 4 Testing Requirements:
 * - Enables testing of MainPresenter sidebar integration
 * - Provides controllable responses for cluster operations
 * - Supports verification of method calls and parameters
 */
class MockProjectManager : public QObject {
    Q_OBJECT

public:
    explicit MockProjectManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MockProjectManager() = default;

    // Sprint 4: Cluster management methods
    MOCK_METHOD(QString, createCluster, (const QString& clusterName, const QString& parentClusterId), ());
    MOCK_METHOD(bool, deleteCluster, (const QString& clusterId), ());
    MOCK_METHOD(bool, renameCluster, (const QString& clusterId, const QString& newName), ());
    MOCK_METHOD(QStringList, getScansInCluster, (const QString& clusterId), ());
    MOCK_METHOD(bool, moveScanToCluster, (const QString& scanId, const QString& clusterId), ());
    
    // Sprint 4: Scan management methods
    MOCK_METHOD(bool, deleteScan, (const QString& scanId, bool deletePhysicalFile), ());
    
    // Sprint 4: Cluster locking methods
    MOCK_METHOD(bool, setClusterLockState, (const QString& clusterId, bool isLocked), ());
    MOCK_METHOD(bool, getClusterLockState, (const QString& clusterId), ());
    
    // Sprint 4: Enhanced deletion methods
    MOCK_METHOD(bool, deleteClusterRecursive, (const QString& clusterId, bool deletePhysicalFiles), ());

    // Helper methods for test setup
    void setupSuccessfulClusterCreation(const QString& expectedClusterId) {
        ON_CALL(*this, createCluster(testing::_, testing::_))
            .WillByDefault(testing::Return(expectedClusterId));
    }
    
    void setupFailedClusterCreation() {
        ON_CALL(*this, createCluster(testing::_, testing::_))
            .WillByDefault(testing::Return(QString()));
    }
    
    void setupSuccessfulClusterOperations() {
        ON_CALL(*this, deleteCluster(testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, renameCluster(testing::_, testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, setClusterLockState(testing::_, testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, deleteScan(testing::_, testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, deleteClusterRecursive(testing::_, testing::_))
            .WillByDefault(testing::Return(true));
    }
    
    void setupClusterWithScans(const QString& clusterId, const QStringList& scanIds) {
        ON_CALL(*this, getScansInCluster(clusterId))
            .WillByDefault(testing::Return(scanIds));
    }
    
    void setupClusterLockState(const QString& clusterId, bool isLocked) {
        ON_CALL(*this, getClusterLockState(clusterId))
            .WillByDefault(testing::Return(isLocked));
    }

signals:
    // Mock signals that match ProjectManager interface
    void clusterCreated(const ClusterInfo& cluster);
    void clusterDeleted(const QString& clusterId);
    void clusterRenamed(const QString& clusterId, const QString& newName);
    void scanMovedToCluster(const QString& scanId, const QString& clusterId);
    void clusterLockStateChanged(const QString& clusterId, bool isLocked);
    void scanDeleted(const QString& scanId);
    void clusterDeletedRecursive(const QString& clusterId);

public:
    // Helper methods to emit signals for testing
    void emitClusterCreated(const ClusterInfo& cluster) {
        emit clusterCreated(cluster);
    }
    
    void emitClusterDeleted(const QString& clusterId) {
        emit clusterDeleted(clusterId);
    }
    
    void emitClusterRenamed(const QString& clusterId, const QString& newName) {
        emit clusterRenamed(clusterId, newName);
    }
    
    void emitScanMovedToCluster(const QString& scanId, const QString& clusterId) {
        emit scanMovedToCluster(scanId, clusterId);
    }
    
    void emitClusterLockStateChanged(const QString& clusterId, bool isLocked) {
        emit clusterLockStateChanged(clusterId, isLocked);
    }
    
    void emitScanDeleted(const QString& scanId) {
        emit scanDeleted(scanId);
    }
    
    void emitClusterDeletedRecursive(const QString& clusterId) {
        emit clusterDeletedRecursive(clusterId);
    }
};

#endif // MOCKPROJECTMANAGER_H
