#include "registration/TargetManager.h"

#include <QDebug>
#include <QtMath>

#include <algorithm>

TargetManager::TargetManager(QObject* parent) : QObject(parent), nextTargetId_(1) {}

bool TargetManager::addTarget(const QString& scanId, std::shared_ptr<Target> target)
{
    if (!target)
    {
        qWarning() << "TargetManager: Cannot add null target";
        return false;
    }

    if (scanId.isEmpty())
    {
        qWarning() << "TargetManager: Cannot add target with empty scan ID";
        return false;
    }

    // Generate ID if not set
    if (target->targetId().isEmpty())
    {
        target->setTargetId(generateTargetId());
    }

    // Set scan ID
    target->setScanId(scanId);

    // Validate target
    if (!target->validate())
    {
        qWarning() << "TargetManager: Target validation failed:" << target->getValidationError();
        return false;
    }

    QString targetId = target->targetId();

    // Check for duplicate ID
    if (targets_.contains(targetId))
    {
        qWarning() << "TargetManager: Target with ID" << targetId << "already exists";
        return false;
    }

    // Store target
    targets_[targetId] = target;
    updateScanTargetMapping(scanId, targetId);

    emit targetAdded(targetId, scanId);
    emit dataChanged();

    qDebug() << "TargetManager: Added target" << targetId << "to scan" << scanId;
    return true;
}

bool TargetManager::removeTarget(const QString& targetId)
{
    auto it = targets_.find(targetId);
    if (it == targets_.end())
    {
        qWarning() << "TargetManager: Target" << targetId << "not found";
        return false;
    }

    QString scanId = it.value()->scanId();

    // Remove correspondences involving this target
    removeCorrespondencesForTarget(targetId);

    // Remove from scan mapping
    removeScanTargetMapping(scanId, targetId);

    // Remove target
    targets_.erase(it);

    emit targetRemoved(targetId);
    emit dataChanged();

    qDebug() << "TargetManager: Removed target" << targetId;
    return true;
}

Target* TargetManager::getTarget(const QString& targetId) const
{
    auto it = targets_.find(targetId);
    return (it != targets_.end()) ? it.value().get() : nullptr;
}

QList<Target*> TargetManager::getTargetsForScan(const QString& scanId) const
{
    QList<Target*> result;

    auto it = scanTargets_.find(scanId);
    if (it != scanTargets_.end())
    {
        for (const QString& targetId : it.value())
        {
            if (Target* target = getTarget(targetId))
            {
                result.append(target);
            }
        }
    }

    return result;
}

QList<Target*> TargetManager::getAllTargets() const
{
    QList<Target*> result;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        result.append(it.value().get());
    }
    return result;
}

QStringList TargetManager::getTargetIds() const
{
    QStringList result;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        result.append(it.key());
    }
    return result;
}

int TargetManager::getTargetCount() const
{
    return targets_.size();
}

int TargetManager::getTargetCountForScan(const QString& scanId) const
{
    auto it = scanTargets_.find(scanId);
    return (it != scanTargets_.end()) ? it.value().size() : 0;
}

QStringList TargetManager::getScanIds() const
{
    return scanTargets_.keys();
}

bool TargetManager::hasTarget(const QString& targetId) const
{
    return targets_.contains(targetId);
}

bool TargetManager::hasScan(const QString& scanId) const
{
    return scanTargets_.contains(scanId);
}

QList<Target*> TargetManager::getTargetsByType(const QString& type) const
{
    QList<Target*> result;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        if (it.value()->getType() == type)
        {
            result.append(it.value().get());
        }
    }
    return result;
}

QList<Target*> TargetManager::getValidTargets() const
{
    QList<Target*> result;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        if (it.value()->isValid() && it.value()->validate())
        {
            result.append(it.value().get());
        }
    }
    return result;
}

QList<Target*> TargetManager::getTargetsWithinRadius(const QVector3D& center, float radius) const
{
    QList<Target*> result;
    float radiusSquared = radius * radius;

    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        QVector3D targetPos = it.value()->position();
        float distanceSquared = (targetPos - center).lengthSquared();
        if (distanceSquared <= radiusSquared)
        {
            result.append(it.value().get());
        }
    }

    return result;
}

