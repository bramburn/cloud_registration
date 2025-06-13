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
#include "interfaces/IE57Parser.h"
#include "E57ParserCore.h"

// Forward declarations for E57Format types
namespace e57 {
    class ImageFile;
    class StructureNode;
}

/**
 * @brief E57ParserLib - Qt adapter for E57ParserCore
 *
 * This class provides a Qt-compatible interface to the E57ParserCore library.
 * It acts as a thin adapter that wraps the core parsing functionality and
 * provides Qt signals/slots integration for the main application.
 * Implements Sprint 1-5 requirements for E57 library integration with MainWindow compatibility.
 *
 * Sprint 1 Decoupling: Now implements IE57Parser interface for loose coupling.
 * Sprint 2 Decoupling: Refactored to delegate core parsing to E57ParserCore.
 */
class E57ParserLib : public IE57Parser {
    Q_OBJECT

public:
    explicit E57ParserLib(QObject *parent = nullptr);
    ~E57ParserLib();

    // Main entry point for MainWindow integration
    void startParsing(const QString& filePath, const IE57Parser::LoadingSettings& settings = IE57Parser::LoadingSettings()) override;

    // Thread-safe cancellation
    void cancelParsing() override;

    // Error reporting
    QString getLastError() const override;

    // Utility methods for MainWindow
    bool isValidE57File(const QString& filePath) override;
    int getScanCount(const QString& filePath) override;

    /**
     * @brief Open an E57 file for reading
     * @param filePath Path to the E57 file
     * @return true if file opened successfully, false otherwise
     */
    bool openFile(const std::string& filePath) override;

    /**
     * @brief Close the currently opened E57 file
     */
    void closeFile() override;

    /**
     * @brief Get the GUID of the opened E57 file
     * @return File GUID as string, empty if not available
     */
    std::string getGuid() const override;

    /**
     * @brief Get the E57 standard version of the opened file
     * @return Pair of (major, minor) version numbers
     */
    std::pair<int, int> getVersion() const override;

    /**
     * @brief Get the number of scans (Data3D sections) in the file
     * @return Number of scans, 0 if none or file not open
     */
    int getScanCount() const override;

    // Sprint 4: Multi-scan support enhancement

    /**
     * @brief Get metadata for a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Scan metadata structure
     */
    IE57Parser::ScanMetadata getScanMetadata(int scanIndex) const override;



    /**
     * @brief Check if a file is open
     * @return true if file is open, false otherwise
     */
    bool isOpen() const override;

    // Sprint 2 & 3: Point data extraction methods

    /**
     * @brief Extract XYZ point data from the first scan (legacy method)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData() override;

    /**
     * @brief Extract XYZ point data from a specific scan (legacy method)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData(int scanIndex) override;

    /**
     * @brief Extract enhanced point data with intensity and color (Sprint 3)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of PointData structures with all available attributes
     */
    std::vector<IE57Parser::PointData> extractEnhancedPointData(int scanIndex = 0) override;

    /**
     * @brief Get the number of points in a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Number of points, 0 if scan doesn't exist or error
     */
    int64_t getPointCount(int scanIndex = 0) const override;

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
    // Qt adapter functionality
    void setupForThreading();
    std::vector<float> convertToXYZVector(const std::vector<PointData>& pointData);

    // Progress tracking
    void updateProgress(int percentage, const QString& stage);
    void onCoreProgress(int percentage, const std::string& stage);

    // Error handling
    void handleE57Exception(const std::exception& ex, const QString& context);
    QString translateE57Error(const QString& technicalError);

    // Data conversion helpers
    IE57Parser::PointData convertCorePointData(const CorePointData& corePoint);
    CoreLoadingSettings convertLoadingSettings(const IE57Parser::LoadingSettings& qtSettings);

    // Data members
    std::unique_ptr<E57ParserCore> m_parserCore;
    QString m_currentFilePath;
    IE57Parser::LoadingSettings m_currentSettings;
    mutable QString m_lastError;

    // Threading and cancellation
    std::atomic<bool> m_cancelRequested{false};
    mutable QMutex m_errorMutex;
    QTimer* m_progressTimer = nullptr;

    // Internal data storage
    std::vector<IE57Parser::PointData> m_extractedPoints;
    QStringList m_scanNames;
    int m_totalScans = 0;

    // E57 file handling (for legacy methods that bypass m_parserCore)
    std::unique_ptr<e57::ImageFile> m_imageFile;
    std::vector<float> m_points;  // Legacy point storage

    // Prototype information for parsing
    struct PrototypeInfo {
        bool hasCartesianX = false;
        bool hasCartesianY = false;
        bool hasCartesianZ = false;
        bool isDoublePrec = false;
        bool hasIntensity = false;
        bool hasColorRed = false;
        bool hasColorGreen = false;
        bool hasColorBlue = false;
        std::string intensityDataType;
        std::string colorDataType;

        void reset() {
            hasCartesianX = hasCartesianY = hasCartesianZ = false;
            isDoublePrec = false;
            hasIntensity = false;
            hasColorRed = hasColorGreen = hasColorBlue = false;
            intensityDataType.clear();
            colorDataType.clear();
        }
    } m_prototypeInfo;

    // Data limits for normalization
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

        void reset() {
            intensityMin = 0.0;
            intensityMax = 1.0;
            colorRedMin = colorGreenMin = colorBlueMin = 0.0;
            colorRedMax = colorGreenMax = colorBlueMax = 255.0;
            hasIntensityLimits = hasColorLimits = false;
        }
    } m_dataLimits;

    // Helper methods
    void clearError() const;
    void setError(const std::string& error) const;

    // Legacy parsing methods (to be refactored)
    bool inspectPointPrototype(const e57::StructureNode& scanHeaderNode);
    void validatePrototypeFields(const e57::StructureNode& prototype);
    bool extractUncompressedXYZData(const e57::StructureNode& scanHeaderNode);

    // Enhanced parsing methods
    bool inspectEnhancedPrototype(const e57::StructureNode& scanHeaderNode);
    bool extractDataLimits(const e57::StructureNode& scanHeaderNode);
    bool extractEnhancedPointData(const e57::StructureNode& scanHeaderNode, std::vector<IE57Parser::PointData>& points);

    // Normalization helpers
    float normalizeIntensity(float rawValue) const;
    uint8_t normalizeColorChannel(float rawValue, double minVal, double maxVal) const;
};

#endif // E57PARSERLIB_H
