#include "registration/Target.h"

#include <QDebug>
#include <QtMath>

// Target base class implementation
Target::Target(const QString& id, const QVector3D& pos)
    : targetId_(id), position_(pos), confidence_(1.0f), isValid_(true)
{
}

QVariantMap Target::serialize() const
{
    QVariantMap data = serializeBase();
    data["type"] = getType();
    return data;
}

bool Target::deserialize(const QVariantMap& data)
{
    return deserializeBase(data);
}

bool Target::validate() const
{
    if (targetId_.isEmpty())
    {
        return false;
    }

    if (!position_.isNull() && (qIsNaN(position_.x()) || qIsNaN(position_.y()) || qIsNaN(position_.z())))
    {
        return false;
    }

    if (confidence_ < 0.0f || confidence_ > 1.0f)
    {
        return false;
    }

    return true;
}

QString Target::getValidationError() const
{
    if (targetId_.isEmpty())
    {
        return "Target ID cannot be empty";
    }

    if (!position_.isNull() && (qIsNaN(position_.x()) || qIsNaN(position_.y()) || qIsNaN(position_.z())))
    {
        return "Target position contains invalid values";
    }

    if (confidence_ < 0.0f || confidence_ > 1.0f)
    {
        return "Confidence must be between 0.0 and 1.0";
    }

    return QString();
}

QVariantMap Target::serializeBase() const
{
    QVariantMap data;
    data["targetId"] = targetId_;
    data["position"] = QVariantList{position_.x(), position_.y(), position_.z()};
    data["confidence"] = confidence_;
    data["isValid"] = isValid_;
    data["description"] = description_;
    data["scanId"] = scanId_;
    return data;
}

bool Target::deserializeBase(const QVariantMap& data)
{
    targetId_ = data.value("targetId").toString();

    QVariantList posList = data.value("position").toList();
    if (posList.size() == 3)
    {
        position_ = QVector3D(posList[0].toFloat(), posList[1].toFloat(), posList[2].toFloat());
    }

    confidence_ = data.value("confidence", 1.0f).toFloat();
    isValid_ = data.value("isValid", true).toBool();
    description_ = data.value("description").toString();
    scanId_ = data.value("scanId").toString();

    return true;
}

// SphereTarget implementation
SphereTarget::SphereTarget(const QString& id, const QVector3D& pos, float radius)
    : Target(id, pos), radius_(radius), rmsError_(0.0f), inlierCount_(0), coverage_(0.0f)
{
}

QVariantMap SphereTarget::serialize() const
{
    QVariantMap data = Target::serialize();
    data["radius"] = radius_;
    data["rmsError"] = rmsError_;
    data["inlierCount"] = inlierCount_;
    data["coverage"] = coverage_;
    return data;
}

bool SphereTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data))
    {
        return false;
    }

    radius_ = data.value("radius", 0.0f).toFloat();
    rmsError_ = data.value("rmsError", 0.0f).toFloat();
    inlierCount_ = data.value("inlierCount", 0).toInt();
    coverage_ = data.value("coverage", 0.0f).toFloat();

    return true;
}

std::unique_ptr<Target> SphereTarget::clone() const
{
    auto cloned = std::make_unique<SphereTarget>(targetId_, position_, radius_);
    cloned->setConfidence(confidence_);
    cloned->setValid(isValid_);
    cloned->setDescription(description_);
    cloned->setScanId(scanId_);
    cloned->setRmsError(rmsError_);
    cloned->setInlierCount(inlierCount_);
    cloned->setCoverage(coverage_);
    return cloned;
}

bool SphereTarget::validate() const
{
    if (!Target::validate())
    {
        return false;
    }

    if (radius_ <= 0.0f)
    {
        return false;
    }

    if (rmsError_ < 0.0f)
    {
        return false;
    }

    if (inlierCount_ < 0)
    {
        return false;
    }

    if (coverage_ < 0.0f || coverage_ > 1.0f)
    {
        return false;
    }

    return true;
}

