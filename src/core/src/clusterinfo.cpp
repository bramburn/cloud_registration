#include "core/clusterinfo.h"

QVariantMap ClusterInfo::serialize() const
{
    QVariantMap data;
    data["clusterId"] = clusterId;
    data["name"] = name;
    data["parentClusterId"] = parentClusterId;
    data["projectId"] = projectId;
    data["description"] = description;
    data["creationDate"] = creationDate;
    data["isLocked"] = isLocked;

    return data;
}

bool ClusterInfo::deserialize(const QVariantMap& data)
{
    clusterId = data.value("clusterId").toString();
    name = data.value("name").toString();
    parentClusterId = data.value("parentClusterId").toString();
    projectId = data.value("projectId").toString();
    description = data.value("description").toString();
    creationDate = data.value("creationDate").toString();
    isLocked = data.value("isLocked", false).toBool();

    return true;
}
