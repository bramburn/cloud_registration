#ifndef COORDINATESYSTEMMANAGER_H
#define COORDINATESYSTEMMANAGER_H

#include <QMatrix4x4>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector3D>

#include <vector>

/**
 * @brief Coordinate Reference System definition
 */
struct CRSDefinition
{
    QString name;
    QString code;  // EPSG code (e.g., "EPSG:4326")
    QString description;
    QString units;  // "meters", "degrees", "feet"
    QString type;   // "geographic", "projected", "geocentric"

    // Projection parameters (for projected systems)
    double centralMeridian = 0.0;
    double falseEasting = 0.0;
    double falseNorthing = 0.0;
    double scaleFactor = 1.0;
    double standardParallel1 = 0.0;
    double standardParallel2 = 0.0;

    // Datum parameters
    QString datumName;
    QString ellipsoidName;
    double semiMajorAxis = 6378137.0;         // WGS84 default
    double flattening = 1.0 / 298.257223563;  // WGS84 default

    // Additional metadata
    QVariantMap customParameters;
    bool isValid = true;

    CRSDefinition() = default;
    CRSDefinition(const QString& name_, const QString& code_, const QString& description_)
        : name(name_), code(code_), description(description_)
    {
    }
};

/**
 * @brief Transformation parameters between coordinate systems
 */
struct TransformationParameters
{
    QString sourceCRS;
    QString targetCRS;

    // 7-parameter Helmert transformation
    QVector3D translation;     // dx, dy, dz in meters
    QVector3D rotation;        // rx, ry, rz in radians
    double scaleFactor = 1.0;  // scale factor (unitless)

    // Additional transformation methods
    QString transformationMethod = "Helmert7";
    QVariantMap additionalParams;

    bool isValid = false;
    double accuracy = 0.0;  // Transformation accuracy in meters
};

/**
 * @brief Point structure for coordinate transformations
 */
struct CRSPoint
{
    double x, y, z;
    QString sourceCRS;
    QString targetCRS;

    CRSPoint() = default;
    CRSPoint(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    CRSPoint(double x_, double y_, double z_, const QString& crs) : x(x_), y(y_), z(z_), sourceCRS(crs) {}

    QVector3D toVector3D() const
    {
        return QVector3D(x, y, z);
    }
    void fromVector3D(const QVector3D& vec)
    {
        x = vec.x();
        y = vec.y();
        z = vec.z();
    }
};

/**
 * @brief Coordinate System Manager
 *
 * Sprint 6 User Story 3: Coordinate System Management and Transformation
 * Manages coordinate reference systems and provides transformation capabilities.
 */
class CoordinateSystemManager : public QObject
{
    Q_OBJECT

public:
    explicit CoordinateSystemManager(QObject* parent = nullptr);
    ~CoordinateSystemManager() override;

    // CRS Management
    QList<CRSDefinition> getAvailableSystems() const;
    CRSDefinition getCRSDefinition(const QString& code) const;
    bool addCustomCRS(const CRSDefinition& crs);
    bool removeCRS(const QString& code);

    // Predefined CRS
    void initializePredefinedCRS();
    QStringList getPredefinedCRSCodes() const;

    // Validation
    bool isValidCRS(const QString& code) const;
    bool isTransformationSupported(const QString& sourceCRS, const QString& targetCRS) const;

    // Transformation
    CRSPoint transformPoint(const CRSPoint& point, const QString& targetCRS);
    std::vector<CRSPoint> transformPoints(const std::vector<CRSPoint>& points, const QString& targetCRS);

    // Transformation parameters
    TransformationParameters getTransformationParameters(const QString& sourceCRS, const QString& targetCRS) const;
    bool setTransformationParameters(const TransformationParameters& params);

    // Utilities
    QString getUnitsForCRS(const QString& code) const;
    QString getTypeForCRS(const QString& code) const;
    QStringList searchCRS(const QString& searchTerm) const;

    // Configuration
    void setDefaultCRS(const QString& code)
    {
        m_defaultCRS = code;
    }
    QString getDefaultCRS() const
    {
        return m_defaultCRS;
    }

signals:
    void crsAdded(const QString& code);
    void crsRemoved(const QString& code);
    void transformationProgress(int percentage, const QString& stage);
    void transformationCompleted(int pointsTransformed);
    void transformationError(const QString& error);

private:
    // Core transformation methods
    CRSPoint performGeographicToProjected(const CRSPoint& point, const CRSDefinition& targetCRS);
    CRSPoint performProjectedToGeographic(const CRSPoint& point, const CRSDefinition& sourceCRS);
    CRSPoint performHelmertTransformation(const CRSPoint& point, const TransformationParameters& params);

    // Specific projection implementations
    CRSPoint performUTMProjection(const CRSPoint& geographicPoint, int zone, bool isNorth);
    CRSPoint performUTMInverseProjection(const CRSPoint& projectedPoint, int zone, bool isNorth);
    CRSPoint performMercatorProjection(const CRSPoint& geographicPoint, const CRSDefinition& crs);
    CRSPoint performMercatorInverseProjection(const CRSPoint& projectedPoint, const CRSDefinition& crs);

    // Utility methods
    int getUTMZone(double longitude) const;
    bool isNorthernHemisphere(double latitude) const;
    double degToRad(double degrees) const
    {
        return degrees * M_PI / 180.0;
    }
    double radToDeg(double radians) const
    {
        return radians * 180.0 / M_PI;
    }

    // Data storage
    QList<CRSDefinition> m_crsList;
    QList<TransformationParameters> m_transformationParams;
    QString m_defaultCRS = "EPSG:4326";  // WGS84 default

    // Predefined CRS initialization helpers
    void addPredefinedCRS(const CRSDefinition& crs);
    CRSDefinition createWGS84();
    CRSDefinition createUTMZone(int zone, bool isNorth);
    CRSDefinition createWebMercator();
};

#endif  // COORDINATESYSTEMMANAGER_H