QString SphereTarget::getValidationError() const
{
    QString baseError = Target::getValidationError();
    if (!baseError.isEmpty())
    {
        return baseError;
    }

    if (radius_ <= 0.0f)
    {
        return "Sphere radius must be positive";
    }

    if (rmsError_ < 0.0f)
    {
        return "RMS error cannot be negative";
    }

    if (inlierCount_ < 0)
    {
        return "Inlier count cannot be negative";
    }

    if (coverage_ < 0.0f || coverage_ > 1.0f)
    {
        return "Coverage must be between 0.0 and 1.0";
    }

    return QString();
}

// CheckerboardTarget implementation
CheckerboardTarget::CheckerboardTarget(const QString& id, const QVector3D& pos, const QList<QVector3D>& corners)
    : Target(id, pos), cornerPoints_(corners), planeError_(0.0f)
{
    calculateDerivedProperties();
}

QVector3D CheckerboardTarget::centroid() const
{
    if (cornerPoints_.isEmpty())
    {
        return QVector3D();
    }

    return calculateCentroid(cornerPoints_);
}

float CheckerboardTarget::area() const
{
    if (cornerPoints_.size() < 3)
    {
        return 0.0f;
    }

    // Simple area calculation for quadrilateral (assuming 4 corners)
    if (cornerPoints_.size() == 4)
    {
        QVector3D v1 = cornerPoints_[1] - cornerPoints_[0];
        QVector3D v2 = cornerPoints_[3] - cornerPoints_[0];
        return 0.5f * QVector3D::crossProduct(v1, v2).length();
    }

    return 0.0f;
}

QVariantMap CheckerboardTarget::serialize() const
{
    QVariantMap data = Target::serialize();

    QVariantList corners;
    for (const auto& corner : cornerPoints_)
    {
        corners.append(QVariantList{corner.x(), corner.y(), corner.z()});
    }
    data["cornerPoints"] = corners;
    data["normal"] = QVariantList{normal_.x(), normal_.y(), normal_.z()};
    data["planeError"] = planeError_;

    return data;
}

bool CheckerboardTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data))
    {
        return false;
    }

    cornerPoints_.clear();
    QVariantList corners = data.value("cornerPoints").toList();
    for (const auto& cornerVar : corners)
    {
        QVariantList cornerList = cornerVar.toList();
        if (cornerList.size() == 3)
        {
            cornerPoints_.append(QVector3D(cornerList[0].toFloat(), cornerList[1].toFloat(), cornerList[2].toFloat()));
        }
    }

    QVariantList normalList = data.value("normal").toList();
    if (normalList.size() == 3)
    {
        normal_ = QVector3D(normalList[0].toFloat(), normalList[1].toFloat(), normalList[2].toFloat());
    }

    planeError_ = data.value("planeError", 0.0f).toFloat();

    return true;
}

std::unique_ptr<Target> CheckerboardTarget::clone() const
{
    auto cloned = std::make_unique<CheckerboardTarget>(targetId_, position_, cornerPoints_);
    cloned->setConfidence(confidence_);
    cloned->setValid(isValid_);
    cloned->setDescription(description_);
    cloned->setScanId(scanId_);
    cloned->setNormal(normal_);
    cloned->setPlaneError(planeError_);
    return cloned;
}

bool CheckerboardTarget::validate() const
{
    if (!Target::validate())
    {
        return false;
    }

    if (cornerPoints_.size() < 3)
    {
        return false;
    }

    if (planeError_ < 0.0f)
    {
        return false;
    }

    return true;
}

QString CheckerboardTarget::getValidationError() const
{
    QString baseError = Target::getValidationError();
    if (!baseError.isEmpty())
    {
        return baseError;
    }

    if (cornerPoints_.size() < 3)
    {
        return "Checkerboard must have at least 3 corner points";
    }

    if (planeError_ < 0.0f)
    {
        return "Plane error cannot be negative";
    }

    return QString();
}

void CheckerboardTarget::calculateDerivedProperties()
{
    if (cornerPoints_.size() >= 3)
    {
        // Calculate normal vector from first three points
        QVector3D v1 = cornerPoints_[1] - cornerPoints_[0];
        QVector3D v2 = cornerPoints_[2] - cornerPoints_[0];
        normal_ = QVector3D::crossProduct(v1, v2).normalized();

        // Update position to centroid if not set
        if (position_.isNull())
        {
            position_ = centroid();
        }
    }
}

