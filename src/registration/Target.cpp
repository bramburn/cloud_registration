#include "Target.h"

// Target base class implementation
Target::Target(const QString& id, const QVector3D& pos)
    : m_targetId(id), m_position(pos), m_quality(1.0f), m_isValid(true)
{
}

QVariantMap Target::serialize() const
{
    QVariantMap data;
    data["targetId"] = m_targetId;
    data["type"] = getType();
    data["position"] = QVariantList{m_position.x(), m_position.y(), m_position.z()};
    data["quality"] = m_quality;
    data["isValid"] = m_isValid;
    return data;
}

bool Target::deserialize(const QVariantMap& data)
{
    if (!data.contains("targetId") || !data.contains("position")) {
        return false;
    }
    
    m_targetId = data["targetId"].toString();
    
    QVariantList posList = data["position"].toList();
    if (posList.size() == 3) {
        m_position = QVector3D(posList[0].toFloat(), posList[1].toFloat(), posList[2].toFloat());
    } else {
        return false;
    }
    
    if (data.contains("quality")) m_quality = data["quality"].toFloat();
    if (data.contains("isValid")) m_isValid = data["isValid"].toBool();
    
    return true;
}

// SphereTarget implementation
SphereTarget::SphereTarget(const QString& id, const QVector3D& pos, float radius)
    : Target(id, pos), m_radius(radius), m_rmsError(0.0f), m_inlierCount(0)
{
}

QVariantMap SphereTarget::serialize() const
{
    QVariantMap data = Target::serialize();
    data["radius"] = m_radius;
    data["rmsError"] = m_rmsError;
    data["inlierCount"] = m_inlierCount;
    return data;
}

bool SphereTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data) || !data.contains("radius")) {
        return false;
    }
    
    m_radius = data["radius"].toFloat();
    if (data.contains("rmsError")) m_rmsError = data["rmsError"].toFloat();
    if (data.contains("inlierCount")) m_inlierCount = data["inlierCount"].toInt();
    
    return true;
}

// NaturalPointTarget implementation
NaturalPointTarget::NaturalPointTarget(const QString& id, const QVector3D& pos, const QString& description)
    : Target(id, pos), m_description(description), m_featureVector(0, 0, 0), m_confidence(1.0f)
{
}

QVariantMap NaturalPointTarget::serialize() const
{
    QVariantMap data = Target::serialize();
    data["description"] = m_description;
    data["featureVector"] = QVariantList{m_featureVector.x(), m_featureVector.y(), m_featureVector.z()};
    data["confidence"] = m_confidence;
    return data;
}

bool NaturalPointTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data)) return false;
    
    if (data.contains("description")) m_description = data["description"].toString();
    if (data.contains("confidence")) m_confidence = data["confidence"].toFloat();
    
    if (data.contains("featureVector")) {
        QVariantList featureList = data["featureVector"].toList();
        if (featureList.size() == 3) {
            m_featureVector = QVector3D(featureList[0].toFloat(), featureList[1].toFloat(), featureList[2].toFloat());
        }
    }
    
    return true;
}

// CheckerboardTarget implementation
CheckerboardTarget::CheckerboardTarget(const QString& id, const QVector3D& pos, const QList<QVector3D>& corners)
    : Target(id, pos), m_cornerPoints(corners), m_normal(0, 0, 1), m_patternWidth(0), m_patternHeight(0)
{
}

QVariantMap CheckerboardTarget::serialize() const
{
    QVariantMap data = Target::serialize();
    
    QVariantList cornersList;
    for (const auto& corner : m_cornerPoints) {
        cornersList.append(QVariantList{corner.x(), corner.y(), corner.z()});
    }
    data["cornerPoints"] = cornersList;
    data["normal"] = QVariantList{m_normal.x(), m_normal.y(), m_normal.z()};
    data["patternWidth"] = m_patternWidth;
    data["patternHeight"] = m_patternHeight;
    
    return data;
}

bool CheckerboardTarget::deserialize(const QVariantMap& data)
{
    if (!Target::deserialize(data) || !data.contains("cornerPoints")) return false;
    
    QVariantList cornersList = data["cornerPoints"].toList();
    m_cornerPoints.clear();
    for (const auto& cornerVar : cornersList) {
        QVariantList cornerCoords = cornerVar.toList();
        if (cornerCoords.size() == 3) {
            m_cornerPoints.append(QVector3D(cornerCoords[0].toFloat(), cornerCoords[1].toFloat(), cornerCoords[2].toFloat()));
        }
    }
    
    if (data.contains("normal")) {
        QVariantList normalList = data["normal"].toList();
        if (normalList.size() == 3) {
            m_normal = QVector3D(normalList[0].toFloat(), normalList[1].toFloat(), normalList[2].toFloat());
        }
    }
    
    if (data.contains("patternWidth")) m_patternWidth = data["patternWidth"].toInt();
    if (data.contains("patternHeight")) m_patternHeight = data["patternHeight"].toInt();
    
    return true;
}
