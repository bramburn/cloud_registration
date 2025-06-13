#include "registration/RegistrationProject.h"

#include <QDateTime>
#include <QDebug>
#include <QtMath>

// ScanInfo implementation
QVariantMap ScanInfo::serialize() const
{
    QVariantMap data;
    data["scanId"] = scanId;
    data["filePath"] = filePath;
    data["name"] = name;
    data["boundingBoxMin"] = QVariantList{boundingBoxMin.x(), boundingBoxMin.y(), boundingBoxMin.z()};
    data["boundingBoxMax"] = QVariantList{boundingBoxMax.x(), boundingBoxMax.y(), boundingBoxMax.z()};
    data["pointCount"] = pointCount;
    data["isReference"] = isReference;
    data["description"] = description;

    // Serialize transformation matrix
    QVariantList transformData;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            transformData.append(transform(row, col));
        }
    }
    data["transform"] = transformData;

    return data;
}

bool ScanInfo::deserialize(const QVariantMap& data)
{
    scanId = data.value("scanId").toString();
    filePath = data.value("filePath").toString();
    name = data.value("name").toString();

    QVariantList bboxMin = data.value("boundingBoxMin").toList();
    if (bboxMin.size() == 3)
    {
        boundingBoxMin = QVector3D(bboxMin[0].toFloat(), bboxMin[1].toFloat(), bboxMin[2].toFloat());
    }

    QVariantList bboxMax = data.value("boundingBoxMax").toList();
    if (bboxMax.size() == 3)
    {
        boundingBoxMax = QVector3D(bboxMax[0].toFloat(), bboxMax[1].toFloat(), bboxMax[2].toFloat());
    }

    pointCount = data.value("pointCount", 0).toInt();
    isReference = data.value("isReference", false).toBool();
    description = data.value("description").toString();

    // Deserialize transformation matrix
    QVariantList transformData = data.value("transform").toList();
    if (transformData.size() == 16)
    {
        transform.setToIdentity();
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                int index = row * 4 + col;
                transform(row, col) = transformData[index].toFloat();
            }
        }
    }

    return true;
}

// RegistrationResult implementation
QVariantMap RegistrationProject::RegistrationResult::serialize() const
{
    QVariantMap data;
    data["sourceScanId"] = sourceScanId;
    data["targetScanId"] = targetScanId;
    data["rmsError"] = rmsError;
    data["correspondenceCount"] = correspondenceCount;
    data["isValid"] = isValid;
    data["algorithm"] = algorithm;
    data["timestamp"] = timestamp.toString(Qt::ISODate);

    // Serialize transformation matrix
    QVariantList transformData;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            transformData.append(transformation(row, col));
        }
    }
    data["transformation"] = transformData;

    return data;
}

bool RegistrationProject::RegistrationResult::deserialize(const QVariantMap& data)
{
    sourceScanId = data.value("sourceScanId").toString();
    targetScanId = data.value("targetScanId").toString();
    rmsError = data.value("rmsError", 0.0f).toFloat();
    correspondenceCount = data.value("correspondenceCount", 0).toInt();
    isValid = data.value("isValid", false).toBool();
    algorithm = data.value("algorithm").toString();
    timestamp = QDateTime::fromString(data.value("timestamp").toString(), Qt::ISODate);

    // Deserialize transformation matrix
    QVariantList transformData = data.value("transformation").toList();
    if (transformData.size() == 16)
    {
        transformation.setToIdentity();
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                int index = row * 4 + col;
                transformation(row, col) = transformData[index].toFloat();
            }
        }
    }

    return true;
}

// RegistrationProject implementation
RegistrationProject::RegistrationProject(QObject* parent)
    : Project(parent), targetManager_(std::make_unique<TargetManager>(this)), registrationState_(NotStarted)
{
    initializeRegistrationProject();
}

RegistrationProject::RegistrationProject(const QString& name, const QString& path, QObject* parent)
    : Project(name, path, parent), targetManager_(std::make_unique<TargetManager>(this)), registrationState_(NotStarted)
{
    initializeRegistrationProject();
}

void RegistrationProject::initializeRegistrationProject()
{
    connectTargetManagerSignals();
    updateProjectMetadata();
}

