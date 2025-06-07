#ifndef IPOINTCLOUDVIEWER_H
#define IPOINTCLOUDVIEWER_H

#include <QObject>
#include <QString>
#include <QVector3D>
#include <vector>

/**
 * @brief Abstract interface for 3D point cloud rendering components
 * 
 * This interface defines the contract for all 3D point cloud viewers, enabling
 * loose coupling between the application's core logic and specific rendering
 * implementations. It follows the established decoupling pattern used throughout
 * the CloudRegistration application.
 * 
 * Sprint 3 Decoupling Requirements:
 * - Provides abstraction layer for 3D rendering operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Maintains compatibility with existing MainWindow interface
 * - Follows the same pattern as IPointCloudProcessor and IE57Parser
 */

/**
 * @brief Enum defining the possible states of the point cloud viewer
 */
enum class ViewerState {
    Idle,           // No data loaded, ready for input
    Loading,        // Currently loading point cloud data
    DisplayingData, // Successfully displaying point cloud data
    LoadFailed      // Failed to load or display data
};

/**
 * @brief Abstract interface for point cloud viewer components
 * 
 * This interface abstracts the essential operations needed to control a 3D
 * point cloud viewer from the application's main logic. It enables testing
 * with mock implementations and future integration of alternative rendering
 * technologies.
 */
class IPointCloudViewer : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Virtual destructor ensures proper cleanup of derived classes
     */
    virtual ~IPointCloudViewer() = default;

    // Core data operations
    /**
     * @brief Load point cloud data into the viewer
     * @param points Vector of point coordinates (x,y,z interleaved)
     */
    virtual void loadPointCloud(const std::vector<float>& points) = 0;

    /**
     * @brief Clear all point cloud data from the viewer
     */
    virtual void clearPointCloud() = 0;

    /**
     * @brief Set the current state of the viewer with optional message
     * @param state The new viewer state
     * @param message Optional status message
     */
    virtual void setState(ViewerState state, const QString& message = "") = 0;

    // View control operations
    /**
     * @brief Set the camera to top view
     */
    virtual void setTopView() = 0;

    /**
     * @brief Set the camera to left view
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

    // Level of Detail (LOD) control
    /**
     * @brief Enable or disable Level of Detail rendering
     * @param enabled True to enable LOD, false to disable
     */
    virtual void setLODEnabled(bool enabled) = 0;

    /**
     * @brief Check if LOD is currently enabled
     * @return True if LOD is enabled, false otherwise
     */
    virtual bool isLODEnabled() const = 0;

    // Rendering attribute controls
    /**
     * @brief Enable or disable color-based rendering
     * @param enabled True to render with colors, false for default coloring
     */
    virtual void setRenderWithColor(bool enabled) = 0;

    /**
     * @brief Enable or disable intensity-based rendering
     * @param enabled True to render with intensity values, false for default
     */
    virtual void setRenderWithIntensity(bool enabled) = 0;

    /**
     * @brief Enable or disable point size attenuation
     * @param enabled True to enable distance-based point size attenuation
     */
    virtual void setPointSizeAttenuationEnabled(bool enabled) = 0;

    /**
     * @brief Set point size attenuation parameters
     * @param minSize Minimum point size
     * @param maxSize Maximum point size
     * @param factor Attenuation factor
     */
    virtual void setPointSizeAttenuationParams(float minSize, float maxSize, float factor) = 0;

    // State query methods (for testing and debugging)
    /**
     * @brief Get the current viewer state
     * @return Current ViewerState
     */
    virtual ViewerState getViewerState() const = 0;

    /**
     * @brief Check if the viewer has point cloud data loaded
     * @return True if data is loaded, false otherwise
     */
    virtual bool hasPointCloudData() const = 0;

    /**
     * @brief Get the number of points currently loaded
     * @return Number of points in the current point cloud
     */
    virtual size_t getPointCount() const = 0;

    // Coordinate transformation access (for advanced operations)
    /**
     * @brief Get the global coordinate offset used for large coordinate handling
     * @return Global offset vector
     */
    virtual QVector3D getGlobalOffset() const = 0;

    // Performance monitoring
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

public slots:
    // Loading feedback slots
    /**
     * @brief Slot called when loading operation starts
     */
    virtual void onLoadingStarted() = 0;

    /**
     * @brief Slot called during loading to report progress
     * @param percentage Progress percentage (0-100)
     * @param stage Current loading stage description
     */
    virtual void onLoadingProgress(int percentage, const QString& stage) = 0;

    /**
     * @brief Slot called when loading operation finishes
     * @param success True if loading succeeded, false if failed
     * @param message Status message
     * @param points Loaded point data (if successful)
     */
    virtual void onLoadingFinished(bool success, const QString& message,
                                  const std::vector<float>& points) = 0;

    // LOD control slots
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

    // Screen-space error LOD control slots
    /**
     * @brief Set screen-space error threshold for LOD
     * @param threshold Error threshold in pixels
     */
    virtual void setScreenSpaceErrorThreshold(float threshold) = 0;

    /**
     * @brief Set primary screen-space error threshold
     * @param threshold Primary threshold in pixels
     */
    virtual void setPrimaryScreenSpaceErrorThreshold(float threshold) = 0;

    /**
     * @brief Set culling screen-space error threshold
     * @param threshold Culling threshold in pixels
     */
    virtual void setCullScreenSpaceErrorThreshold(float threshold) = 0;

signals:
    // Performance monitoring signals
    /**
     * @brief Emitted when rendering statistics are updated
     * @param fps Current frame rate
     * @param visiblePoints Number of visible points
     */
    void statsUpdated(float fps, int visiblePoints);

    /**
     * @brief Emitted when a rendering error occurs
     * @param error Error description
     */
    void renderingError(const QString& error);

    // Data operation signals
    /**
     * @brief Emitted when point cloud data is successfully loaded
     * @param points The loaded point data
     */
    void pointCloudLoaded(const std::vector<float>& points);

    /**
     * @brief Emitted when point cloud loading fails
     * @param error Error description
     */
    void pointCloudLoadFailed(const QString& error);

    /**
     * @brief Emitted when point cloud data is cleared
     */
    void pointCloudCleared();

    /**
     * @brief Emitted when the viewer state changes
     * @param newState The new viewer state
     * @param message Optional status message
     */
    void stateChanged(ViewerState newState, const QString& message);
};

#endif // IPOINTCLOUDVIEWER_H
