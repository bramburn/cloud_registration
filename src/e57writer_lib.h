#ifndef E57WRITER_LIB_H
#define E57WRITER_LIB_H

#include <string>
#include <memory>
#include <QObject>
#include <QString>
#include <QUuid>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class StructureNode;
    class VectorNode;
    class CompressedVectorNode;
}

/**
 * @brief E57WriterLib - A wrapper class for libE57Format library writing capabilities
 *
 * This class provides a simplified interface to the libE57Format library
 * for creating E57 files, writing metadata, and setting up point cloud data structures.
 * Implements Sprint W1 requirements for E57 file creation with basic structure writing.
 */
class E57WriterLib : public QObject {
    Q_OBJECT

public:
    explicit E57WriterLib(QObject *parent = nullptr);
    ~E57WriterLib();

    /**
     * @brief Create and initialize a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     * 
     * User Story W1.1: Initialize E57 File for Writing
     * Creates an e57::ImageFile in write mode and sets up basic E57Root elements
     */
    bool createFile(const QString& filePath);

    /**
     * @brief Add a scan to the E57 file with basic header information
     * @param scanName Name for the scan (default: "Default Scan 001")
     * @return true if scan added successfully, false otherwise
     * 
     * User Story W1.2: Define Core E57 XML Structure for a Single Scan
     * Creates /data3D VectorNode and adds a Data3D StructureNode with basic metadata
     */
    bool addScan(const QString& scanName = "Default Scan 001");

    /**
     * @brief Define XYZ prototype for point data in the current scan
     * @return true if prototype defined successfully, false otherwise
     * 
     * User Story W1.3: Define Point Prototype for XYZ Data
     * Creates a CompressedVectorNode with prototype for cartesianX, Y, Z fields
     */
    bool defineXYZPrototype();

    /**
     * @brief Close the E57 file and finalize writing
     * @return true if file closed successfully, false otherwise
     */
    bool closeFile();

    /**
     * @brief Get the last error message
     * @return Last error message as QString
     */
    QString getLastError() const;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    bool isFileOpen() const;

    /**
     * @brief Get the current file path
     * @return Current file path as QString
     */
    QString getCurrentFilePath() const;

signals:
    /**
     * @brief Emitted when file creation is completed
     * @param success true if successful, false if failed
     * @param filePath Path of the created file
     */
    void fileCreated(bool success, const QString& filePath);

    /**
     * @brief Emitted when scan is added
     * @param success true if successful, false if failed
     * @param scanName Name of the added scan
     */
    void scanAdded(bool success, const QString& scanName);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Description of the error
     */
    void errorOccurred(const QString& errorMessage);

private:
    // Core E57 writing functionality
    bool initializeE57Root();
    bool createData3DVectorNode();
    bool createScanStructureNode(const QString& scanName);
    QString generateGUID() const;

    // Error handling
    void setError(const QString& errorMessage);
    void handleE57Exception(const std::exception& ex, const QString& context);

    // Data members
    std::unique_ptr<e57::ImageFile> m_imageFile;
    QString m_currentFilePath;
    QString m_lastError;
    bool m_fileOpen;
    int m_scanCount;

    // Current scan tracking
    std::shared_ptr<e57::StructureNode> m_currentScanNode;
    std::shared_ptr<e57::VectorNode> m_data3DNode;
};

#endif // E57WRITER_LIB_H
