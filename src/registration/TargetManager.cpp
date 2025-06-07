#include "TargetManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <algorithm>
#include <cmath>

TargetManager::TargetManager(QObject* parent)
    : QObject(parent)
    , m_nextTargetId(1)
{
}

bool TargetManager::addTarget(const QString& scanId, std::shared_ptr<Target> target)
{
    if (!target || scanId.isEmpty()) {
        qWarning() << "TargetManager: Cannot add null target or empty scan ID";
        return false;
    }
    
    QString targetId = target->getTargetId();
    if (targetId.isEmpty()) {
        targetId = generateUniqueTargetId();
        // Note: We can't modify the target ID after creation in this design
        // In a real implementation, you might want to allow setting the ID
        qWarning() << "TargetManager: Target has empty ID, cannot add";
        return false;
    }
    
    if (m_targets.contains(targetId)) {
        qWarning() << "TargetManager: Target with ID" << targetId << "already exists";
        return false;
    }
    
    // Add target to main collection
    m_targets[targetId] = target;
    
    // Add target ID to scan's target list
    if (!m_scanTargets.contains(scanId)) {
        m_scanTargets[scanId] = QStringList();
    }
    m_scanTargets[scanId].append(targetId);
    
    qDebug() << "TargetManager: Added target" << targetId << "to scan" << scanId;
    
    emit targetAdded(scanId, targetId);
    
    return true;
}

bool TargetManager::removeTarget(const QString& targetId)
{
    if (!m_targets.contains(targetId)) {
        return false;
    }
    
    // Remove target from main collection
    m_targets.remove(targetId);
    
    // Remove target ID from all scan lists
    for (auto& targetList : m_scanTargets) {
        targetList.removeAll(targetId);
    }
    
    // Remove any correspondences involving this target
    m_correspondences.erase(
        std::remove_if(m_correspondences.begin(), m_correspondences.end(),
                      [&targetId](const TargetCorrespondence& corr) {
                          return corr.targetId1 == targetId || corr.targetId2 == targetId;
                      }),
        m_correspondences.end());
    
    qDebug() << "TargetManager: Removed target" << targetId;
    
    emit targetRemoved(targetId);
    
    return true;
}

std::shared_ptr<Target> TargetManager::getTarget(const QString& targetId) const
{
    auto it = m_targets.find(targetId);
    return (it != m_targets.end()) ? it.value() : nullptr;
}

QList<std::shared_ptr<Target>> TargetManager::getTargetsForScan(const QString& scanId) const
{
    QList<std::shared_ptr<Target>> targets;
    
    if (m_scanTargets.contains(scanId)) {
        const QStringList& targetIds = m_scanTargets[scanId];
        targets.reserve(targetIds.size());
        
        for (const QString& targetId : targetIds) {
            auto target = getTarget(targetId);
            if (target) {
                targets.append(target);
            }
        }
    }
    
    return targets;
}

QList<std::shared_ptr<Target>> TargetManager::getTargetsByType(const QString& targetType) const
{
    QList<std::shared_ptr<Target>> targets;
    
    for (const auto& target : m_targets) {
        if (target && target->getType() == targetType) {
            targets.append(target);
        }
    }
    
    return targets;
}

QList<std::shared_ptr<Target>> TargetManager::getAllTargets() const
{
    QList<std::shared_ptr<Target>> targets;
    targets.reserve(m_targets.size());
    
    for (const auto& target : m_targets) {
        if (target) {
            targets.append(target);
        }
    }
    
    return targets;
}

QStringList TargetManager::getScansWithTargets() const
{
    QStringList scans;
    
    for (auto it = m_scanTargets.begin(); it != m_scanTargets.end(); ++it) {
        if (!it.value().isEmpty()) {
            scans.append(it.key());
        }
    }
    
    return scans;
}

void TargetManager::clearTargetsForScan(const QString& scanId)
{
    if (!m_scanTargets.contains(scanId)) {
        return;
    }
    
    const QStringList& targetIds = m_scanTargets[scanId];
    
    // Remove each target
    for (const QString& targetId : targetIds) {
        removeTarget(targetId);
    }
    
    // Clear the scan's target list
    m_scanTargets[scanId].clear();
    
    qDebug() << "TargetManager: Cleared all targets for scan" << scanId;
}

void TargetManager::clearAllTargets()
{
    m_targets.clear();
    m_scanTargets.clear();
    m_correspondences.clear();
    
    qDebug() << "TargetManager: Cleared all targets and correspondences";
}

