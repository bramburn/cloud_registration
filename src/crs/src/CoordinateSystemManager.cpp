#include "crs/CoordinateSystemManager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <cmath>

#include "export/IFormatWriter.h"  // For Point struct

// Constants
const double CoordinateSystemManager::EARTH_RADIUS = 6378137.0;  // WGS84 semi-major axis
const double CoordinateSystemManager::DEGREES_TO_RADIANS = M_PI / 180.0;
const double CoordinateSystemManager::RADIANS_TO_DEGREES = 180.0 / M_PI;

/**
 * @brief Private implementation data for CoordinateSystemManager
 */
struct CoordinateSystemManager::PrivateData
{
    QMap<QString, CRSDefinition> predefinedCRS;
    QMap<QString, CRSDefinition> customCRS;
    QMap<QString, TransformationParameters> cachedTransformations;
};

CoordinateSystemManager::CoordinateSystemManager(QObject* parent) : QObject(parent), d(std::make_unique<PrivateData>())
{
    initializePredefinedCRS();
}

CoordinateSystemManager::~CoordinateSystemManager() = default;

QStringList CoordinateSystemManager::getAvailableCRS() const
{
    QStringList crsList;

    // Add predefined CRS
    for (auto it = d->predefinedCRS.begin(); it != d->predefinedCRS.end(); ++it)
    {
        crsList << it.key();
    }

    // Add custom CRS
    for (auto it = d->customCRS.begin(); it != d->customCRS.end(); ++it)
    {
        crsList << it.key();
    }

    crsList.sort();
    return crsList;
}

CRSDefinition CoordinateSystemManager::getCRSDefinition(const QString& crsName) const
{
    // Check predefined CRS first
    if (d->predefinedCRS.contains(crsName))
    {
        return d->predefinedCRS[crsName];
    }

    // Check custom CRS
    if (d->customCRS.contains(crsName))
    {
        return d->customCRS[crsName];
    }

    // Return invalid definition
    CRSDefinition invalid;
    invalid.name = "Invalid";
    return invalid;
}

bool CoordinateSystemManager::addCustomCRS(const CRSDefinition& definition)
{
    QString validationError = validateCRSDefinition(definition);
    if (!validationError.isEmpty())
    {
        m_lastError = validationError;
        return false;
    }

    d->customCRS[definition.name] = definition;
    emit crsDefinitionsUpdated();

    qDebug() << "CoordinateSystemManager: Added custom CRS:" << definition.name;
    return true;
}

bool CoordinateSystemManager::removeCustomCRS(const QString& crsName)
{
    if (!d->customCRS.contains(crsName))
    {
        m_lastError = QString("Custom CRS '%1' not found").arg(crsName);
        return false;
    }

    d->customCRS.remove(crsName);
    emit crsDefinitionsUpdated();

    qDebug() << "CoordinateSystemManager: Removed custom CRS:" << crsName;
    return true;
}

bool CoordinateSystemManager::isTransformationAvailable(const QString& sourceCRS, const QString& targetCRS) const
{
    if (sourceCRS == targetCRS)
    {
        return true;  // Identity transformation
    }

    CRSDefinition source = getCRSDefinition(sourceCRS);
    CRSDefinition target = getCRSDefinition(targetCRS);

    return (source.name != "Invalid" && target.name != "Invalid");
}

TransformationParameters CoordinateSystemManager::getTransformationParameters(const QString& sourceCRS,
                                                                              const QString& targetCRS) const
{
    QString transformKey = sourceCRS + "->" + targetCRS;

    // Check cache first
    if (d->cachedTransformations.contains(transformKey))
    {
        return d->cachedTransformations[transformKey];
    }

    TransformationParameters params;
    params.sourceCRS = sourceCRS;
    params.targetCRS = targetCRS;

    if (sourceCRS == targetCRS)
    {
        // Identity transformation
        params.transformationMatrix.setToIdentity();
        params.accuracy = 0.0;
        params.isValid = true;
    }
    else
    {
        CRSDefinition source = getCRSDefinition(sourceCRS);
        CRSDefinition target = getCRSDefinition(targetCRS);

        if (source.name != "Invalid" && target.name != "Invalid")
        {
            params.transformationMatrix = createTransformationMatrix(source, target);
            params.accuracy = 0.1;  // Default accuracy
            params.isValid = true;
        }
    }

    // Cache the result
    d->cachedTransformations[transformKey] = params;

    return params;
}