void RegistrationProject::connectTargetManagerSignals()
{
    connect(targetManager_.get(), &TargetManager::dataChanged, this, &RegistrationProject::markAsModified);
    connect(targetManager_.get(), &TargetManager::targetAdded, this, &RegistrationProject::markAsModified);
    connect(targetManager_.get(), &TargetManager::targetRemoved, this, &RegistrationProject::markAsModified);
}

void RegistrationProject::addScan(const ScanInfo& scanInfo)
{
    if (scanInfo.scanId.isEmpty())
    {
        qWarning() << "RegistrationProject: Cannot add scan with empty ID";
        return;
    }

    if (scans_.contains(scanInfo.scanId))
    {
        qWarning() << "RegistrationProject: Scan with ID" << scanInfo.scanId << "already exists";
        return;
    }

    scans_[scanInfo.scanId] = scanInfo;

    // If this is the first scan or marked as reference, set it as reference
    if (referenceScanId_.isEmpty() || scanInfo.isReference)
    {
        setReferenceScan(scanInfo.scanId);
    }

    markAsModified();
    emit scanAdded(scanInfo.scanId);

    qDebug() << "RegistrationProject: Added scan" << scanInfo.scanId;
}

void RegistrationProject::removeScan(const QString& scanId)
{
    auto it = scans_.find(scanId);
    if (it == scans_.end())
    {
        qWarning() << "RegistrationProject: Scan" << scanId << "not found";
        return;
    }

    // Remove targets for this scan
    targetManager_->clearScan(scanId);

    // Remove registration results involving this scan
    auto resultIt = registrationResults_.begin();
    while (resultIt != registrationResults_.end())
    {
        if (resultIt->sourceScanId == scanId || resultIt->targetScanId == scanId)
        {
            resultIt = registrationResults_.erase(resultIt);
        }
        else
        {
            ++resultIt;
        }
    }

    // If this was the reference scan, choose a new one
    if (referenceScanId_ == scanId)
    {
        referenceScanId_.clear();
        if (!scans_.isEmpty())
        {
            setReferenceScan(scans_.firstKey());
        }
    }

    scans_.erase(it);
    markAsModified();
    emit scanRemoved(scanId);

    qDebug() << "RegistrationProject: Removed scan" << scanId;
}

void RegistrationProject::updateScan(const ScanInfo& scanInfo)
{
    if (!scans_.contains(scanInfo.scanId))
    {
        qWarning() << "RegistrationProject: Cannot update non-existent scan" << scanInfo.scanId;
        return;
    }

    scans_[scanInfo.scanId] = scanInfo;
    markAsModified();
    emit scanUpdated(scanInfo.scanId);

    qDebug() << "RegistrationProject: Updated scan" << scanInfo.scanId;
}

ScanInfo RegistrationProject::getScan(const QString& scanId) const
{
    return scans_.value(scanId, ScanInfo());
}

QList<ScanInfo> RegistrationProject::getAllScans() const
{
    return scans_.values();
}

QStringList RegistrationProject::getScanIds() const
{
    return scans_.keys();
}

int RegistrationProject::getScanCount() const
{
    return scans_.size();
}

bool RegistrationProject::hasScan(const QString& scanId) const
{
    return scans_.contains(scanId);
}

ScanInfo RegistrationProject::getReferenceScan() const
{
    return getScan(referenceScanId_);
}

void RegistrationProject::setReferenceScan(const QString& scanId)
{
    if (!scans_.contains(scanId))
    {
        qWarning() << "RegistrationProject: Cannot set non-existent scan as reference:" << scanId;
        return;
    }

    if (referenceScanId_ != scanId)
    {
        // Update old reference scan
        if (!referenceScanId_.isEmpty() && scans_.contains(referenceScanId_))
        {
            scans_[referenceScanId_].isReference = false;
        }

        // Update new reference scan
        referenceScanId_ = scanId;
        scans_[scanId].isReference = true;

        markAsModified();
        emit referenceScanChanged(scanId);

        qDebug() << "RegistrationProject: Reference scan set to" << scanId;
    }
}

void RegistrationProject::setScanTransform(const QString& scanId, const QMatrix4x4& transform)
{
    auto it = scans_.find(scanId);
    if (it != scans_.end())
    {
        it.value().transform = transform;
        markAsModified();
        emit scanUpdated(scanId);
    }
}

QMatrix4x4 RegistrationProject::getScanTransform(const QString& scanId) const
{
    auto it = scans_.find(scanId);
    if (it != scans_.end())
    {
        return it.value().transform;
    }

    QMatrix4x4 identity;
    identity.setToIdentity();
    return identity;
}

