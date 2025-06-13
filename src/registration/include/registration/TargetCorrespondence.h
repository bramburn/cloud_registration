#ifndef TARGETCORRESPONDENCE_H
#define TARGETCORRESPONDENCE_H

#include <QString>
#include <QVariantMap>
#include <QMetaType>

/**
 * @brief Represents a correspondence between targets in different scans
 * 
 * This class links targets from different scans that represent the same
 * physical feature. These correspondences are used to compute transformations
 * for scan registration.
 * 
 * Sprint 2 Implementation: Target correspondence data model
 */
class TargetCorrespondence
{
public:
    TargetCorrespondence();
    TargetCorrespondence(const QString& targetId1, const QString& targetId2,
                        const QString& scanId1, const QString& scanId2);
    
    // Basic properties
    QString targetId1() const { return targetId1_; }
    void setTargetId1(const QString& id) { targetId1_ = id; }
    
    QString targetId2() const { return targetId2_; }
    void setTargetId2(const QString& id) { targetId2_ = id; }
    
    QString scanId1() const { return scanId1_; }
    void setScanId1(const QString& id) { scanId1_ = id; }
    
    QString scanId2() const { return scanId2_; }
    void setScanId2(const QString& id) { scanId2_ = id; }
    
    // Quality metrics
    float confidence() const { return confidence_; }
    void setConfidence(float confidence) { confidence_ = confidence; }
    
    float distance() const { return distance_; }
    void setDistance(float distance) { distance_ = distance; }
    
    float residualError() const { return residualError_; }
    void setResidualError(float error) { residualError_ = error; }
    
    // Status
    bool isValid() const { return isValid_; }
    void setValid(bool valid) { isValid_ = valid; }
    
    bool isManual() const { return isManual_; }
    void setManual(bool manual) { isManual_ = manual; }
    
    // Metadata
    QString description() const { return description_; }
    void setDescription(const QString& desc) { description_ = desc; }
    
    // Serialization
    QVariantMap serialize() const;
    bool deserialize(const QVariantMap& data);
    
    // Validation
    bool validate() const;
    QString getValidationError() const;
    
    // Utility
    QString getCorrespondenceId() const;
    bool matches(const QString& scanId1, const QString& scanId2) const;
    bool containsTarget(const QString& targetId) const;
    bool containsScan(const QString& scanId) const;
    
    // Operators
    bool operator==(const TargetCorrespondence& other) const;
    bool operator!=(const TargetCorrespondence& other) const;

private:
    QString targetId1_;
    QString targetId2_;
    QString scanId1_;
    QString scanId2_;
    float confidence_;
    float distance_;
    float residualError_;
    bool isValid_;
    bool isManual_;
    QString description_;
};

// Utility functions
QString generateCorrespondenceId(const QString& targetId1, const QString& targetId2);
bool areCorrespondencesCompatible(const TargetCorrespondence& c1, const TargetCorrespondence& c2);

// Register metatype for Qt's type system
Q_DECLARE_METATYPE(TargetCorrespondence)

#endif // TARGETCORRESPONDENCE_H