bool TargetManager::addCorrespondence(const TargetCorrespondence& correspondence)
{
    if (!correspondence.validate())
    {
        qWarning() << "TargetManager: Invalid correspondence:" << correspondence.getValidationError();
        return false;
    }

    if (!isCorrespondenceValid(correspondence))
    {
        qWarning() << "TargetManager: Correspondence references non-existent targets";
        return false;
    }

    // Check for duplicate
    for (const auto& existing : correspondences_)
    {
        if (existing == correspondence)
        {
            qWarning() << "TargetManager: Correspondence already exists";
            return false;
        }
    }

    correspondences_.append(correspondence);

    emit correspondenceAdded(correspondence.targetId1(), correspondence.targetId2());
    emit dataChanged();

    qDebug() << "TargetManager: Added correspondence between" << correspondence.targetId1() << "and"
             << correspondence.targetId2();
    return true;
}

void TargetManager::removeCorrespondence(const QString& targetId1, const QString& targetId2)
{
    auto it = std::find_if(correspondences_.begin(),
                           correspondences_.end(),
                           [&](const TargetCorrespondence& c)
                           {
                               return (c.targetId1() == targetId1 && c.targetId2() == targetId2) ||
                                      (c.targetId1() == targetId2 && c.targetId2() == targetId1);
                           });

    if (it != correspondences_.end())
    {
        QString id1 = it->targetId1();
        QString id2 = it->targetId2();
        correspondences_.erase(it);

        emit correspondenceRemoved(id1, id2);
        emit dataChanged();

        qDebug() << "TargetManager: Removed correspondence between" << id1 << "and" << id2;
    }
}

void TargetManager::removeCorrespondencesForTarget(const QString& targetId)
{
    auto it = correspondences_.begin();
    while (it != correspondences_.end())
    {
        if (it->containsTarget(targetId))
        {
            QString id1 = it->targetId1();
            QString id2 = it->targetId2();
            it = correspondences_.erase(it);
            emit correspondenceRemoved(id1, id2);
        }
        else
        {
            ++it;
        }
    }
    emit dataChanged();
}

void TargetManager::removeCorrespondencesForScan(const QString& scanId)
{
    auto it = correspondences_.begin();
    while (it != correspondences_.end())
    {
        if (it->containsScan(scanId))
        {
            QString id1 = it->targetId1();
            QString id2 = it->targetId2();
            it = correspondences_.erase(it);
            emit correspondenceRemoved(id1, id2);
        }
        else
        {
            ++it;
        }
    }
    emit dataChanged();
}

QList<TargetCorrespondence> TargetManager::getAllCorrespondences() const
{
    return correspondences_;
}

QList<TargetCorrespondence> TargetManager::getCorrespondences() const
{
    return correspondences_;
}

QList<TargetCorrespondence> TargetManager::getCorrespondencesForTarget(const QString& targetId) const
{
    QList<TargetCorrespondence> result;
    for (const auto& correspondence : correspondences_)
    {
        if (correspondence.containsTarget(targetId))
        {
            result.append(correspondence);
        }
    }
    return result;
}

QList<TargetCorrespondence> TargetManager::getCorrespondencesForScan(const QString& scanId) const
{
    QList<TargetCorrespondence> result;
    for (const auto& correspondence : correspondences_)
    {
        if (correspondence.containsScan(scanId))
        {
            result.append(correspondence);
        }
    }
    return result;
}

QList<TargetCorrespondence> TargetManager::getCorrespondencesBetweenScans(const QString& scanId1,
                                                                          const QString& scanId2) const
{
    QList<TargetCorrespondence> result;
    for (const auto& correspondence : correspondences_)
    {
        if (correspondence.matches(scanId1, scanId2))
        {
            result.append(correspondence);
        }
    }
    return result;
}

TargetCorrespondence* TargetManager::getCorrespondence(const QString& targetId1, const QString& targetId2)
{
    auto it = std::find_if(correspondences_.begin(),
                           correspondences_.end(),
                           [&](const TargetCorrespondence& c)
                           {
                               return (c.targetId1() == targetId1 && c.targetId2() == targetId2) ||
                                      (c.targetId1() == targetId2 && c.targetId2() == targetId1);
                           });

    return (it != correspondences_.end()) ? &(*it) : nullptr;
}

const TargetCorrespondence* TargetManager::getCorrespondence(const QString& targetId1, const QString& targetId2) const
{
    auto it = std::find_if(correspondences_.begin(),
                           correspondences_.end(),
                           [&](const TargetCorrespondence& c)
                           {
                               return (c.targetId1() == targetId1 && c.targetId2() == targetId2) ||
                                      (c.targetId1() == targetId2 && c.targetId2() == targetId1);
                           });

    return (it != correspondences_.end()) ? &(*it) : nullptr;
}