void RegistrationProject::resetScanTransforms()
{
    for (auto& scan : scans_)
    {
        scan.transform.setToIdentity();
    }
    markAsModified();

    qDebug() << "RegistrationProject: Reset all scan transforms";
}

void RegistrationProject::addRegistrationResult(const RegistrationResult& result)
{
    // Remove existing result for the same scan pair
    removeRegistrationResult(result.sourceScanId, result.targetScanId);

    registrationResults_.append(result);
    markAsModified();
    emit registrationResultAdded(result.sourceScanId, result.targetScanId);

    qDebug() << "RegistrationProject: Added registration result for" << result.sourceScanId << "to"
             << result.targetScanId;
}

void RegistrationProject::removeRegistrationResult(const QString& sourceScanId, const QString& targetScanId)
{
    auto it = std::find_if(registrationResults_.begin(),
                           registrationResults_.end(),
                           [&](const RegistrationResult& result)
                           {
                               return (result.sourceScanId == sourceScanId && result.targetScanId == targetScanId) ||
                                      (result.sourceScanId == targetScanId && result.targetScanId == sourceScanId);
                           });

    if (it != registrationResults_.end())
    {
        registrationResults_.erase(it);
        markAsModified();
        qDebug() << "RegistrationProject: Removed registration result for" << sourceScanId << "to" << targetScanId;
    }
}

QList<RegistrationProject::RegistrationResult> RegistrationProject::getRegistrationResults() const
{
    return registrationResults_;
}

RegistrationProject::RegistrationResult RegistrationProject::getRegistrationResult(const QString& sourceScanId,
                                                                                   const QString& targetScanId) const
{
    auto it = std::find_if(registrationResults_.begin(),
                           registrationResults_.end(),
                           [&](const RegistrationResult& result)
                           {
                               return (result.sourceScanId == sourceScanId && result.targetScanId == targetScanId) ||
                                      (result.sourceScanId == targetScanId && result.targetScanId == sourceScanId);
                           });

    return (it != registrationResults_.end()) ? *it : RegistrationResult();
}

bool RegistrationProject::hasRegistrationResult(const QString& sourceScanId, const QString& targetScanId) const
{
    return std::any_of(registrationResults_.begin(),
                       registrationResults_.end(),
                       [&](const RegistrationResult& result)
                       {
                           return (result.sourceScanId == sourceScanId && result.targetScanId == targetScanId) ||
                                  (result.sourceScanId == targetScanId && result.targetScanId == sourceScanId);
                       });
}

void RegistrationProject::setRegistrationState(RegistrationState state)
{
    if (registrationState_ != state)
    {
        registrationState_ = state;
        markAsModified();
        emit registrationStateChanged(state);
        qDebug() << "RegistrationProject: Registration state changed to" << registrationStateToString(state);
    }
}

float RegistrationProject::getOverallRegistrationQuality() const
{
    if (registrationResults_.isEmpty())
    {
        return 0.0f;
    }

    float totalQuality = 0.0f;
    int validResults = 0;

    for (const auto& result : registrationResults_)
    {
        if (result.isValid && result.rmsError > 0.0f)
        {
            // Convert RMS error to quality score (lower error = higher quality)
            float quality = 1.0f / (1.0f + result.rmsError);
            totalQuality += quality;
            validResults++;
        }
    }

    return (validResults > 0) ? totalQuality / validResults : 0.0f;
}

int RegistrationProject::getTotalCorrespondenceCount() const
{
    return targetManager_->getCorrespondenceCount();
}

int RegistrationProject::getValidCorrespondenceCount() const
{
    return targetManager_->getValidCorrespondenceCount();
}

QVariantMap RegistrationProject::serialize() const
{
    QVariantMap data = Project::serialize();

    // Add registration-specific data
    data["projectType"] = "Registration";
    data["registrationState"] = registrationStateToString(registrationState_);
    data["referenceScanId"] = referenceScanId_;

    // Serialize scans
    QVariantList scansList;
    for (const auto& scan : scans_)
    {
        scansList.append(scan.serialize());
    }
    data["scans"] = scansList;

    // Serialize target manager
    data["targetManager"] = targetManager_->serialize();

    // Serialize registration results
    QVariantList resultsList;
    for (const auto& result : registrationResults_)
    {
        resultsList.append(result.serialize());
    }
    data["registrationResults"] = resultsList;

    return data;
}