QVector3D CoordinateSystemManager::transformPoint(const QVector3D& point,
                                                  const QString& sourceCRS,
                                                  const QString& targetCRS) const
{
    if (sourceCRS == targetCRS)
    {
        return point;  // No transformation needed
    }

    TransformationParameters params = getTransformationParameters(sourceCRS, targetCRS);
    if (!params.isValid)
    {
        qWarning() << "CoordinateSystemManager: Invalid transformation from" << sourceCRS << "to" << targetCRS;
        return point;
    }

    // Apply transformation matrix
    QVector4D homogeneousPoint(point.x(), point.y(), point.z(), 1.0f);
    QVector4D transformedPoint = params.transformationMatrix * homogeneousPoint;

    return QVector3D(transformedPoint.x(), transformedPoint.y(), transformedPoint.z());
}

std::vector<Point> CoordinateSystemManager::transformPoints(const std::vector<Point>& points,
                                                            const QString& sourceCRS,
                                                            const QString& targetCRS) const
{
    std::vector<Point> transformedPoints;
    transformedPoints.reserve(points.size());

    if (sourceCRS == targetCRS)
    {
        return points;  // No transformation needed
    }

    TransformationParameters params = getTransformationParameters(sourceCRS, targetCRS);
    if (!params.isValid)
    {
        qWarning() << "CoordinateSystemManager: Invalid transformation from" << sourceCRS << "to" << targetCRS;
        return points;
    }

    emit transformationProgress(0, "Starting coordinate transformation...");

    size_t totalPoints = points.size();
    for (size_t i = 0; i < totalPoints; ++i)
    {
        const Point& originalPoint = points[i];

        // Transform coordinates
        QVector3D originalPos(originalPoint.x, originalPoint.y, originalPoint.z);
        QVector3D transformedPos = transformPoint(originalPos, sourceCRS, targetCRS);

        // Create transformed point
        Point transformedPoint = originalPoint;
        transformedPoint.x = transformedPos.x();
        transformedPoint.y = transformedPos.y();
        transformedPoint.z = transformedPos.z();

        transformedPoints.push_back(transformedPoint);

        // Update progress
        if (i % 1000 == 0)
        {
            int progress = static_cast<int>((i * 100) / totalPoints);
            emit transformationProgress(progress, QString("Transforming points: %1/%2").arg(i).arg(totalPoints));
        }
    }

    emit transformationProgress(100, "Coordinate transformation completed");

    qDebug() << "CoordinateSystemManager: Transformed" << points.size() << "points from" << sourceCRS << "to"
             << targetCRS;
    return transformedPoints;
}

QMatrix4x4 CoordinateSystemManager::calculateTransformationMatrix(const QString& sourceCRS,
                                                                  const QString& targetCRS) const
{
    CRSDefinition source = getCRSDefinition(sourceCRS);
    CRSDefinition target = getCRSDefinition(targetCRS);

    return createTransformationMatrix(source, target);
}

QString CoordinateSystemManager::validateCRSDefinition(const CRSDefinition& definition) const
{
    if (definition.name.isEmpty())
    {
        return "CRS name cannot be empty";
    }

    if (definition.units.isEmpty())
    {
        return "CRS units must be specified";
    }

    if (definition.type != "geographic" && definition.type != "projected" && definition.type != "local")
    {
        return "CRS type must be 'geographic', 'projected', or 'local'";
    }

    // Check for duplicate names
    if (d->predefinedCRS.contains(definition.name) || d->customCRS.contains(definition.name))
    {
        return QString("CRS name '%1' already exists").arg(definition.name);
    }

    return QString();  // Valid
}

bool CoordinateSystemManager::loadCRSDefinitions(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        m_lastError = QString("Cannot open CRS definitions file: %1").arg(file.errorString());
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        m_lastError = QString("JSON parse error: %1").arg(parseError.errorString());
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray crsArray = root["coordinate_systems"].toArray();

    int loadedCount = 0;
    for (const QJsonValue& value : crsArray)
    {
        QJsonObject crsObj = value.toObject();

        CRSDefinition definition;
        definition.name = crsObj["name"].toString();
        definition.code = crsObj["code"].toString();
        definition.description = crsObj["description"].toString();
        definition.units = crsObj["units"].toString();
        definition.type = crsObj["type"].toString();

        // Load transformation parameters
        QJsonObject origin = crsObj["origin"].toObject();
        definition.origin = QVector3D(origin["x"].toDouble(), origin["y"].toDouble(), origin["z"].toDouble());

        QJsonObject scale = crsObj["scale"].toObject();
        definition.scale = QVector3D(scale["x"].toDouble(1.0), scale["y"].toDouble(1.0), scale["z"].toDouble(1.0));

        QJsonObject rotation = crsObj["rotation"].toObject();
        definition.rotation = QVector3D(rotation["x"].toDouble(), rotation["y"].toDouble(), rotation["z"].toDouble());

        // Load projection parameters
        definition.centralMeridian = crsObj["central_meridian"].toDouble();
        definition.standardParallel1 = crsObj["standard_parallel_1"].toDouble();
        definition.standardParallel2 = crsObj["standard_parallel_2"].toDouble();
        definition.falseEasting = crsObj["false_easting"].toDouble();
        definition.falseNorthing = crsObj["false_northing"].toDouble();

        if (addCustomCRS(definition))
        {
            loadedCount++;
        }
    }

    qDebug() << "CoordinateSystemManager: Loaded" << loadedCount << "CRS definitions from" << filePath;
    return true;
}

