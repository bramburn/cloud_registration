#ifndef IE57PARSER_H
#define IE57PARSER_H

#include <QObject>
#include <QString>

#include <memory>
#include <string>
#include <vector>

/**
 * @brief IE57Parser - Abstract interface for E57 file parsing
 *
 * This interface defines the contract for all E57 parser implementations.
 * It enables loose coupling between the parsing logic and the rest of the application,
 * allowing for easy testing with mock implementations and future substitution
 * of different E57 parsing libraries.
 *
 * Sprint 1 Decoupling Requirements:
 * - Provides abstraction layer for E57 parsing operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Maintains compatibility with existing MainWindow interface
 */
class IE57Parser : public QObject
{
    Q_OBJECT

public:
    // Point data structure for enhanced parsing
    struct PointData
    {
        double x = 0.0, y = 0.0, z = 0.0;
        float intensity = 0.0f;
        uint8_t r = 0, g = 0, b = 0;
        bool hasIntensity = false;
        bool hasColor = false;
        bool isValid = true;

        // Default constructor
        PointData() = default;

        // Constructor with XYZ coordinates
        PointData(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    };

    // Loading settings structure
    struct LoadingSettings
    {
        bool loadIntensity = true;
        bool loadColor = true;
        int maxPointsPerScan = -1;      // -1 = unlimited
        double subsamplingRatio = 1.0;  // 1.0 = no subsampling
    };

    // Scan metadata structure
    struct ScanMetadata
    {
        int index = -1;
        std::string name;
        std::string guid;
        int64_t pointCount = 0;
        bool isLoaded = false;
        bool hasIntensity = false;
        bool hasColor = false;
    };

public:
    explicit IE57Parser(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IE57Parser() = default;

    // Main entry point for MainWindow integration
    virtual void startParsing(const QString& filePath, const LoadingSettings& settings = LoadingSettings()) = 0;

    // Thread-safe cancellation
    virtual void cancelParsing() = 0;

    // Error reporting
    virtual QString getLastError() const = 0;

    // Utility methods for MainWindow
    virtual bool isValidE57File(const QString& filePath) = 0;
    virtual int getScanCount(const QString& filePath) = 0;

    /**
     * @brief Open an E57 file for reading
     * @param filePath Path to the E57 file
     * @return true if file opened successfully, false otherwise
     */
    virtual bool openFile(const std::string& filePath) = 0;

    /**
     * @brief Close the currently opened E57 file
     */
    virtual void closeFile() = 0;

    /**
     * @brief Check if a file is open
     * @return true if file is open, false otherwise
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Get the GUID of the opened E57 file
     * @return File GUID as string, empty if not available
     */
    virtual std::string getGuid() const = 0;

    /**
     * @brief Get the E57 standard version of the opened file
     * @return Pair of (major, minor) version numbers
     */
    virtual std::pair<int, int> getVersion() const = 0;

    /**
     * @brief Get the number of scans (Data3D sections) in the file
     * @return Number of scans, 0 if none or file not open
     */
    virtual int getScanCount() const = 0;

    /**
     * @brief Get metadata for a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Scan metadata structure
     */
    virtual ScanMetadata getScanMetadata(int scanIndex) const = 0;

    /**
     * @brief Extract XYZ point data from the first scan (legacy method)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    virtual std::vector<float> extractPointData() = 0;

    /**
     * @brief Extract XYZ point data from a specific scan (legacy method)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    virtual std::vector<float> extractPointData(int scanIndex) = 0;

    /**
     * @brief Extract enhanced point data with intensity and color
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of PointData structures with all available attributes
     */
    virtual std::vector<PointData> extractEnhancedPointData(int scanIndex = 0) = 0;

    /**
     * @brief Get the number of points in a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Number of points, 0 if scan doesn't exist or error
     */
    virtual int64_t getPointCount(int scanIndex = 0) const = 0;

signals:
    // MainWindow-compatible signals matching old E57Parser interface
    void progressUpdated(int percentage, const QString& stage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

    // Additional signals for enhanced functionality
    void scanMetadataAvailable(int scanCount, const QStringList& scanNames);
    void intensityDataExtracted(const std::vector<float>& intensityValues);
    void colorDataExtracted(const std::vector<uint8_t>& colorValues);  // RGB interleaved
};

#endif  // IE57PARSER_H
