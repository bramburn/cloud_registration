#ifndef E57WRITER_LIB_NOQT_H
#define E57WRITER_LIB_NOQT_H

#include <string>
#include <memory>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class StructureNode;
    class VectorNode;
    class CompressedVectorNode;
}

/**
 * @brief E57WriterLibNoQt - Non-Qt version of E57WriterLib to fix hanging issues
 *
 * This class provides the same functionality as E57WriterLib but without Qt dependencies
 * that cause hanging issues when used with libE57Format. Uses std::string instead of QString.
 * Implements Sprint W1 requirements for E57 file creation with basic structure writing.
 */
class E57WriterLibNoQt {
public:
    explicit E57WriterLibNoQt();
    ~E57WriterLibNoQt();

    /**
     * @brief Create and initialize a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     * 
     * User Story W1.1: Initialize E57 File for Writing
     * Creates an e57::ImageFile in write mode and sets up basic E57Root elements
     */
    bool createFile(const std::string& filePath);

    /**
     * @brief Add a scan to the E57 file with basic header information
     * @param scanName Name for the scan (default: "Default Scan 001")
     * @return true if scan added successfully, false otherwise
     * 
     * User Story W1.2: Define Core E57 XML Structure for a Single Scan
     * Creates /data3D VectorNode and adds a Data3D StructureNode with basic metadata
     */
    bool addScan(const std::string& scanName = "Default Scan 001");

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
     * @return Last error message as std::string
     */
    std::string getLastError() const;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    bool isFileOpen() const;

    /**
     * @brief Get the current file path
     * @return Current file path as std::string
     */
    std::string getCurrentFilePath() const;

private:
    // Core E57 writing functionality
    bool initializeE57Root();
    bool createData3DVectorNode();
    bool createScanStructureNode(const std::string& scanName);
    std::string generateGUID() const;

    // Error handling
    void setError(const std::string& errorMessage);
    void handleE57Exception(const std::exception& ex, const std::string& context);

    // Data members
    std::unique_ptr<e57::ImageFile> m_imageFile;
    std::string m_currentFilePath;
    std::string m_lastError;
    bool m_fileOpen;
    int m_scanCount;

    // Current scan tracking
    std::shared_ptr<e57::StructureNode> m_currentScanNode;
    std::shared_ptr<e57::VectorNode> m_data3DNode;
};

#endif // E57WRITER_LIB_NOQT_H