bool TargetManager::addCorrespondence(const TargetCorrespondence& correspondence)
{
    if (!validateCorrespondence(correspondence)) {
        qWarning() << "TargetManager: Invalid correspondence";
        return false;
    }
    
    // Check if correspondence already exists
    for (const auto& existing : m_correspondences) {
        if ((existing.targetId1 == correspondence.targetId1 && existing.targetId2 == correspondence.targetId2) ||
            (existing.targetId1 == correspondence.targetId2 && existing.targetId2 == correspondence.targetId1)) {
            qWarning() << "TargetManager: Correspondence already exists";
            return false;
        }
    }
    
    TargetCorrespondence newCorr = correspondence;
    updateCorrespondenceQuality(newCorr);
    
    m_correspondences.append(newCorr);
    
    qDebug() << "TargetManager: Added correspondence between" 
             << correspondence.targetId1 << "and" << correspondence.targetId2;
    
    emit correspondenceAdded(correspondence.targetId1, correspondence.targetId2);
    
    return true;
}

bool TargetManager::removeCorrespondence(const QString& targetId1, const QString& targetId2)
{
    auto it = std::find_if(m_correspondences.begin(), m_correspondences.end(),
                          [&](const TargetCorrespondence& corr) {
                              return (corr.targetId1 == targetId1 && corr.targetId2 == targetId2) ||
                                     (corr.targetId1 == targetId2 && corr.targetId2 == targetId1);
                          });
    
    if (it != m_correspondences.end()) {
        m_correspondences.erase(it);
        
        qDebug() << "TargetManager: Removed correspondence between" << targetId1 << "and" << targetId2;
        
        emit correspondenceRemoved(targetId1, targetId2);
        return true;
    }
    
    return false;
}

QList<TargetCorrespondence> TargetManager::getAllCorrespondences() const
{
    return m_correspondences;
}

QList<TargetCorrespondence> TargetManager::getCorrespondencesForTarget(const QString& targetId) const
{
    QList<TargetCorrespondence> correspondences;
    
    for (const auto& corr : m_correspondences) {
        if (corr.targetId1 == targetId || corr.targetId2 == targetId) {
            correspondences.append(corr);
        }
    }
    
    return correspondences;
}

QList<TargetCorrespondence> TargetManager::getCorrespondencesBetweenScans(
    const QString& scanId1, const QString& scanId2) const
{
    QList<TargetCorrespondence> correspondences;
    
    for (const auto& corr : m_correspondences) {
        if ((corr.scanId1 == scanId1 && corr.scanId2 == scanId2) ||
            (corr.scanId1 == scanId2 && corr.scanId2 == scanId1)) {
            correspondences.append(corr);
        }
    }
    
    return correspondences;
}

void TargetManager::clearAllCorrespondences()
{
    m_correspondences.clear();
    qDebug() << "TargetManager: Cleared all correspondences";
}

bool TargetManager::validateAllData() const
{
    // Validate all targets
    for (const auto& target : m_targets) {
        if (!target || !target->isValid()) {
            emit const_cast<TargetManager*>(this)->validationError(
                QString("Invalid target: %1").arg(target ? target->getTargetId() : "null"));
            return false;
        }
    }
    
    // Validate all correspondences
    for (const auto& corr : m_correspondences) {
        if (!validateCorrespondence(corr)) {
            emit const_cast<TargetManager*>(this)->validationError(
                QString("Invalid correspondence between %1 and %2").arg(corr.targetId1, corr.targetId2));
            return false;
        }
    }
    
    return true;
}

TargetManager::TargetStatistics TargetManager::getStatistics() const
{
    TargetStatistics stats;
    
    stats.totalTargets = m_targets.size();
    stats.correspondences = m_correspondences.size();
    
    float qualitySum = 0.0f;
    int validCount = 0;
    
    for (const auto& target : m_targets) {
        if (target) {
            if (target->isValid()) {
                stats.validTargets++;
                validCount++;
                qualitySum += target->getQuality();
            }
            
            QString type = target->getType();
            if (type == "Sphere") {
                stats.sphereTargets++;
            } else if (type == "Checkerboard") {
                stats.checkerboardTargets++;
            } else if (type == "Natural Point") {
                stats.naturalPointTargets++;
            }
        }
    }
    
    if (validCount > 0) {
        stats.averageQuality = qualitySum / validCount;
    }
    
    return stats;
}