// NaturalPointTarget implementation
NaturalPointTarget::NaturalPointTarget(const QString& id, const QVector3D& pos, const QString& desc)
    : Target(id, pos), curvature_(0.0f), distinctiveness_(0.0f), neighborCount_(0)
{
    setDescription(desc);
}

QVariantMap NaturalPointTarget::serialize() const
{
    QVariantMap data = Target::serialize();
    data["normal"] = QVariantList{normal_.x(), normal_.y(), normal_.z()};
    data["curvature"] = curvature_;
    data["distinctiveness"] = distinctiveness_;
    data["neighborCount"] = neighborCount_;

    QVariantList descriptor;
    for (float value : featureDescriptor_)
    {
        descriptor.append(value);
    }
    data["featureDescriptor"] = descriptor;

    return data;
}

bool NaturalPointTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data))
    {
        return false;
    }

    QVariantList normalList = data.value("normal").toList();
    if (normalList.size() == 3)
    {
        normal_ = QVector3D(normalList[0].toFloat(), normalList[1].toFloat(), normalList[2].toFloat());
    }

    curvature_ = data.value("curvature", 0.0f).toFloat();
    distinctiveness_ = data.value("distinctiveness", 0.0f).toFloat();
    neighborCount_ = data.value("neighborCount", 0).toInt();

    featureDescriptor_.clear();
    QVariantList descriptor = data.value("featureDescriptor").toList();
    for (const auto& value : descriptor)
    {
        featureDescriptor_.append(value.toFloat());
    }

    return true;
}

std::unique_ptr<Target> NaturalPointTarget::clone() const
{
    auto cloned = std::make_unique<NaturalPointTarget>(targetId_, position_, description_);
    cloned->setConfidence(confidence_);
    cloned->setValid(isValid_);
    cloned->setScanId(scanId_);
    cloned->setNormal(normal_);
    cloned->setCurvature(curvature_);
    cloned->setDistinctiveness(distinctiveness_);
    cloned->setNeighborCount(neighborCount_);
    cloned->setFeatureDescriptor(featureDescriptor_);
    return cloned;
}

bool NaturalPointTarget::validate() const
{
    if (!Target::validate())
    {
        return false;
    }

    if (distinctiveness_ < 0.0f || distinctiveness_ > 1.0f)
    {
        return false;
    }

    if (neighborCount_ < 0)
    {
        return false;
    }

    return true;
}

QString NaturalPointTarget::getValidationError() const
{
    QString baseError = Target::getValidationError();
    if (!baseError.isEmpty())
    {
        return baseError;
    }

    if (distinctiveness_ < 0.0f || distinctiveness_ > 1.0f)
    {
        return "Distinctiveness must be between 0.0 and 1.0";
    }

    if (neighborCount_ < 0)
    {
        return "Neighbor count cannot be negative";
    }

    return QString();
}

// Utility functions
std::unique_ptr<Target> createTargetFromData(const QVariantMap& data)
{
    QString type = data.value("type").toString();

    if (type == "Sphere")
    {
        auto target = std::make_unique<SphereTarget>("", QVector3D(), 0.0f);
        if (target->deserialize(data))
        {
            return target;
        }
    }
    else if (type == "Checkerboard")
    {
        auto target = std::make_unique<CheckerboardTarget>("", QVector3D(), QList<QVector3D>());
        if (target->deserialize(data))
        {
            return target;
        }
    }
    else if (type == "NaturalPoint")
    {
        auto target = std::make_unique<NaturalPointTarget>("", QVector3D());
        if (target->deserialize(data))
        {
            return target;
        }
    }

    return nullptr;
}

QString targetTypeToString(const Target* target)
{
    if (!target)
    {
        return "Unknown";
    }
    return target->getType();
}

QVector3D calculateCentroid(const QList<QVector3D>& points)
{
    if (points.isEmpty())
    {
        return QVector3D();
    }

    QVector3D sum;
    for (const auto& point : points)
    {
        sum += point;
    }

    return sum / static_cast<float>(points.size());
}

float calculateDistance(const QVector3D& p1, const QVector3D& p2)
{
    return (p2 - p1).length();
}
