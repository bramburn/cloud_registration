#ifndef IPOINTCLOUDVIEWER_H
#define IPOINTCLOUDVIEWER_H

#include <vector>
#include <QString>
#include <QVector3D>
#include <QColor>

/**
 * @brief Enumeration of possible viewer states
 */
enum class ViewerState {
    Idle,
    Loading,
    DisplayingData,
    LoadFailed,
    Error
};

/**
 * @brief Abstract interface for point cloud viewer components
 * 
 * This interface decouples the application's core logic from the specific
 * Qt and OpenGL implementation details of the point cloud viewer.
 */
class IPointCloudViewer
{
public:
    virtual ~IPointCloudViewer() = default;

    // Core point cloud operations
    virtual void loadPointCloud(const std::vector<float>& points) = 0;
    virtual void clearPointCloud() = 0;
    virtual void addPointCloudData(const std::vector<float>& additionalPoints) = 0;

    // State management
    virtual void setState(ViewerState state, const QString& message = "") = 0;
    virtual ViewerState getState() const = 0;

    // Rendering settings
    virtual void setPointSize(float size) = 0;
    virtual void setBackgroundColor(const QColor& color) = 0;
    virtual void setShowGrid(bool show) = 0;
    virtual void setShowAxes(bool show) = 0;

    // Level of Detail (LOD) settings
    virtual void setLODEnabled(bool enabled) = 0;
    virtual bool isLODEnabled() const = 0;

    // Color and intensity rendering
    virtual void setRenderWithColor(bool enabled) = 0;
    virtual void setRenderWithIntensity(bool enabled) = 0;
    virtual bool isRenderingWithColor() const = 0;
    virtual bool isRenderingWithIntensity() const = 0;

    // View controls
    virtual void setTopView() = 0;
    virtual void setLeftView() = 0;
    virtual void setRightView() = 0;
    virtual void setBottomView() = 0;
    virtual void setFrontView() = 0;
    virtual void setBackView() = 0;
    virtual void setIsometricView() = 0;

    // Data access
    virtual bool hasData() const = 0;
    virtual size_t pointCount() const = 0;

    // Performance and rendering settings
    virtual void setMinPointSize(float size) = 0;
    virtual void setMaxPointSize(float size) = 0;
    virtual void setAttenuationEnabled(bool enabled) = 0;
    virtual void setAttenuationFactor(float factor) = 0;

    // Advanced rendering features
    virtual void setSplattingEnabled(bool enabled) = 0;
    virtual void setLightingEnabled(bool enabled) = 0;
    virtual void setLightDirection(const QVector3D& direction) = 0;
    virtual void setLightColor(const QColor& color) = 0;
    virtual void setAmbientIntensity(float intensity) = 0;

    // Loading progress feedback
    virtual void onLoadingStarted() = 0;
    virtual void onLoadingProgress(int percentage, const QString& stage) = 0;
    virtual void onLoadingFinished(bool success, const QString& message) = 0;

    // Memory and performance
    virtual size_t getMemoryUsage() const = 0;
    virtual void optimizeMemory() = 0;
};

#endif // IPOINTCLOUDVIEWER_H