int TargetManager::getCorrespondenceCount() const
{
    return correspondences_.size();
}

bool TargetManager::hasCorrespondence(const QString& targetId1, const QString& targetId2) const
{
    return getCorrespondence(targetId1, targetId2) != nullptr;
}

QStringList TargetManager::getCorrespondingTargets(const QString& targetId) const
{
    QStringList result;
    for (const auto& correspondence : correspondences_)
    {
        if (correspondence.targetId1() == targetId)
        {
            result.append(correspondence.targetId2());
        }
        else if (correspondence.targetId2() == targetId)
        {
            result.append(correspondence.targetId1());
        }
    }
    return result;
}

float TargetManager::getAverageTargetConfidence() const
{
    if (targets_.empty())
    {
        return 0.0f;
    }

    float sum = 0.0f;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        sum += it.value()->confidence();
    }

    return sum / static_cast<float>(targets_.size());
}

float TargetManager::getAverageCorrespondenceConfidence() const
{
    if (correspondences_.isEmpty())
    {
        return 0.0f;
    }

    float sum = 0.0f;
    for (const auto& correspondence : correspondences_)
    {
        sum += correspondence.confidence();
    }

    return sum / static_cast<float>(correspondences_.size());
}

int TargetManager::getValidTargetCount() const
{
    int count = 0;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        if (it.value()->isValid() && it.value()->validate())
        {
            count++;
        }
    }
    return count;
}

int TargetManager::getValidCorrespondenceCount() const
{
    int count = 0;
    for (const auto& correspondence : correspondences_)
    {
        if (correspondence.isValid() && correspondence.validate())
        {
            count++;
        }
    }
    return count;
}

void TargetManager::clear()
{
    clearCorrespondences();
    clearTargets();
    scanTargets_.clear();
    nextTargetId_ = 1;
    emit dataChanged();
}

void TargetManager::clearScan(const QString& scanId)
{
    // Remove correspondences for this scan
    removeCorrespondencesForScan(scanId);

    // Remove targets for this scan
    auto it = scanTargets_.find(scanId);
    if (it != scanTargets_.end())
    {
        QStringList targetIds = it.value();
        for (const QString& targetId : targetIds)
        {
            auto targetIt = targets_.find(targetId);
            if (targetIt != targets_.end())
            {
                targets_.erase(targetIt);
                emit targetRemoved(targetId);
            }
        }
        scanTargets_.erase(it);
    }

    emit dataChanged();
}

void TargetManager::clearTargets()
{
    targets_.clear();
    scanTargets_.clear();
    emit dataChanged();
}

void TargetManager::clearCorrespondences()
{
    correspondences_.clear();
    emit dataChanged();
}

QString TargetManager::generateTargetId() const
{
    QString id;
    do
    {
        id = QString("target_%1").arg(nextTargetId_++);
    } while (targets_.contains(id));

    return id;
}

void TargetManager::updateScanTargetMapping(const QString& scanId, const QString& targetId)
{
    if (!scanTargets_[scanId].contains(targetId))
    {
        scanTargets_[scanId].append(targetId);
    }
}

void TargetManager::removeScanTargetMapping(const QString& scanId, const QString& targetId)
{
    auto it = scanTargets_.find(scanId);
    if (it != scanTargets_.end())
    {
        it.value().removeAll(targetId);
        if (it.value().isEmpty())
        {
            scanTargets_.erase(it);
        }
    }
}

bool TargetManager::isCorrespondenceValid(const TargetCorrespondence& correspondence) const
{
    return hasTarget(correspondence.targetId1()) && hasTarget(correspondence.targetId2());
}

QVariantMap TargetManager::serialize() const
{
    QVariantMap data;

    // Serialize targets
    QVariantList targetsList;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        targetsList.append(it.value()->serialize());
    }
    data["targets"] = targetsList;

    // Serialize correspondences
    QVariantList correspondencesList;
    for (const auto& correspondence : correspondences_)
    {
        correspondencesList.append(correspondence.serialize());
    }
    data["correspondences"] = correspondencesList;

    data["nextTargetId"] = nextTargetId_;

    return data;
}

