#include "core/scaninfo.h"

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
