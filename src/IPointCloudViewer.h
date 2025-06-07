#ifndef IPOINTCLOUDVIEWER_H
#define IPOINTCLOUDVIEWER_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QVector3D>
#include <vector>

/**
 * @brief ViewerState - Enumeration of possible viewer states
 */
enum class ViewerState {
    Empty,          // No point cloud loaded
    Loading,        // Point cloud is being loaded
    Ready,          // Point cloud loaded and ready for display
    Rendering,      // Currently rendering
    Error           // Error state
};

/**
 * @brief IPointCloudViewer - Abstract interface for 3D point cloud rendering
 * 
 * This interface defines the contract for all point cloud viewer implementations.
 * It enables loose coupling between the rendering logic and the rest of the application,
 * allowing for easy testing with mock implementations and future substitution
 * of different rendering engines.
 * 
 * Sprint 3 Decoupling Requirements:
 * - Provides abstraction layer for 3D point cloud rendering operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Maintains compatibility with existing MainWindow interface
 */
class IPointCloudViewer : public QObject {
    Q_OBJECT

public:
    explicit IPointCloudViewer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IPointCloudViewer() = default;

    /**
     * @brief Load point cloud data for rendering
     * @param points Vector of point coordinates (x,y,z interleaved)
     */
    virtual void loadPointCloud(const std::vector<float>& points) = 0;

    /**
     * @brief Clear the currently loaded point cloud
     */
    virtual void clearPointCloud() = 0;

    /**
     * @brief Set the viewer state and optional message
     * @param state New state for the viewer
     * @param message Optional message associated with the state
     */
    virtual void setState(ViewerState state, const QString& message = QString()) = 0;

    /**
     * @brief Get the current viewer state
     * @return Current viewer state
     */
    virtual ViewerState getState() const = 0;

    /**
     * @brief Enable or disable Level of Detail (LOD) rendering
     * @param enabled true to enable LOD, false to disable
     */
    virtual void setLODEnabled(bool enabled) = 0;

    /**
     * @brief Enable or disable color rendering
     * @param enabled true to render with color, false for monochrome
     */
    virtual void setRenderWithColor(bool enabled) = 0;

    /**
     * @brief Enable or disable intensity-based rendering
     * @param enabled true to render with intensity, false for uniform color
     */
    virtual void setRenderWithIntensity(bool enabled) = 0;

    /**
     * @brief Set the point size for rendering
     * @param size Point size in pixels
     */
    virtual void setPointSize(float size) = 0;

    /**
     * @brief Set the background color
     * @param color Background color
     */
    virtual void setBackgroundColor(const QColor& color) = 0;

    /**
     * @brief Enable or disable point size attenuation
     * @param enabled true to enable attenuation, false for constant size
     */
    virtual void setPointSizeAttenuationEnabled(bool enabled) = 0;

    /**
     * @brief Set point size attenuation parameters
     * @param minSize Minimum point size
     * @param maxSize Maximum point size
     * @param factor Attenuation factor
     */
    virtual void setPointSizeAttenuationParams(float minSize, float maxSize, float factor) = 0;

    /**
     * @brief Enable or disable splatting rendering
     * @param enabled true to enable splatting, false for point rendering
     */
    virtual void setSplattingEnabled(bool enabled) = 0;

    /**
     * @brief Enable or disable lighting
     * @param enabled true to enable lighting, false for flat shading
     */
    virtual void setLightingEnabled(bool enabled) = 0;

    /**
     * @brief Set the light direction
     * @param direction Light direction vector
     */
    virtual void setLightDirection(const QVector3D& direction) = 0;

    /**
     * @brief Set the light color
     * @param color Light color
     */
    virtual void setLightColor(const QColor& color) = 0;

    /**
     * @brief Set the ambient light intensity
     * @param intensity Ambient intensity (0.0 to 1.0)
     */
    virtual void setAmbientIntensity(float intensity) = 0;

    /**
     * @brief Check if point cloud data is loaded
     * @return true if data is loaded, false otherwise
     */
    virtual bool hasData() const = 0;

    /**
     * @brief Get the number of points currently loaded
     * @return Number of points
     */
    virtual size_t pointCount() const = 0;

    /**
     * @brief Reset the camera to fit the loaded point cloud
     */
    virtual void resetCamera() = 0;

    /**
     * @brief Set camera to top view
     */
    virtual void setTopView() = 0;

    /**
     * @brief Set camera to front view
     */
    virtual void setFrontView() = 0;

    /**
     * @brief Set camera to side view
     */
    virtual void setSideView() = 0;

signals:
    /**
     * @brief Emitted when point cloud loading starts
     */
    void pointCloudLoadingStarted();

    /**
     * @brief Emitted when point cloud is successfully loaded
     * @param points Vector of loaded points
     */
    void pointCloudLoaded(const std::vector<float>& points);

    /**
     * @brief Emitted when point cloud loading fails
     * @param error Error message
     */
    void pointCloudLoadFailed(const QString& error);

    /**
     * @brief Emitted when point cloud is cleared
     */
    void pointCloudCleared();

    /**
     * @brief Emitted when viewer state changes
     * @param newState New viewer state
     * @param message Optional message associated with the state change
     */
    void stateChanged(ViewerState newState, const QString& message);

    /**
     * @brief Emitted when a rendering error occurs
     * @param error Error message
     */
    void renderingError(const QString& error);

    /**
     * @brief Emitted with rendering statistics
     * @param fps Frames per second
     * @param visiblePoints Number of visible points
     */
    void statsUpdated(float fps, int visiblePoints);
};

#endif // IPOINTCLOUDVIEWER_H
