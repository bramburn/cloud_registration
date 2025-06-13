#ifndef REGISTRATIONPROJECT_H
#define REGISTRATIONPROJECT_H

#include <QList>
#include <QMap>
#include <QMatrix4x4>

#include <memory>

#include "Target.h"
#include "TargetCorrespondence.h"
#include "TargetManager.h"
#include "core/project.h"

/**
 * @brief Information about a scan in the registration project
 */
struct ScanInfo
{
    QString scanId;
    QString filePath;
    QString name;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    int pointCount;
    QMatrix4x4 transform;  // Current transformation matrix
    bool isReference;      // True if this is the reference scan
    QString description;

    ScanInfo() : pointCount(0), isReference(false)
    {
        transform.setToIdentity();
    }

    QVariantMap serialize() const;
    bool deserialize(const QVariantMap& data);
};

/**
 * @brief Registration-specific project extension
 *
 * This class extends the base Project class with registration-specific
 * functionality including scan management, target storage, and registration
 * results tracking.
 *
 * Sprint 2 Implementation: Registration project management
 */
class RegistrationProject : public Project
{
    Q_OBJECT

public:
    explicit RegistrationProject(QObject* parent = nullptr);
    explicit RegistrationProject(const QString& name, const QString& path, QObject* parent = nullptr);
    virtual ~RegistrationProject() = default;

    // Scan management
    void addScan(const ScanInfo& scanInfo);
    void removeScan(const QString& scanId);
    void updateScan(const ScanInfo& scanInfo);
    ScanInfo getScan(const QString& scanId) const;
    QList<ScanInfo> getAllScans() const;
    QStringList getScanIds() const;

    // Scan queries
    int getScanCount() const;
    bool hasScan(const QString& scanId) const;
    ScanInfo getReferenceScan() const;
    void setReferenceScan(const QString& scanId);

    // Scan transformations
    void setScanTransform(const QString& scanId, const QMatrix4x4& transform);
    QMatrix4x4 getScanTransform(const QString& scanId) const;
    void resetScanTransforms();

    // Target management (delegates to TargetManager)
    TargetManager* targetManager() const
    {
        return targetManager_.get();
    }

    // Registration results
    struct RegistrationResult
    {
        QString sourceScanId;
        QString targetScanId;
        QMatrix4x4 transformation;
        float rmsError;
        int correspondenceCount;
        bool isValid;
        QString algorithm;  // "Manual", "ICP", etc.
        QDateTime timestamp;

        RegistrationResult() : rmsError(0.0f), correspondenceCount(0), isValid(false)
        {
            transformation.setToIdentity();
            timestamp = QDateTime::currentDateTime();
        }

        QVariantMap serialize() const;
        bool deserialize(const QVariantMap& data);
    };

    void addRegistrationResult(const RegistrationResult& result);
    void removeRegistrationResult(const QString& sourceScanId, const QString& targetScanId);
    QList<RegistrationResult> getRegistrationResults() const;
    RegistrationResult getRegistrationResult(const QString& sourceScanId, const QString& targetScanId) const;
    bool hasRegistrationResult(const QString& sourceScanId, const QString& targetScanId) const;

    // Sprint 6.1: Get latest registration result for deviation analysis
    RegistrationResult getLatestRegistrationResult() const;

    // Project state
    enum RegistrationState
    {
        NotStarted,
        ScanSelection,
        TargetDetection,
        ManualAlignment,
        ICPRegistration,
        QualityReview,
        Completed
    };

    RegistrationState getRegistrationState() const
    {
        return registrationState_;
    }
    void setRegistrationState(RegistrationState state);

    // Quality metrics
    float getOverallRegistrationQuality() const;
    int getTotalCorrespondenceCount() const;
    int getValidCorrespondenceCount() const;

    // Serialization (extends base Project)
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;

    // Validation
    bool validate() const override;
    QStringList getValidationErrors() const;

signals:
    void scanAdded(const QString& scanId);
    void scanRemoved(const QString& scanId);
    void scanUpdated(const QString& scanId);
    void referenceScanChanged(const QString& scanId);
    void registrationResultAdded(const QString& sourceScanId, const QString& targetScanId);
    void registrationStateChanged(RegistrationState state);

private:
    // Scan storage
    QMap<QString, ScanInfo> scans_;
    QString referenceScanId_;

    // Target management
    std::unique_ptr<TargetManager> targetManager_;

    // Registration results
    QList<RegistrationResult> registrationResults_;

    // Project state
    RegistrationState registrationState_;

    // Helper methods
    void initializeRegistrationProject();
    void connectTargetManagerSignals();
    QString generateScanId() const;
    void updateProjectMetadata();
};

// Utility functions
QString registrationStateToString(RegistrationProject::RegistrationState state);
RegistrationProject::RegistrationState stringToRegistrationState(const QString& stateString);

// Register metatypes
Q_DECLARE_METATYPE(ScanInfo)
Q_DECLARE_METATYPE(RegistrationProject::RegistrationResult)
Q_DECLARE_METATYPE(RegistrationProject::RegistrationState)

#endif  // REGISTRATIONPROJECT_H
