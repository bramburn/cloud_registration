#include "app/pointcloudloadmanager.h"
#include "algorithms/ICPRegistration.h"

#include <QDebug>
#include <QVector3D>
#include <cmath>
#include <random>

PointCloudLoadManager::PointCloudLoadManager(QObject* parent) : QObject(parent) {}

void PointCloudLoadManager::loadPointCloud(const QString& filePath)
{
    m_isLoading = true;
    emit loadingStarted(filePath);

    // Stub implementation - actual loading logic will be added in future sprints
    emit loadingProgress(100, "Complete");
    emit loadingFinished(true, "Loaded successfully", std::vector<float>());
    m_isLoading = false;
}

void PointCloudLoadManager::cancelLoading()
{
    if (m_isLoading)
    {
        m_isLoading = false;
        emit loadingCancelled();
    }
}

// Sprint 4: Sidebar integration methods
bool PointCloudLoadManager::loadScan(const QString& scanId)
{
    if (scanId.isEmpty())
    {
        return false;
    }

    if (m_loadedScans.contains(scanId))
    {
        return true;  // Already loaded
    }

    // Stub implementation - actual loading logic will be added in future sprints
    m_loadedScans.append(scanId);
    return true;
}

bool PointCloudLoadManager::unloadScan(const QString& scanId)
{
    if (scanId.isEmpty())
    {
        return false;
    }

    if (!m_loadedScans.contains(scanId))
    {
        return true;  // Already unloaded
    }

    // Stub implementation - actual unloading logic will be added in future sprints
    m_loadedScans.removeAll(scanId);
    return true;
}

bool PointCloudLoadManager::isScanLoaded(const QString& scanId) const
{
    return m_loadedScans.contains(scanId);
}

PointCloud PointCloudLoadManager::getLoadedPointCloud(const QString& scanId) const
{
    if (!isScanLoaded(scanId))
    {
        qWarning() << "Scan" << scanId << "is not loaded";
        return PointCloud(); // Return empty point cloud
    }

    // TODO: Implement actual point cloud retrieval from memory/disk
    // For now, return a placeholder point cloud with some test data
    PointCloud cloud;

    // Generate some test points for demonstration
    std::vector<float> testPoints = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    };

    cloud = PointCloud(testPoints);

    qDebug() << "Retrieved point cloud for scan" << scanId << "with" << cloud.size() << "points";
    return cloud;
}

// Sprint 6.1: Get loaded point data for deviation analysis
std::vector<PointFullData> PointCloudLoadManager::getLoadedPointFullData(const QString& scanId) const
{
    std::vector<PointFullData> points;

    if (isScanLoaded(scanId))
    {
        qDebug() << "PointCloudLoadManager: Generating test point cloud data for scan" << scanId;

        // Generate test data with spherical targets for detection testing
        // This creates a synthetic point cloud with embedded spheres

        // Sphere 1: Center at (2, 2, 2), radius 0.1
        QVector3D center1(2.0f, 2.0f, 2.0f);
        float radius1 = 0.1f;
        generateSpherePoints(points, center1, radius1, 200);

        // Sphere 2: Center at (-1, 3, 1), radius 0.15
        QVector3D center2(-1.0f, 3.0f, 1.0f);
        float radius2 = 0.15f;
        generateSpherePoints(points, center2, radius2, 300);

        // Add some random background points
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-5.0f, 5.0f);

        for (int i = 0; i < 1000; ++i)
        {
            float x = dis(gen);
            float y = dis(gen);
            float z = dis(gen);

            // Skip points too close to sphere centers to avoid interference
            QVector3D point(x, y, z);
            if ((point - center1).length() > radius1 + 0.05f &&
                (point - center2).length() > radius2 + 0.05f)
            {
                points.emplace_back(x, y, z, 128, 128, 128);
            }
        }

        qDebug() << "Generated" << points.size() << "test points with 2 embedded spheres";
    }

    return points;
}

void PointCloudLoadManager::generateSpherePoints(std::vector<PointFullData>& points,
                                                const QVector3D& center,
                                                float radius,
                                                int numPoints) const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    std::normal_distribution<float> noise(0.0f, 0.002f); // Small amount of noise

    for (int i = 0; i < numPoints; ++i)
    {
        // Generate random point on sphere surface using spherical coordinates
        float theta = 2.0f * 3.14159265359f * dis(gen);  // Azimuthal angle
        float phi = std::acos(2.0f * dis(gen) - 1.0f);  // Polar angle

        float x = center.x() + radius * std::sin(phi) * std::cos(theta) + noise(gen);
        float y = center.y() + radius * std::sin(phi) * std::sin(theta) + noise(gen);
        float z = center.z() + radius * std::cos(phi) + noise(gen);

        // Color spheres differently for visual distinction
        uint8_t r = (center.x() > 0) ? 255 : 100;
        uint8_t g = (center.y() > 0) ? 255 : 100;
        uint8_t b = (center.z() > 0) ? 255 : 100;

        points.emplace_back(x, y, z, r, g, b);
    }
}