void TargetManager::updateTargetQualities()
{
    for (const auto& target : m_targets) {
        if (target) {
            // Quality update logic would go here
            // For now, we'll just emit the update signal
            emit targetUpdated(target->getTargetId());
        }
    }

    TargetStatistics stats = getStatistics();
    emit statisticsUpdated(stats);
}

QList<TargetCorrespondence> TargetManager::findPotentialCorrespondences(
    const QString& scanId1, const QString& scanId2, float maxDistance) const
{
    QList<TargetCorrespondence> potentialCorrespondences;

    QList<std::shared_ptr<Target>> targets1 = getTargetsForScan(scanId1);
    QList<std::shared_ptr<Target>> targets2 = getTargetsForScan(scanId2);

    for (const auto& target1 : targets1) {
        for (const auto& target2 : targets2) {
            if (!target1 || !target2) continue;

            // Only match targets of the same type
            if (target1->getType() != target2->getType()) continue;

            // Check distance
            float distance = (target1->getPosition() - target2->getPosition()).length();
            if (distance <= maxDistance) {
                TargetCorrespondence corr(target1->getTargetId(), target2->getTargetId(),
                                        scanId1, scanId2);
                corr.distance = distance;
                corr.confidence = 1.0f - (distance / maxDistance);  // Higher confidence for closer targets

                potentialCorrespondences.append(corr);
            }
        }
    }

    // Sort by confidence (best first)
    std::sort(potentialCorrespondences.begin(), potentialCorrespondences.end(),
              [](const TargetCorrespondence& a, const TargetCorrespondence& b) {
                  return a.confidence > b.confidence;
              });

    return potentialCorrespondences;
}

QVariantMap TargetManager::serialize() const
{
    QVariantMap data;

    // Serialize targets
    QVariantMap targetsData;
    for (auto it = m_targets.begin(); it != m_targets.end(); ++it) {
        if (it.value()) {
            targetsData[it.key()] = it.value()->serialize();
        }
    }
    data["targets"] = targetsData;

    // Serialize scan-target mapping
    QVariantMap scanTargetsData;
    for (auto it = m_scanTargets.begin(); it != m_scanTargets.end(); ++it) {
        scanTargetsData[it.key()] = QVariantList(it.value().begin(), it.value().end());
    }
    data["scanTargets"] = scanTargetsData;

    // Serialize correspondences
    QVariantList correspondencesData;
    for (const auto& corr : m_correspondences) {
        QVariantMap corrData;
        corrData["targetId1"] = corr.targetId1;
        corrData["targetId2"] = corr.targetId2;
        corrData["scanId1"] = corr.scanId1;
        corrData["scanId2"] = corr.scanId2;
        corrData["confidence"] = corr.confidence;
        corrData["distance"] = corr.distance;
        correspondencesData.append(corrData);
    }
    data["correspondences"] = correspondencesData;

    // Add metadata
    data["version"] = "1.0";
    data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    data["nextTargetId"] = m_nextTargetId;

    return data;
}

bool TargetManager::deserialize(const QVariantMap& data)
{
    if (!data.contains("targets") || !data.contains("correspondences")) {
        qWarning() << "TargetManager: Invalid serialization data";
        return false;
    }

    // Clear existing data
    clearAllTargets();

    // Deserialize targets
    QVariantMap targetsData = data["targets"].toMap();
    for (auto it = targetsData.begin(); it != targetsData.end(); ++it) {
        QVariantMap targetData = it.value().toMap();
        QString targetType = targetData["type"].toString();

        std::shared_ptr<Target> target;

        if (targetType == "Sphere") {
            target = std::make_shared<SphereTarget>("", QVector3D(), 0.0f);
        } else if (targetType == "Checkerboard") {
            target = std::make_shared<CheckerboardTarget>("", QVector3D(), QList<QVector3D>());
        } else if (targetType == "Natural Point") {
            target = std::make_shared<NaturalPointTarget>("", QVector3D(), "");
        }

        if (target && target->deserialize(targetData)) {
            m_targets[it.key()] = target;
        }
    }

    // Deserialize scan-target mapping
    if (data.contains("scanTargets")) {
        QVariantMap scanTargetsData = data["scanTargets"].toMap();
        for (auto it = scanTargetsData.begin(); it != scanTargetsData.end(); ++it) {
            QVariantList targetList = it.value().toList();
            QStringList targetIds;
            for (const auto& targetId : targetList) {
                targetIds.append(targetId.toString());
            }
            m_scanTargets[it.key()] = targetIds;
        }
    }

    // Deserialize correspondences
    QVariantList correspondencesData = data["correspondences"].toList();
    for (const auto& corrVar : correspondencesData) {
        QVariantMap corrData = corrVar.toMap();
        TargetCorrespondence corr(
            corrData["targetId1"].toString(),
            corrData["targetId2"].toString(),
            corrData["scanId1"].toString(),
            corrData["scanId2"].toString()
        );
        corr.confidence = corrData["confidence"].toFloat();
        corr.distance = corrData["distance"].toFloat();

        m_correspondences.append(corr);
    }

    // Restore metadata
    if (data.contains("nextTargetId")) {
        m_nextTargetId = data["nextTargetId"].toInt();
    }

    qDebug() << "TargetManager: Deserialized" << m_targets.size() << "targets and"
             << m_correspondences.size() << "correspondences";

    return true;
}

