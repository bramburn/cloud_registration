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
    Empty,       // No point cloud loaded
    Loading,     // Point cloud is being loaded
    Ready,       // Point cloud loaded and ready for display
    Rendering,   // Currently rendering
    Error        // Error state
};

/**
 * @brief IPointCloudViewer - Abstract interface for 3D point cloud rendering
 *
 * This interface defines the contract for all point cloud viewer implementations.
 * It enables loose coupling between the rendering logic and the rest of the application,
 * allowing for easy testing with mock implementations and future substitution
 * of different rendering engines.
 *
 * Sprint 1 Decoupling Requirements:
 * - Provides abstraction layer for 3D rendering operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Maintains compatibility with existing MainWindow interface
 * - Does not inherit from QObject to avoid multiple inheritance issues
 */
class IPointCloudViewer {
public:
    virtual ~IPointCloudViewer() = default;

    // --- Data Management ---

    /**
     * @brief Load point cloud data for rendering
     * @param points Vector of point coordinates (x,y,z interleaved)
     */
    virtual void loadPointCloud(const std::vector<float>& points) = 0;

    /**
     * @brief Clear the currently loaded point cloud
     */
    virtual void clearPointCloud() = 0;

    // --- State Management ---

    /**
     * @brief Set the viewer state and optional message
     * @param state New state for the viewer
     * @param message Optional message associated with the state
     */
    virtual void setState(ViewerState state, const QString& message = "") = 0;

    /**
     * @brief Get the current viewer state
     * @return Current viewer state
     */
    virtual ViewerState getViewerState() const = 0;

    // --- View Control ---

    /**
     * @brief Reset the camera to fit the loaded point cloud
     */
    virtual void resetCamera() = 0;

    /**
     * @brief Set the camera to top view
     */
    virtual void setTopView() = 0;

    /**
     * @brief Set the camera to front view
     */
    virtual void setFrontView() = 0;

    /**
     * @brief Set the camera to side/left view
     */
    virtual void setLeftView() = 0;

    /**
     * @brief Set the camera to right view
     */
    virtual void setRightView() = 0;

    /**
     * @brief Set the camera to bottom view
     */
    virtual void setBottomView() = 0;

    // --- Rendering Attributes & Quality ---

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
     * @brief Enable or disable splatting rendering
     * @param enabled true to enable splatting, false for point rendering
     */
    virtual void setSplattingEnabled(bool enabled) = 0;

    // --- Level of Detail (LOD) ---

    /**
     * @brief Enable or disable Level of Detail (LOD) rendering
     * @param enabled true to enable LOD, false to disable
     */
    virtual void setLODEnabled(bool enabled) = 0;

    /**
     * @brief Check if LOD is currently enabled
     * @return True if LOD is enabled, false otherwise
     */
    virtual bool isLODEnabled() const = 0;

    // --- Point Size Attenuation ---

    /**
     * @brief Enable or disable point size attenuation based on distance
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

    // --- Lighting ---

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

    // --- State & Data Query ---

    /**
     * @brief Check if point cloud data is loaded
     * @return true if data is loaded, false otherwise
     */
    virtual bool hasPointCloudData() const = 0;

    /**
     * @brief Check if point cloud data is loaded (legacy method name)
     * @return true if data is loaded, false otherwise
     */
    virtual bool hasData() const = 0;



    /**
     * @brief Get the number of points currently loaded
     * @return Number of points
     */
    virtual size_t getPointCount() const = 0;

    /**
     * @brief Get the global coordinate offset used for large coordinate handling
     * @return Global offset vector
     */
    virtual QVector3D getGlobalOffset() const = 0;

    // --- Performance Monitoring ---

    /**
     * @brief Get the current rendering frame rate
     * @return Current FPS
     */
    virtual float getCurrentFPS() const = 0;

    /**
     * @brief Get the number of visible points in the current view
     * @return Number of visible points (after LOD culling)
     */
    virtual size_t getVisiblePointCount() const = 0;

    // --- Loading Feedback Methods ---

    /**
     * @brief Method called when loading operation starts
     */
    virtual void onLoadingStarted() = 0;

    /**
     * @brief Method called during loading to report progress
     * @param percentage Progress percentage (0-100)
     * @param stage Current loading stage description
     */
    virtual void onLoadingProgress(int percentage, const QString& stage) = 0;

    /**
     * @brief Method called when loading operation finishes
     * @param success True if loading succeeded, false if failed
     * @param message Status message
     * @param points Loaded point data (if successful)
     */
    virtual void onLoadingFinished(bool success, const QString& message,
                                   const std::vector<float>& points) = 0;

    // --- LOD Control Methods ---

    /**
     * @brief Toggle LOD rendering on/off
     * @param enabled True to enable LOD, false to disable
     */
    virtual void toggleLOD(bool enabled) = 0;

    /**
     * @brief Set LOD subsample rate
     * @param rate Subsampling rate (0.0 to 1.0)
     */
    virtual void setLODSubsampleRate(float rate) = 0;

    /**
     * @brief Set screen-space error threshold for LOD
     * @param threshold Error threshold in pixels
     */
    virtual void setScreenSpaceErrorThreshold(float threshold) = 0;
};

#endif // IPOINTCLOUDVIEWER_H
