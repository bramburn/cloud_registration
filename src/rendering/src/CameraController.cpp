#include "rendering/CameraController.h"
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CameraController::CameraController(QObject* parent)
    : QObject(parent)
    , m_position(0.0f, 0.0f, 5.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_distance(5.0f)
    , m_fov(45.0f)
    , m_azimuth(0.0f)
    , m_elevation(0.0f)
    , m_minDistance(0.1f)
    , m_maxDistance(1000.0f)
    , m_minElevation(-89.0f)
    , m_maxElevation(89.0f)
    , m_panSensitivity(0.01f)
    , m_orbitSensitivity(0.5f)
    , m_zoomSensitivity(0.1f)
{
    updateCameraPosition();
}

void CameraController::orbit(float deltaX, float deltaY)
{
    m_azimuth += deltaX * m_orbitSensitivity;
    m_elevation += deltaY * m_orbitSensitivity;

    // Normalize azimuth to [0, 360)
    while (m_azimuth >= 360.0f) m_azimuth -= 360.0f;
    while (m_azimuth < 0.0f) m_azimuth += 360.0f;

    constrainElevation();
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::pan(float deltaX, float deltaY)
{
    // Calculate camera's right and up vectors
    QVector3D forward = (m_target - m_position).normalized();
    QVector3D right = QVector3D::crossProduct(forward, m_up).normalized();
    QVector3D up = QVector3D::crossProduct(right, forward).normalized();

    // Apply pan movement
    QVector3D panOffset = (right * deltaX + up * deltaY) * m_panSensitivity * m_distance;
    m_target += panOffset;
    m_position += panOffset;

    emit cameraChanged();
}

void CameraController::zoom(float delta)
{
    m_distance -= delta * m_zoomSensitivity * m_distance;
    constrainDistance();
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::fitToView(const QVector3D& minBounds, const QVector3D& maxBounds)
{
    // Calculate bounding box center and size
    QVector3D center = (minBounds + maxBounds) * 0.5f;
    QVector3D size = maxBounds - minBounds;
    float maxDimension = qMax(qMax(size.x(), size.y()), size.z());

    // Set target to center
    m_target = center;

    // Calculate appropriate distance based on field of view
    float distance = maxDimension / (2.0f * tan(m_fov * M_PI / 360.0f));
    m_distance = qMax(distance * 1.2f, m_minDistance); // Add 20% margin

    // Reset to isometric view
    m_azimuth = 45.0f;
    m_elevation = 30.0f;

    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::reset()
{
    m_position = QVector3D(0.0f, 0.0f, 5.0f);
    m_target = QVector3D(0.0f, 0.0f, 0.0f);
    m_up = QVector3D(0.0f, 1.0f, 0.0f);
    m_distance = 5.0f;
    m_azimuth = 0.0f;
    m_elevation = 0.0f;
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::setTopView()
{
    m_azimuth = 0.0f;
    m_elevation = 90.0f;
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::setFrontView()
{
    m_azimuth = 0.0f;
    m_elevation = 0.0f;
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::setSideView()
{
    m_azimuth = 90.0f;
    m_elevation = 0.0f;
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::setIsometricView()
{
    m_azimuth = 45.0f;
    m_elevation = 30.0f;
    updateCameraPosition();
    emit cameraChanged();
}

QMatrix4x4 CameraController::getViewMatrix() const
{
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(m_position, m_target, m_up);
    return viewMatrix;
}

QMatrix4x4 CameraController::getProjectionMatrix(float aspectRatio) const
{
    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(m_fov, aspectRatio, 0.1f, 1000.0f);
    return projectionMatrix;
}

void CameraController::setFieldOfView(float fov)
{
    m_fov = qBound(10.0f, fov, 120.0f);
    emit cameraChanged();
}

void CameraController::setDistance(float distance)
{
    m_distance = distance;
    constrainDistance();
    updateCameraPosition();
    emit cameraChanged();
}

void CameraController::setZoomConstraints(float minDistance, float maxDistance)
{
    m_minDistance = qMax(0.01f, minDistance);
    m_maxDistance = qMax(m_minDistance, maxDistance);
    constrainDistance();
}

void CameraController::updateCameraPosition()
{
    m_position = calculateCameraPosition();
}

void CameraController::constrainElevation()
{
    m_elevation = qBound(m_minElevation, m_elevation, m_maxElevation);
}

void CameraController::constrainDistance()
{
    m_distance = qBound(m_minDistance, m_distance, m_maxDistance);
}

QVector3D CameraController::calculateCameraPosition() const
{
    // Convert spherical coordinates to Cartesian
    float azimuthRad = m_azimuth * M_PI / 180.0f;
    float elevationRad = m_elevation * M_PI / 180.0f;

    float x = m_distance * cos(elevationRad) * cos(azimuthRad);
    float y = m_distance * sin(elevationRad);
    float z = m_distance * cos(elevationRad) * sin(azimuthRad);

    return m_target + QVector3D(x, y, z);
}
