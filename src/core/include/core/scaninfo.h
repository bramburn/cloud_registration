#ifndef SCANINFO_H
#define SCANINFO_H

#include <QString>
#include <QVector3D>
#include <QMatrix4x4>
#include <QVariantMap>

/**
 * @brief Information about a scan in the project
 * 
 * This structure contains all the metadata and state information
 * for a scan within a project, including transformation data
 * for registration purposes.
 */
struct ScanInfo
{
    QString scanId;
    QString filePath;
    QString name;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    int pointCount;
    QMatrix4x4 transform;  // Current transformation matrix
    bool isReference;      // True if this is the reference scan
    QString description;

    ScanInfo() : pointCount(0), isReference(false)
    {
        transform.setToIdentity();
    }

    QVariantMap serialize() const;
    bool deserialize(const QVariantMap& data);
};

// Make ScanInfo available to Qt's meta-object system
Q_DECLARE_METATYPE(ScanInfo)

#endif // SCANINFO_H