bool RegistrationProject::deserialize(const QVariantMap& data)
{
    if (!Project::deserialize(data))
    {
        return false;
    }

    // Deserialize registration-specific data
    registrationState_ = stringToRegistrationState(data.value("registrationState").toString());
    referenceScanId_ = data.value("referenceScanId").toString();

    // Deserialize scans
    scans_.clear();
    QVariantList scansList = data.value("scans").toList();
    for (const auto& scanVar : scansList)
    {
        ScanInfo scan;
        if (scan.deserialize(scanVar.toMap()))
        {
            scans_[scan.scanId] = scan;
        }
    }

    // Deserialize target manager
    QVariantMap targetManagerData = data.value("targetManager").toMap();
    if (!targetManagerData.isEmpty())
    {
        targetManager_->deserialize(targetManagerData);
    }

    // Deserialize registration results
    registrationResults_.clear();
    QVariantList resultsList = data.value("registrationResults").toList();
    for (const auto& resultVar : resultsList)
    {
        RegistrationResult result;
        if (result.deserialize(resultVar.toMap()))
        {
            registrationResults_.append(result);
        }
    }

    return true;
}

bool RegistrationProject::validate() const
{
    if (!Project::validate())
    {
        return false;
    }

    // Validate scans
    for (const auto& scan : scans_)
    {
        if (scan.scanId.isEmpty() || scan.filePath.isEmpty())
        {
            return false;
        }
    }

    // Validate reference scan
    if (!referenceScanId_.isEmpty() && !scans_.contains(referenceScanId_))
    {
        return false;
    }

    // Validate target manager
    if (!targetManager_->validate())
    {
        return false;
    }

    return true;
}

QStringList RegistrationProject::getValidationErrors() const
{
    QStringList errors;

    // Add base project validation errors
    // Note: Assuming base Project class has getValidationErrors method
    // If not, this line should be removed

    // Validate scans
    for (const auto& scan : scans_)
    {
        if (scan.scanId.isEmpty())
        {
            errors.append("Scan has empty ID");
        }
        if (scan.filePath.isEmpty())
        {
            errors.append(QString("Scan %1 has empty file path").arg(scan.scanId));
        }
    }

    // Validate reference scan
    if (!referenceScanId_.isEmpty() && !scans_.contains(referenceScanId_))
    {
        errors.append("Reference scan ID does not exist in scan list");
    }

    // Add target manager validation errors
    errors.append(targetManager_->getValidationErrors());

    return errors;
}

QString RegistrationProject::generateScanId() const
{
    int counter = 1;
    QString id;
    do
    {
        id = QString("scan_%1").arg(counter++);
    } while (scans_.contains(id));

    return id;
}

void RegistrationProject::updateProjectMetadata()
{
    // Update project metadata based on registration-specific information
    // This could include updating the project description, tags, etc.
}

// Utility functions
QString registrationStateToString(RegistrationProject::RegistrationState state)
{
    static const QMap<RegistrationProject::RegistrationState, QString> stateStrings = {
        {RegistrationProject::NotStarted, "NotStarted"},
        {RegistrationProject::ScanSelection, "ScanSelection"},
        {RegistrationProject::TargetDetection, "TargetDetection"},
        {RegistrationProject::ManualAlignment, "ManualAlignment"},
        {RegistrationProject::ICPRegistration, "ICPRegistration"},
        {RegistrationProject::QualityReview, "QualityReview"},
        {RegistrationProject::Completed, "Completed"}};
    return stateStrings.value(state, "Unknown");
}

RegistrationProject::RegistrationState stringToRegistrationState(const QString& stateString)
{
    static const QMap<QString, RegistrationProject::RegistrationState> stringStates = {
        {"NotStarted", RegistrationProject::NotStarted},
        {"ScanSelection", RegistrationProject::ScanSelection},
        {"TargetDetection", RegistrationProject::TargetDetection},
        {"ManualAlignment", RegistrationProject::ManualAlignment},
        {"ICPRegistration", RegistrationProject::ICPRegistration},
        {"QualityReview", RegistrationProject::QualityReview},
        {"Completed", RegistrationProject::Completed}};
    return stringStates.value(stateString, RegistrationProject::NotStarted);
}