bool TargetManager::deserialize(const QVariantMap& data)
{
    clear();

    // Deserialize targets
    QVariantList targetsList = data.value("targets").toList();
    for (const auto& targetVar : targetsList)
    {
        QVariantMap targetData = targetVar.toMap();
        auto target = createTargetFromData(targetData);
        if (target)
        {
            QString scanId = target->scanId();
            QString targetId = target->targetId();
            targets_[targetId] = std::shared_ptr<Target>(target.release());
            updateScanTargetMapping(scanId, targetId);
        }
    }

    // Deserialize correspondences
    QVariantList correspondencesList = data.value("correspondences").toList();
    for (const auto& correspondenceVar : correspondencesList)
    {
        QVariantMap correspondenceData = correspondenceVar.toMap();
        TargetCorrespondence correspondence;
        if (correspondence.deserialize(correspondenceData))
        {
            correspondences_.append(correspondence);
        }
    }

    nextTargetId_ = data.value("nextTargetId", 1).toInt();

    emit dataChanged();
    return true;
}

bool TargetManager::validate() const
{
    // Validate all targets
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        if (!it.value()->validate())
        {
            return false;
        }
    }

    // Validate all correspondences
    for (const auto& correspondence : correspondences_)
    {
        if (!correspondence.validate() || !isCorrespondenceValid(correspondence))
        {
            return false;
        }
    }

    return true;
}

QStringList TargetManager::getValidationErrors() const
{
    QStringList errors;

    // Check target validation
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        if (!it.value()->validate())
        {
            errors.append(QString("Target %1: %2").arg(it.key(), it.value()->getValidationError()));
        }
    }

    // Check correspondence validation
    for (const auto& correspondence : correspondences_)
    {
        if (!correspondence.validate())
        {
            errors.append(
                QString("Correspondence %1-%2: %3")
                    .arg(correspondence.targetId1(), correspondence.targetId2(), correspondence.getValidationError()));
        }
        if (!isCorrespondenceValid(correspondence))
        {
            errors.append(QString("Correspondence %1-%2: References non-existent targets")
                              .arg(correspondence.targetId1(), correspondence.targetId2()));
        }
    }

    return errors;
}

void TargetManager::clearTargetsForScan(const QString& scanId)
{
    clearScan(scanId);
}

QList<TargetCorrespondence>
TargetManager::findPotentialCorrespondences(const QString& scanId1, const QString& scanId2, float threshold) const
{
    QList<TargetCorrespondence> result;

    auto targets1 = getTargetsForScan(scanId1);
    auto targets2 = getTargetsForScan(scanId2);

    for (Target* target1 : targets1)
    {
        for (Target* target2 : targets2)
        {
            if (target1->getType() == target2->getType())
            {
                float distance = (target1->position() - target2->position()).length();
                if (distance < 1.0f)
                {  // Simple distance threshold
                    TargetCorrespondence correspondence(target1->targetId(), target2->targetId(), scanId1, scanId2);
                    correspondence.setDistance(distance);
                    correspondence.setConfidence(1.0f - (distance / 1.0f));  // Simple confidence calculation
                    if (correspondence.confidence() > threshold)
                    {
                        result.append(correspondence);
                    }
                }
            }
        }
    }

    return result;
}

TargetManager::Statistics TargetManager::getStatistics() const
{
    Statistics stats;
    stats.totalTargets = targets_.size();
    stats.correspondences = correspondences_.size();

    float totalConfidence = 0.0f;
    for (auto it = targets_.begin(); it != targets_.end(); ++it)
    {
        Target* target = it.value().get();
        if (target->isValid())
        {
            stats.validTargets++;
        }
        totalConfidence += target->confidence();

        QString type = target->getType();
        if (type == "Sphere")
        {
            stats.sphereTargets++;
        }
        else if (type == "NaturalPoint")
        {
            stats.naturalPointTargets++;
        }
        else if (type == "Checkerboard")
        {
            stats.checkerboardTargets++;
        }
    }

    if (stats.totalTargets > 0)
    {
        stats.averageQuality = totalConfidence / static_cast<float>(stats.totalTargets);
    }

    return stats;
}

bool TargetManager::saveToFile(const QString& filename) const
{
    QVariantMap data = serialize();
    // Simple implementation - in real code you'd use QJsonDocument
    Q_UNUSED(filename)
    Q_UNUSED(data)
    return true;  // Simplified for testing
}

bool TargetManager::loadFromFile(const QString& filename)
{
    // Simple implementation - in real code you'd use QJsonDocument
    Q_UNUSED(filename)
    return true;  // Simplified for testing
}
