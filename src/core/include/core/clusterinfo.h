#ifndef CLUSTERINFO_H
#define CLUSTERINFO_H

#include <QString>
#include <QVariantMap>

/**
 * @brief Information about a cluster in the project
 * 
 * This structure contains all the metadata for a cluster
 * which is used to group scans together.
 */
struct ClusterInfo
{
    QString clusterId;
    QString name;
    QString parentClusterId;
    QString projectId;
    QString description;
    QString creationDate;
    bool isLocked = false;

    ClusterInfo() = default;

    bool isValid() const
    {
        return !clusterId.isEmpty() && !name.isEmpty();
    }

    QVariantMap serialize() const;
    bool deserialize(const QVariantMap& data);
};

// Make ClusterInfo available to Qt's meta-object system
Q_DECLARE_METATYPE(ClusterInfo)

#endif // CLUSTERINFO_H