bool TargetManager::saveToFile(const QString& filePath) const
{
    QVariantMap data = serialize();

    QJsonDocument doc = QJsonDocument::fromVariant(data);
    QByteArray jsonData = doc.toJson();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "TargetManager: Cannot open file for writing:" << filePath;
        return false;
    }

    qint64 bytesWritten = file.write(jsonData);
    file.close();

    if (bytesWritten != jsonData.size()) {
        qWarning() << "TargetManager: Error writing to file:" << filePath;
        return false;
    }

    qDebug() << "TargetManager: Saved data to" << filePath;
    return true;
}

bool TargetManager::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "TargetManager: Cannot open file for reading:" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "TargetManager: JSON parse error:" << error.errorString();
        return false;
    }

    QVariantMap data = doc.toVariant().toMap();
    bool success = deserialize(data);

    if (success) {
        qDebug() << "TargetManager: Loaded data from" << filePath;
    }

    return success;
}

QString TargetManager::generateUniqueTargetId() const
{
    return QString("target_%1_%2")
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(m_nextTargetId++);
}

bool TargetManager::validateCorrespondence(const TargetCorrespondence& correspondence) const
{
    // Check that both targets exist
    if (!m_targets.contains(correspondence.targetId1) ||
        !m_targets.contains(correspondence.targetId2)) {
        return false;
    }

    // Check that targets are from different scans
    if (correspondence.scanId1 == correspondence.scanId2) {
        return false;
    }

    // Check that targets are valid
    auto target1 = m_targets[correspondence.targetId1];
    auto target2 = m_targets[correspondence.targetId2];

    if (!target1 || !target2 || !target1->isValid() || !target2->isValid()) {
        return false;
    }

    // Check that targets are of the same type
    if (target1->getType() != target2->getType()) {
        return false;
    }

    return correspondence.isValid();
}

float TargetManager::calculateCorrespondenceDistance(const TargetCorrespondence& correspondence) const
{
    auto target1 = getTarget(correspondence.targetId1);
    auto target2 = getTarget(correspondence.targetId2);

    if (!target1 || !target2) {
        return std::numeric_limits<float>::max();
    }

    return (target1->getPosition() - target2->getPosition()).length();
}

void TargetManager::updateCorrespondenceQuality(TargetCorrespondence& correspondence) const
{
    auto target1 = getTarget(correspondence.targetId1);
    auto target2 = getTarget(correspondence.targetId2);

    if (!target1 || !target2) {
        correspondence.confidence = 0.0f;
        return;
    }

    // Calculate distance
    correspondence.distance = calculateCorrespondenceDistance(correspondence);

    // Calculate confidence based on target qualities and distance
    float avgQuality = (target1->getQuality() + target2->getQuality()) * 0.5f;
    float distanceFactor = std::exp(-correspondence.distance);  // Closer = better

    correspondence.confidence = avgQuality * distanceFactor;
    correspondence.confidence = std::min(1.0f, correspondence.confidence);
}

// TargetStatistics implementation
QVariantMap TargetManager::TargetStatistics::toVariantMap() const
{
    QVariantMap data;
    data["totalTargets"] = totalTargets;
    data["sphereTargets"] = sphereTargets;
    data["checkerboardTargets"] = checkerboardTargets;
    data["naturalPointTargets"] = naturalPointTargets;
    data["validTargets"] = validTargets;
    data["correspondences"] = correspondences;
    data["averageQuality"] = averageQuality;
    return data;
}
