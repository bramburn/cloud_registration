#ifndef IPOINTCLOUDPROCESSOR_H
#define IPOINTCLOUDPROCESSOR_H

#include <QString>

#include <memory>
#include <vector>

/**
 * @brief Modern C++ interface for point cloud processing operations
 *
 * This interface demonstrates the modern C++ interface pattern recommended
 * in the cmarkguide.md document. It follows best practices:
 * - Pure virtual functions only
 * - Virtual destructor for proper cleanup
 * - Clear contract definition
 * - Enables dependency injection and testing
 */
class IPointCloudProcessor
{
public:
    /**
     * @brief Point data structure for processing
     */
    struct Point3D
    {
        float x, y, z;
        float intensity = 0.0f;
        bool hasIntensity = false;

        Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
        Point3D(float x_, float y_, float z_, float intensity_)
            : x(x_), y(y_), z(z_), intensity(intensity_), hasIntensity(true)
        {
        }
    };

    /**
     * @brief Processing result structure
     */
    struct ProcessingResult
    {
        std::vector<Point3D> processedPoints;
        size_t originalPointCount = 0;
        size_t processedPointCount = 0;
        double processingTimeSeconds = 0.0;
        bool success = false;
        QString errorMessage;
    };

public:
    /**
     * @brief Virtual destructor ensures proper cleanup of derived classes
     */
    virtual ~IPointCloudProcessor() = default;

    /**
     * @brief Process a point cloud with the specified algorithm
     * @param points Input point cloud data
     * @param parameters Algorithm-specific parameters
     * @return Processing result with output data and metrics
     */
    virtual ProcessingResult processPointCloud(const std::vector<Point3D>& points,
                                               const QString& parameters = QString()) = 0;

    /**
     * @brief Get the name/type of this processor
     * @return Human-readable processor name
     */
    virtual QString getProcessorName() const = 0;

    /**
     * @brief Check if the processor supports the given point cloud size
     * @param pointCount Number of points to process
     * @return true if the processor can handle this size
     */
    virtual bool supportsPointCount(size_t pointCount) const = 0;

    /**
     * @brief Get the recommended parameters for this processor
     * @return JSON-formatted parameter description
     */
    virtual QString getRecommendedParameters() const = 0;

    /**
     * @brief Validate parameters before processing
     * @param parameters Parameters to validate
     * @return true if parameters are valid
     */
    virtual bool validateParameters(const QString& parameters) const = 0;

    /**
     * @brief Get processing statistics from the last operation
     * @return Statistics as key-value pairs
     */
    virtual std::vector<std::pair<QString, QString>> getLastProcessingStats() const = 0;
};

/**
 * @brief Factory interface for creating point cloud processors
 *
 * This demonstrates the Factory pattern combined with modern interfaces
 */
class IPointCloudProcessorFactory
{
public:
    virtual ~IPointCloudProcessorFactory() = default;

    /**
     * @brief Create a processor instance by name
     * @param processorType Type of processor to create
     * @return Unique pointer to the processor, or nullptr if type not supported
     */
    virtual std::unique_ptr<IPointCloudProcessor> createProcessor(const QString& processorType) = 0;

    /**
     * @brief Get list of supported processor types
     * @return List of processor type names
     */
    virtual std::vector<QString> getSupportedProcessorTypes() const = 0;
};

#endif  // IPOINTCLOUDPROCESSOR_H
