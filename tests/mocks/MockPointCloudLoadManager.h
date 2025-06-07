#ifndef MOCKPOINTCLOUDLOADMANAGER_H
#define MOCKPOINTCLOUDLOADMANAGER_H

#include <gmock/gmock.h>
#include <QObject>
#include <QString>
#include <QStringList>
#include "../../src/pointcloudloadmanager.h"

/**
 * @brief MockPointCloudLoadManager - Mock implementation of PointCloudLoadManager for testing
 * 
 * This mock class provides a test double for PointCloudLoadManager, allowing unit tests
 * to verify interactions with the point cloud loading system without requiring
 * actual file I/O operations or memory management.
 * 
 * Sprint 4 Testing Requirements:
 * - Enables testing of MainPresenter sidebar integration
 * - Provides controllable responses for scan loading operations
 * - Supports verification of method calls and parameters
 */
class MockPointCloudLoadManager : public QObject {
    Q_OBJECT

public:
    explicit MockPointCloudLoadManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~MockPointCloudLoadManager() = default;

    // Sprint 4: Scan loading methods
    MOCK_METHOD(bool, loadScan, (const QString& scanId), ());
    MOCK_METHOD(bool, unloadScan, (const QString& scanId), ());
    MOCK_METHOD(bool, isScanLoaded, (const QString& scanId), (const));
    
    // Basic loading methods
    MOCK_METHOD(void, loadPointCloud, (const QString& filePath), ());
    MOCK_METHOD(void, cancelLoading, (), ());
    MOCK_METHOD(bool, isLoading, (), (const));

    // Helper methods for test setup
    void setupSuccessfulLoading() {
        ON_CALL(*this, loadScan(testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, unloadScan(testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*this, isLoading())
            .WillByDefault(testing::Return(false));
    }
    
    void setupFailedLoading() {
        ON_CALL(*this, loadScan(testing::_))
            .WillByDefault(testing::Return(false));
        ON_CALL(*this, unloadScan(testing::_))
            .WillByDefault(testing::Return(false));
    }
    
    void setupScanLoadedState(const QString& scanId, bool isLoaded) {
        ON_CALL(*this, isScanLoaded(scanId))
            .WillByDefault(testing::Return(isLoaded));
    }
    
    void setupLoadingState(bool isLoading) {
        ON_CALL(*this, isLoading())
            .WillByDefault(testing::Return(isLoading));
    }

signals:
    // Mock signals that match PointCloudLoadManager interface
    void loadingStarted(const QString& filePath);
    void loadingProgress(int percentage, const QString& stage);
    void loadingFinished(bool success, const QString& message, const std::vector<float>& points);
    void loadingCancelled();

public:
    // Helper methods to emit signals for testing
    void emitLoadingStarted(const QString& filePath) {
        emit loadingStarted(filePath);
    }
    
    void emitLoadingProgress(int percentage, const QString& stage) {
        emit loadingProgress(percentage, stage);
    }
    
    void emitLoadingFinished(bool success, const QString& message, const std::vector<float>& points) {
        emit loadingFinished(success, message, points);
    }
    
    void emitLoadingCancelled() {
        emit loadingCancelled();
    }
};

#endif // MOCKPOINTCLOUDLOADMANAGER_H
