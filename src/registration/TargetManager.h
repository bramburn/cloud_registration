#ifndef TARGETMANAGER_H
#define TARGETMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <memory>
#include "Target.h"
#include "TargetCorrespondence.h"

/**
 * @brief Manages targets and correspondences for registration
 * 
 * This class provides centralized management of registration targets and
 * their correspondences across multiple scans. It handles target storage,
 * correspondence creation, and quality assessment.
 * 
 * Sprint 2 Implementation: Target management system
 */
class TargetManager : public QObject
{
    Q_OBJECT

public:
    explicit TargetManager(QObject* parent = nullptr);
    virtual ~TargetManager() = default;

    // Target management
    void addTarget(const QString& scanId, std::unique_ptr<Target> target);
    void removeTarget(const QString& targetId);
    Target* getTarget(const QString& targetId) const;
    QList<Target*> getTargetsForScan(const QString& scanId) const;
    QList<Target*> getAllTargets() const;
    QStringList getTargetIds() const;
    
    // Target queries
    int getTargetCount() const;
    int getTargetCountForScan(const QString& scanId) const;
    QStringList getScanIds() const;
    bool hasTarget(const QString& targetId) const;
    bool hasScan(const QString& scanId) const;
    
    // Target filtering
    QList<Target*> getTargetsByType(const QString& type) const;
    QList<Target*> getValidTargets() const;
    QList<Target*> getTargetsWithinRadius(const QVector3D& center, float radius) const;
    
    // Correspondence management
    void addCorrespondence(const TargetCorrespondence& correspondence);
    void removeCorrespondence(const QString& targetId1, const QString& targetId2);
    void removeCorrespondencesForTarget(const QString& targetId);
    void removeCorrespondencesForScan(const QString& scanId);
    
    QList<TargetCorrespondence> getCorrespondences() const;
    QList<TargetCorrespondence> getCorrespondencesForScan(const QString& scanId) const;
    QList<TargetCorrespondence> getCorrespondencesBetweenScans(const QString& scanId1, const QString& scanId2) const;
    TargetCorrespondence* getCorrespondence(const QString& targetId1, const QString& targetId2);
    
    // Correspondence queries
    int getCorrespondenceCount() const;
    bool hasCorrespondence(const QString& targetId1, const QString& targetId2) const;
    QStringList getCorrespondingTargets(const QString& targetId) const;
    
    // Quality assessment
    float getAverageTargetConfidence() const;
    float getAverageCorrespondenceConfidence() const;
    int getValidTargetCount() const;
    int getValidCorrespondenceCount() const;
    
    // Data management
    void clear();
    void clearScan(const QString& scanId);
    void clearTargets();
    void clearCorrespondences();
    
    // Serialization
    QVariantMap serialize() const;
    bool deserialize(const QVariantMap& data);
    
    // Validation
    bool validate() const;
    QStringList getValidationErrors() const;

signals:
    void targetAdded(const QString& targetId, const QString& scanId);
    void targetRemoved(const QString& targetId);
    void targetUpdated(const QString& targetId);
    
    void correspondenceAdded(const QString& targetId1, const QString& targetId2);
    void correspondenceRemoved(const QString& targetId1, const QString& targetId2);
    void correspondenceUpdated(const QString& targetId1, const QString& targetId2);
    
    void dataChanged();

private:
    // Target storage
    QMap<QString, std::unique_ptr<Target>> targets_;
    QMap<QString, QStringList> scanTargets_; // scanId -> list of targetIds
    
    // Correspondence storage
    QList<TargetCorrespondence> correspondences_;
    
    // Helper methods
    QString generateTargetId() const;
    void updateScanTargetMapping(const QString& scanId, const QString& targetId);
    void removeScanTargetMapping(const QString& scanId, const QString& targetId);
    bool isCorrespondenceValid(const TargetCorrespondence& correspondence) const;
    void validateCorrespondenceConsistency();
    
    // Internal state
    int nextTargetId_;
};

#endif // TARGETMANAGER_H
