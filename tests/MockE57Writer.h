#ifndef MOCKE57WRITER_H
#define MOCKE57WRITER_H

#include "../src/IE57Writer.h"
#include <QStringList>
#include <vector>

/**
 * @brief MockE57Writer - Mock implementation of IE57Writer for testing
 * 
 * This mock implementation simulates E57 writing operations without actually
 * creating files on disk. It's used for testing components that depend on
 * IE57Writer interface without requiring actual file I/O.
 * 
 * Sprint 2 Decoupling: Enables testing of UI and application logic
 * independently of the actual E57 writing implementation.
 */
class MockE57Writer : public IE57Writer {
    Q_OBJECT

public:
    explicit MockE57Writer(QObject *parent = nullptr) : IE57Writer(parent) {
        reset();
    }

    // Reset mock state for clean testing
    void reset() {
        m_isFileOpen = false;
        m_currentFilePath.clear();
        m_lastError.clear();
        m_scanCount = 0;
        m_scans.clear();
        m_methodCalls.clear();
        m_shouldFailNextOperation = false;
    }

    // Test control methods
    void setShouldFailNextOperation(bool shouldFail) {
        m_shouldFailNextOperation = shouldFail;
    }

    QStringList getMethodCalls() const {
        return m_methodCalls;
    }

    int getTotalPointsWritten() const {
        int total = 0;
        for (const auto& scan : m_scans) {
            total += scan.points.size();
        }
        return total;
    }

    // IE57Writer interface implementation
    bool createFile(const QString& filePath) override {
        m_methodCalls.append(QString("createFile(%1)").arg(filePath));
        
        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated file creation failure";
            emit errorOccurred(m_lastError);
            return false;
        }

        m_currentFilePath = filePath;
        m_isFileOpen = true;
        m_scanCount = 0;
        m_scans.clear();
        m_lastError.clear();
        
        emit fileCreated(true, filePath);
        return true;
    }

    bool addScan(const QString& scanName = "Default Scan 001") override {
        m_methodCalls.append(QString("addScan(%1)").arg(scanName));
        
        if (!m_isFileOpen) {
            m_lastError = "Mock: No file is open";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated scan addition failure";
            emit errorOccurred(m_lastError);
            return false;
        }

        ScanMetadata metadata;
        metadata.name = scanName;
        m_scans.append({metadata, {}, ExportOptions()});
        m_scanCount++;
        
        emit scanAdded(true, scanName);
        return true;
    }

    bool addScan(const ScanMetadata& metadata) override {
        m_methodCalls.append(QString("addScan(metadata: %1)").arg(metadata.name));
        
        if (!m_isFileOpen) {
            m_lastError = "Mock: No file is open";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated scan addition failure";
            emit errorOccurred(m_lastError);
            return false;
        }

        m_scans.append({metadata, {}, ExportOptions()});
        m_scanCount++;
        
        emit scanAdded(true, metadata.name);
        return true;
    }

    bool definePointPrototype(const ExportOptions& options = ExportOptions()) override {
        m_methodCalls.append(QString("definePointPrototype(intensity=%1, color=%2)")
                           .arg(options.includeIntensity).arg(options.includeColor));
        
        if (!m_isFileOpen || m_scans.isEmpty()) {
            m_lastError = "Mock: No file open or no scans available";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated prototype definition failure";
            return false;
        }

        // Update the last scan's export options
        m_scans.last().options = options;
        return true;
    }

    bool defineXYZPrototype() override {
        return definePointPrototype(ExportOptions(false, false));
    }

    bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) override {
        m_methodCalls.append(QString("writePoints(%1 points, intensity=%2, color=%3)")
                           .arg(points.size()).arg(options.includeIntensity).arg(options.includeColor));
        
        if (!m_isFileOpen || m_scans.isEmpty()) {
            m_lastError = "Mock: No file open or no scans available";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated point writing failure";
            return false;
        }

        // Add points to the last scan
        m_scans.last().points.insert(m_scans.last().points.end(), points.begin(), points.end());
        return true;
    }

    bool writePoints(int scanIndex, const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) override {
        m_methodCalls.append(QString("writePoints(scan=%1, %2 points, intensity=%3, color=%4)")
                           .arg(scanIndex).arg(points.size()).arg(options.includeIntensity).arg(options.includeColor));
        
        if (!m_isFileOpen || scanIndex < 0 || scanIndex >= m_scans.size()) {
            m_lastError = "Mock: Invalid scan index or no file open";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated point writing failure";
            return false;
        }

        // Add points to the specified scan
        m_scans[scanIndex].points.insert(m_scans[scanIndex].points.end(), points.begin(), points.end());
        return true;
    }

    bool writePoints(const std::vector<Point3D>& points) override {
        return writePoints(points, ExportOptions(false, false));
    }

    bool writePoints(int scanIndex, const std::vector<Point3D>& points) override {
        return writePoints(scanIndex, points, ExportOptions(false, false));
    }

    bool closeFile() override {
        m_methodCalls.append("closeFile()");
        
        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated file close failure";
            return false;
        }

        m_isFileOpen = false;
        return true;
    }

    QString getLastError() const override {
        return m_lastError;
    }

    bool isFileOpen() const override {
        return m_isFileOpen;
    }

    QString getCurrentFilePath() const override {
        return m_currentFilePath;
    }

    int getScanCount() const override {
        return m_scanCount;
    }

    bool writeMultipleScans(const std::vector<ScanData>& scansData) override {
        m_methodCalls.append(QString("writeMultipleScans(%1 scans)").arg(scansData.size()));
        
        if (!m_isFileOpen) {
            m_lastError = "Mock: No file is open";
            return false;
        }

        if (m_shouldFailNextOperation) {
            m_shouldFailNextOperation = false;
            m_lastError = "Mock: Simulated multiple scans writing failure";
            return false;
        }

        for (const auto& scanData : scansData) {
            m_scans.append(scanData);
            m_scanCount++;
        }

        return true;
    }

private:
    bool m_isFileOpen;
    QString m_currentFilePath;
    QString m_lastError;
    int m_scanCount;
    QList<ScanData> m_scans;
    QStringList m_methodCalls;
    bool m_shouldFailNextOperation;
};

#endif // MOCKE57WRITER_H