bool CoordinateSystemManager::saveCRSDefinitions(const QString& filePath) const
{
    QJsonObject root;
    QJsonArray crsArray;

    // Save custom CRS only
    for (auto it = d->customCRS.begin(); it != d->customCRS.end(); ++it)
    {
        const CRSDefinition& definition = it.value();

        QJsonObject crsObj;
        crsObj["name"] = definition.name;
        crsObj["code"] = definition.code;
        crsObj["description"] = definition.description;
        crsObj["units"] = definition.units;
        crsObj["type"] = definition.type;

        // Save transformation parameters
        QJsonObject origin;
        origin["x"] = definition.origin.x();
        origin["y"] = definition.origin.y();
        origin["z"] = definition.origin.z();
        crsObj["origin"] = origin;

        QJsonObject scale;
        scale["x"] = definition.scale.x();
        scale["y"] = definition.scale.y();
        scale["z"] = definition.scale.z();
        crsObj["scale"] = scale;

        QJsonObject rotation;
        rotation["x"] = definition.rotation.x();
        rotation["y"] = definition.rotation.y();
        rotation["z"] = definition.rotation.z();
        crsObj["rotation"] = rotation;

        // Save projection parameters
        crsObj["central_meridian"] = definition.centralMeridian;
        crsObj["standard_parallel_1"] = definition.standardParallel1;
        crsObj["standard_parallel_2"] = definition.standardParallel2;
        crsObj["false_easting"] = definition.falseEasting;
        crsObj["false_northing"] = definition.falseNorthing;

        crsArray.append(crsObj);
    }

    root["coordinate_systems"] = crsArray;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        m_lastError = QString("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());

    qDebug() << "CoordinateSystemManager: Saved" << d->customCRS.size() << "custom CRS definitions to" << filePath;
    return true;
}

void CoordinateSystemManager::initializePredefinedCRS()
{
    // WGS84 Geographic
    CRSDefinition wgs84;
    wgs84.name = "WGS84";
    wgs84.code = "EPSG:4326";
    wgs84.description = "World Geodetic System 1984";
    wgs84.units = "degrees";
    wgs84.type = "geographic";
    addPredefinedCRS(wgs84);

    // UTM Zone 10N
    CRSDefinition utm10n;
    utm10n.name = "UTM Zone 10N";
    utm10n.code = "EPSG:32610";
    utm10n.description = "Universal Transverse Mercator Zone 10 North";
    utm10n.units = "meters";
    utm10n.type = "projected";
    utm10n.centralMeridian = -123.0;
    utm10n.falseEasting = 500000.0;
    utm10n.falseNorthing = 0.0;
    addPredefinedCRS(utm10n);

    // UTM Zone 11N
    CRSDefinition utm11n;
    utm11n.name = "UTM Zone 11N";
    utm11n.code = "EPSG:32611";
    utm11n.description = "Universal Transverse Mercator Zone 11 North";
    utm11n.units = "meters";
    utm11n.type = "projected";
    utm11n.centralMeridian = -117.0;
    utm11n.falseEasting = 500000.0;
    utm11n.falseNorthing = 0.0;
    addPredefinedCRS(utm11n);

    // State Plane California I
    CRSDefinition spca1;
    spca1.name = "State Plane CA I";
    spca1.code = "EPSG:2225";
    spca1.description = "NAD83 / California zone 1";
    spca1.units = "feet";
    spca1.type = "projected";
    spca1.centralMeridian = -122.0;
    spca1.standardParallel1 = 40.0;
    spca1.standardParallel2 = 41.666667;
    spca1.falseEasting = 6561666.667;
    spca1.falseNorthing = 1640416.667;
    addPredefinedCRS(spca1);

    // Local coordinate system
    CRSDefinition local;
    local.name = "Local";
    local.code = "LOCAL:1";
    local.description = "Local coordinate system";
    local.units = "meters";
    local.type = "local";
    addPredefinedCRS(local);

    qDebug() << "CoordinateSystemManager: Initialized" << d->predefinedCRS.size() << "predefined CRS";
}

void CoordinateSystemManager::addPredefinedCRS(const CRSDefinition& definition)
{
    d->predefinedCRS[definition.name] = definition;
}

QMatrix4x4 CoordinateSystemManager::createTransformationMatrix(const CRSDefinition& source,
                                                               const CRSDefinition& target) const
{
    QMatrix4x4 matrix;
    matrix.setToIdentity();

    // Simplified transformation - in practice would use proper geodetic transformations

    // Apply source inverse transformation
    QMatrix4x4 sourceInverse;
    sourceInverse.setToIdentity();
    sourceInverse.translate(-source.origin);
    sourceInverse.scale(1.0f / source.scale.x(), 1.0f / source.scale.y(), 1.0f / source.scale.z());

    // Apply rotation (simplified)
    if (source.rotation.length() > 0.001f)
    {
        sourceInverse.rotate(-source.rotation.x(), 1, 0, 0);
        sourceInverse.rotate(-source.rotation.y(), 0, 1, 0);
        sourceInverse.rotate(-source.rotation.z(), 0, 0, 1);
    }

    // Apply target transformation
    QMatrix4x4 targetTransform;
    targetTransform.setToIdentity();

    // Apply rotation
    if (target.rotation.length() > 0.001f)
    {
        targetTransform.rotate(target.rotation.x(), 1, 0, 0);
        targetTransform.rotate(target.rotation.y(), 0, 1, 0);
        targetTransform.rotate(target.rotation.z(), 0, 0, 1);
    }

    targetTransform.scale(target.scale);
    targetTransform.translate(target.origin);

    // Combine transformations
    matrix = targetTransform * sourceInverse;

    return matrix;
}

QVector3D CoordinateSystemManager::applyProjection(const QVector3D& point, const CRSDefinition& crs) const
{
    // Simplified projection - in practice would implement proper map projections
    if (crs.type == "geographic")
    {
        return point;  // No projection needed
    }
    else if (crs.type == "projected")
    {
        // Simple UTM-like projection
        double x = (point.x() - crs.centralMeridian) * DEGREES_TO_RADIANS * EARTH_RADIUS;
        double y = point.y() * DEGREES_TO_RADIANS * EARTH_RADIUS;
        return QVector3D(x + crs.falseEasting, y + crs.falseNorthing, point.z());
    }
    else
    {
        return point;  // Local coordinates
    }
}

QVector3D CoordinateSystemManager::applyInverseProjection(const QVector3D& point, const CRSDefinition& crs) const
{
    // Simplified inverse projection
    if (crs.type == "geographic")
    {
        return point;  // No projection needed
    }
    else if (crs.type == "projected")
    {
        // Simple inverse UTM-like projection
        double x = ((point.x() - crs.falseEasting) / EARTH_RADIUS) * RADIANS_TO_DEGREES + crs.centralMeridian;
        double y = ((point.y() - crs.falseNorthing) / EARTH_RADIUS) * RADIANS_TO_DEGREES;
        return QVector3D(x, y, point.z());
    }
    else
    {
        return point;  // Local coordinates
    }
}

bool CoordinateSystemManager::isGeographic(const CRSDefinition& crs) const
{
    return crs.type == "geographic";
}

bool CoordinateSystemManager::isProjected(const CRSDefinition& crs) const
{
    return crs.type == "projected";
}

bool CoordinateSystemManager::isLocal(const CRSDefinition& crs) const
{
    return crs.type == "local";
}

QVector3D CoordinateSystemManager::geographicToCartesian(const QVector3D& geographic) const
{
    // Convert geographic coordinates (lon, lat, height) to cartesian (x, y, z)
    double lon = geographic.x() * DEGREES_TO_RADIANS;
    double lat = geographic.y() * DEGREES_TO_RADIANS;
    double height = geographic.z();

    double cosLat = std::cos(lat);
    double sinLat = std::sin(lat);
    double cosLon = std::cos(lon);
    double sinLon = std::sin(lon);

    double radius = EARTH_RADIUS + height;

    double x = radius * cosLat * cosLon;
    double y = radius * cosLat * sinLon;
    double z = radius * sinLat;

    return QVector3D(x, y, z);
}

QVector3D CoordinateSystemManager::cartesianToGeographic(const QVector3D& cartesian) const
{
    // Convert cartesian coordinates (x, y, z) to geographic (lon, lat, height)
    double x = cartesian.x();
    double y = cartesian.y();
    double z = cartesian.z();

    double radius = std::sqrt(x * x + y * y + z * z);
    double lon = std::atan2(y, x) * RADIANS_TO_DEGREES;
    double lat = std::asin(z / radius) * RADIANS_TO_DEGREES;
    double height = radius - EARTH_RADIUS;

    return QVector3D(lon, lat, height);
}
