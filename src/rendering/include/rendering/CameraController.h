#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QMatrix4x4>
#include <QObject>
#include <QPoint>
#include <QVector3D>

#include <cmath>

/**
 * @brief CameraController - Camera navigation and view matrix management
 *
 * This class centralizes all camera-related logic, providing a clean interface
 * for manipulating the 3D view. This separation of concerns simplifies the
 * viewer widget's responsibilities.
 *
 * Sprint 1 Requirements:
 * - Orbit, pan, and zoom operations
 * - View and projection matrix management
 * - Camera position, target, and up-vector handling
 * - Field of view control
 * - Smooth camera movements and constraints
 */
class CameraController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit CameraController(QObject* parent = nullptr);

    /**
     * @brief Perform orbit operation around target
     * @param deltaX Horizontal rotation delta (degrees)
     * @param deltaY Vertical rotation delta (degrees)
     */
    virtual void orbit(float deltaX, float deltaY);

    /**
     * @brief Perform pan operation (move camera and target together)
     * @param deltaX Horizontal pan delta
     * @param deltaY Vertical pan delta
     */
    virtual void pan(float deltaX, float deltaY);

    /**
     * @brief Perform zoom operation (move camera closer/farther from target)
     * @param delta Zoom delta (positive = zoom in, negative = zoom out)
     */
    virtual void zoom(float delta);

    /**
     * @brief Fit camera view to show entire bounding box
     * @param minBounds Minimum bounds of the data
     * @param maxBounds Maximum bounds of the data
     */
    void fitToView(const QVector3D& minBounds, const QVector3D& maxBounds);

    /**
     * @brief Reset camera to default position
     */
    void reset();

    /**
     * @brief Set camera to top view
     */
    void setTopView();

    /**
     * @brief Set camera to front view
     */
    void setFrontView();

    /**
     * @brief Set camera to side view
     */
    void setSideView();

    /**
     * @brief Set camera to isometric view
     */
    void setIsometricView();

    /**
     * @brief Get current view matrix
     * @return View matrix
     */
    QMatrix4x4 getViewMatrix() const;

    /**
     * @brief Get projection matrix
     * @param aspectRatio Viewport aspect ratio
     * @return Projection matrix
     */
    QMatrix4x4 getProjectionMatrix(float aspectRatio) const;

    /**
     * @brief Get camera position in world space
     * @return Camera position
     */
    QVector3D getCameraPosition() const
    {
        return m_position;
    }

    /**
     * @brief Get camera target position
     * @return Target position
     */
    QVector3D getCameraTarget() const
    {
        return m_target;
    }

    /**
     * @brief Get camera up vector
     * @return Up vector
     */
    QVector3D getCameraUp() const
    {
        return m_up;
    }

    /**
     * @brief Get field of view
     * @return Field of view in degrees
     */
    float getFieldOfView() const
    {
        return m_fov;
    }

    /**
     * @brief Set field of view
     * @param fov Field of view in degrees
     */
    void setFieldOfView(float fov);

    /**
     * @brief Get camera distance from target
     * @return Distance
     */
    float getDistance() const
    {
        return m_distance;
    }

    /**
     * @brief Set camera distance from target
     * @param distance New distance
     */
    void setDistance(float distance);

    /**
     * @brief Set zoom constraints
     * @param minDistance Minimum zoom distance
     * @param maxDistance Maximum zoom distance
     */
    void setZoomConstraints(float minDistance, float maxDistance);

    /**
     * @brief Set pan sensitivity
     * @param sensitivity Pan sensitivity factor
     */
    void setPanSensitivity(float sensitivity)
    {
        m_panSensitivity = sensitivity;
    }

    /**
     * @brief Set orbit sensitivity
     * @param sensitivity Orbit sensitivity factor
     */
    void setOrbitSensitivity(float sensitivity)
    {
        m_orbitSensitivity = sensitivity;
    }

    /**
     * @brief Set zoom sensitivity
     * @param sensitivity Zoom sensitivity factor
     */
    void setZoomSensitivity(float sensitivity)
    {
        m_zoomSensitivity = sensitivity;
    }

signals:
    /**
     * @brief Emitted when camera parameters change
     */
    void cameraChanged();

protected:
    // Camera parameters
    QVector3D m_position;
    QVector3D m_target;
    QVector3D m_up;
    float m_distance;
    float m_fov;

    // Spherical coordinates for orbit
    float m_azimuth;    // Horizontal rotation
    float m_elevation;  // Vertical rotation

    // Constraints
    float m_minDistance;
    float m_maxDistance;
    float m_minElevation;
    float m_maxElevation;

    // Sensitivity settings
    float m_panSensitivity;
    float m_orbitSensitivity;
    float m_zoomSensitivity;

    // Helper methods
    void updateCameraPosition();
    void constrainElevation();
    void constrainDistance();
    QVector3D calculateCameraPosition() const;
};

#endif  // CAMERACONTROLLER_H
