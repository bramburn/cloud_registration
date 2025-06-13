#include "registration/TargetCorrespondence.h"
#include <QDebug>
#include <QtMath>

TargetCorrespondence::TargetCorrespondence()
    : confidence_(1.0f)
    , distance_(0.0f)
    , residualError_(0.0f)
    , isValid_(true)
    , isManual_(false)
{
}

TargetCorrespondence::TargetCorrespondence(const QString& targetId1, const QString& targetId2,
                                         const QString& scanId1, const QString& scanId2)
    : targetId1_(targetId1)
    , targetId2_(targetId2)
    , scanId1_(scanId1)
    , scanId2_(scanId2)
    , confidence_(1.0f)
    , distance_(0.0f)
    , residualError_(0.0f)
    , isValid_(true)
    , isManual_(false)
{
}

QVariantMap TargetCorrespondence::serialize() const
{
    QVariantMap data;
    data["targetId1"] = targetId1_;
    data["targetId2"] = targetId2_;
    data["scanId1"] = scanId1_;
    data["scanId2"] = scanId2_;
    data["confidence"] = confidence_;
    data["distance"] = distance_;
    data["residualError"] = residualError_;
    data["isValid"] = isValid_;
    data["isManual"] = isManual_;
    data["description"] = description_;
    return data;
}

bool TargetCorrespondence::deserialize(const QVariantMap& data)
{
    targetId1_ = data.value("targetId1").toString();
    targetId2_ = data.value("targetId2").toString();
    scanId1_ = data.value("scanId1").toString();
    scanId2_ = data.value("scanId2").toString();
    confidence_ = data.value("confidence", 1.0f).toFloat();
    distance_ = data.value("distance", 0.0f).toFloat();
    residualError_ = data.value("residualError", 0.0f).toFloat();
    isValid_ = data.value("isValid", true).toBool();
    isManual_ = data.value("isManual", false).toBool();
    description_ = data.value("description").toString();
    
    return validate();
}

bool TargetCorrespondence::validate() const
{
    // Check that all required IDs are present
    if (targetId1_.isEmpty() || targetId2_.isEmpty() || 
        scanId1_.isEmpty() || scanId2_.isEmpty()) {
        return false;
    }
    
    // Check that we're not linking a target to itself
    if (targetId1_ == targetId2_ && scanId1_ == scanId2_) {
        return false;
    }
    
    // Check that scans are different
    if (scanId1_ == scanId2_) {
        return false;
    }
    
    // Check confidence range
    if (confidence_ < 0.0f || confidence_ > 1.0f) {
        return false;
    }
    
    // Check that distance and error are non-negative
    if (distance_ < 0.0f || residualError_ < 0.0f) {
        return false;
    }
    
    // Check for NaN values
    if (qIsNaN(confidence_) || qIsNaN(distance_) || qIsNaN(residualError_)) {
        return false;
    }
    
    return true;
}

QString TargetCorrespondence::getValidationError() const
{
    if (targetId1_.isEmpty()) {
        return "Target ID 1 cannot be empty";
    }
    
    if (targetId2_.isEmpty()) {
        return "Target ID 2 cannot be empty";
    }
    
    if (scanId1_.isEmpty()) {
        return "Scan ID 1 cannot be empty";
    }
    
    if (scanId2_.isEmpty()) {
        return "Scan ID 2 cannot be empty";
    }
    
    if (targetId1_ == targetId2_ && scanId1_ == scanId2_) {
        return "Cannot create correspondence between a target and itself";
    }
    
    if (scanId1_ == scanId2_) {
        return "Cannot create correspondence between targets in the same scan";
    }
    
    if (confidence_ < 0.0f || confidence_ > 1.0f) {
        return "Confidence must be between 0.0 and 1.0";
    }
    
    if (distance_ < 0.0f) {
        return "Distance cannot be negative";
    }
    
    if (residualError_ < 0.0f) {
        return "Residual error cannot be negative";
    }
    
    if (qIsNaN(confidence_) || qIsNaN(distance_) || qIsNaN(residualError_)) {
        return "Numeric values cannot be NaN";
    }
    
    return QString();
}

QString TargetCorrespondence::getCorrespondenceId() const
{
    return generateCorrespondenceId(targetId1_, targetId2_);
}

bool TargetCorrespondence::matches(const QString& scanId1, const QString& scanId2) const
{
    return (scanId1_ == scanId1 && scanId2_ == scanId2) ||
           (scanId1_ == scanId2 && scanId2_ == scanId1);
}

bool TargetCorrespondence::containsTarget(const QString& targetId) const
{
    return targetId1_ == targetId || targetId2_ == targetId;
}

bool TargetCorrespondence::containsScan(const QString& scanId) const
{
    return scanId1_ == scanId || scanId2_ == scanId;
}

bool TargetCorrespondence::operator==(const TargetCorrespondence& other) const
{
    // Two correspondences are equal if they link the same targets
    // (order doesn't matter)
    return ((targetId1_ == other.targetId1_ && targetId2_ == other.targetId2_) ||
            (targetId1_ == other.targetId2_ && targetId2_ == other.targetId1_)) &&
           ((scanId1_ == other.scanId1_ && scanId2_ == other.scanId2_) ||
            (scanId1_ == other.scanId2_ && scanId2_ == other.scanId1_));
}

bool TargetCorrespondence::operator!=(const TargetCorrespondence& other) const
{
    return !(*this == other);
}

// Utility functions
QString generateCorrespondenceId(const QString& targetId1, const QString& targetId2)
{
    // Create a consistent ID regardless of order
    if (targetId1 < targetId2) {
        return QString("%1_%2").arg(targetId1, targetId2);
    } else {
        return QString("%1_%2").arg(targetId2, targetId1);
    }
}

bool areCorrespondencesCompatible(const TargetCorrespondence& c1, const TargetCorrespondence& c2)
{
    // Check if two correspondences can coexist in the same registration
    // They are incompatible if they try to link the same target to different targets
    
    // Check if c1's first target conflicts with c2
    if (c1.targetId1() == c2.targetId1() && c1.targetId2() != c2.targetId2()) {
        return false;
    }
    if (c1.targetId1() == c2.targetId2() && c1.targetId2() != c2.targetId1()) {
        return false;
    }
    
    // Check if c1's second target conflicts with c2
    if (c1.targetId2() == c2.targetId1() && c1.targetId1() != c2.targetId2()) {
        return false;
    }
    if (c1.targetId2() == c2.targetId2() && c1.targetId1() != c2.targetId1()) {
        return false;
    }
    
    return true;
}
