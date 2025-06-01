#ifndef E57PARSERLIB_H
#define E57PARSERLIB_H

#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QTimer>
#include <QThread>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class StructureNode;
    class CompressedVectorNode;
}

/**
 * @brief E57ParserLib - A wrapper class for libE57Format library
 *
 * This class provides a simplified interface to the libE57Format library
 * for opening E57 files, extracting metadata, and reading point cloud data.
 * Implements Sprint 1-5 requirements for E57 library integration with MainWindow compatibility.
 */
class E57ParserLib : public QObject {
    Q_OBJECT

public:
    struct PointData {
        double x, y, z;
        float intensity = 0.0f;
        uint8_t r = 255, g = 255, b = 255;
        bool hasIntensity = false;
        bool hasColor = false;
        bool isValid = true;
    };

    struct LoadingSettings {
        bool loadIntensity = true;
        bool loadColor = true;
        int maxPointsPerScan = -1;  // -1 = unlimited
        double subsamplingRatio = 1.0;  // 1.0 = no subsampling
    };

public:
    explicit E57ParserLib(QObject *parent = nullptr);
    ~E57ParserLib();

    // Main entry point for MainWindow integration
    void startParsing(const QString& filePath, const LoadingSettings& settings = LoadingSettings());

    // Thread-safe cancellation
    void cancelParsing();

    // Error reporting
    QString getLastError() const;

    // Utility methods for MainWindow
    bool isValidE57File(const QString& filePath);
    int getScanCount(const QString& filePath);

    /**
     * @brief Open an E57 file for reading
     * @param filePath Path to the E57 file
     * @return true if file opened successfully, false otherwise
     */
    bool openFile(const std::string& filePath);

    /**
     * @brief Close the currently opened E57 file
     */
    void closeFile();

    /**
     * @brief Get the GUID of the opened E57 file
     * @return File GUID as string, empty if not available
     */
    std::string getGuid() const;

    /**
     * @brief Get the E57 standard version of the opened file
     * @return Pair of (major, minor) version numbers
     */
    std::pair<int, int> getVersion() const;

    /**
     * @brief Get the number of scans (Data3D sections) in the file
     * @return Number of scans, 0 if none or file not open
     */
    int getScanCount() const;

    // Sprint 4: Multi-scan support enhancement

    /**
     * @brief Get metadata for a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Scan metadata structure
     */
    struct ScanMetadata {
        int index = -1;
        std::string name;
        std::string guid;
        int64_t pointCount = 0;
        bool isLoaded = false;
        bool hasIntensity = false;
        bool hasColor = false;
    };

    ScanMetadata getScanMetadata(int scanIndex) const;



    /**
     * @brief Check if a file is open
     * @return true if file is open, false otherwise
     */
    bool isOpen() const;

    // Sprint 2 & 3: Point data extraction methods

    /**
     * @brief Extract XYZ point data from the first scan (legacy method)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData();

    /**
     * @brief Extract XYZ point data from a specific scan (legacy method)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData(int scanIndex);

    /**
     * @brief Extract enhanced point data with intensity and color (Sprint 3)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of PointData structures with all available attributes
     */
    std::vector<PointData> extractEnhancedPointData(int scanIndex = 0);

    /**
     * @brief Get the number of points in a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Number of points, 0 if scan doesn't exist or error
     */
    int64_t getPointCount(int scanIndex = 0) const;

signals:
    // MainWindow-compatible signals matching old E57Parser interface
    void progressUpdated(int percentage, const QString& stage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

    // Additional signals for enhanced functionality
    void scanMetadataAvailable(int scanCount, const QStringList& scanNames);
    void intensityDataExtracted(const std::vector<float>& intensityValues);
    void colorDataExtracted(const std::vector<uint8_t>& colorValues); // RGB interleaved

private slots:
    void performParsing();

private:
    // Core parsing functionality
    bool openE57File(const QString& filePath);
    void closeE57File();
    std::vector<PointData> extractPointDataFromScan(int scanIndex, const LoadingSettings& settings);
    std::vector<float> convertToXYZVector(const std::vector<PointData>& pointData);

    // Progress tracking
    void updateProgress(int percentage, const QString& stage);

    // Error handling
    void handleE57Exception(const std::exception& ex, const QString& context);
    QString translateE57Error(const QString& technicalError);

    // Threading support
    void setupForThreading();

    // Data members
    std::unique_ptr<e57::ImageFile> m_imageFile;
    QString m_currentFilePath;
    LoadingSettings m_currentSettings;
    QString m_lastError;

    // Threading and cancellation
    std::atomic<bool> m_cancelRequested{false};
    mutable QMutex m_errorMutex;
    QTimer* m_progressTimer = nullptr;

    // Internal data storage
    std::vector<PointData> m_extractedPoints;
    QStringList m_scanNames;
    int m_totalScans = 0;

    // Sprint 2: Point data storage and metadata
    std::vector<float> m_points;

    // Prototype information for current scan (enhanced for Sprint 3)
    struct PrototypeInfo {
        // XYZ coordinates (Sprint 2)
        bool hasCartesianX = false;
        bool hasCartesianY = false;
        bool hasCartesianZ = false;
        bool isDoublePrec = false;
        std::string xFieldName = "cartesianX";
        std::string yFieldName = "cartesianY";
        std::string zFieldName = "cartesianZ";

        // Intensity data (Sprint 3)
        bool hasIntensity = false;
        std::string intensityFieldName = "intensity";
        std::string intensityDataType; // "float", "integer", "scaledInteger"

        // Color data (Sprint 3)
        bool hasColorRed = false;
        bool hasColorGreen = false;
        bool hasColorBlue = false;
        std::string colorRedFieldName = "colorRed";
        std::string colorGreenFieldName = "colorGreen";
        std::string colorBlueFieldName = "colorBlue";
        std::string colorDataType; // "integer", "scaledInteger"
    };
    PrototypeInfo m_prototypeInfo;

    // Sprint 3: Intensity and color limits for normalization
    struct DataLimits {
        double intensityMin = 0.0;
        double intensityMax = 1.0;
        double colorRedMin = 0.0;
        double colorRedMax = 255.0;
        double colorGreenMin = 0.0;
        double colorGreenMax = 255.0;
        double colorBlueMin = 0.0;
        double colorBlueMax = 255.0;
        bool hasIntensityLimits = false;
        bool hasColorLimits = false;
    };
    DataLimits m_dataLimits;

    // Helper methods
    void clearError();
    void setError(const std::string& error);

    // Sprint 2: Point data extraction helpers
    bool accessFirstScanData(e57::StructureNode& scanHeader);
    bool inspectPointPrototype(const e57::StructureNode& scanHeader);
    bool extractUncompressedXYZData(const e57::StructureNode& scanHeader);
    void validatePrototypeFields(const e57::StructureNode& prototype);

    // Sprint 3: Enhanced point data extraction helpers
    bool inspectEnhancedPrototype(const e57::StructureNode& scanHeader);
    bool extractDataLimits(const e57::StructureNode& scanHeader);
    bool extractEnhancedPointData(const e57::StructureNode& scanHeader, std::vector<PointData>& points);

    // Sprint 3: Normalization helpers
    float normalizeIntensity(float rawValue) const;
    uint8_t normalizeColorChannel(float rawValue, double minVal, double maxVal) const;
};

#endif // E57PARSERLIB_H
