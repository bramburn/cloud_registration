#include "e57writer_lib.h"
#include <E57Format.h>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <cfloat>

E57WriterLib::E57WriterLib(QObject *parent)
    : QObject(parent)
    , m_fileOpen(false)
    , m_scanCount(0)
{
}

E57WriterLib::~E57WriterLib()
{
    if (m_fileOpen) {
        closeFile();
    }
}

bool E57WriterLib::createFile(const QString& filePath)
{
    try {
        // Close any existing file
        if (m_fileOpen) {
            closeFile();
        }

        // Validate file path
        QFileInfo fileInfo(filePath);
        QDir parentDir = fileInfo.dir();
        if (!parentDir.exists()) {
            setError(QString("Directory does not exist: %1").arg(parentDir.absolutePath()));
            return false;
        }

        // Task W1.1.1: Create ImageFile in write mode
        m_imageFile = std::make_unique<e57::ImageFile>(filePath.toStdString(), "w");
        
        if (!m_imageFile->isOpen()) {
            setError("Failed to open file handle");
            return false;
        }

        m_currentFilePath = filePath;
        m_fileOpen = true;
        m_scanCount = 0;

        // Task W1.1.1: Setup basic E57Root elements
        if (!initializeE57Root()) {
            closeFile();
            return false;
        }

        qDebug() << "E57WriterLib: Successfully created E57 file:" << filePath;
        emit fileCreated(true, filePath);
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createFile");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in createFile: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::initializeE57Root()
{
    try {
        // Task W1.1.1: Setup basic E57Root elements like formatName, guid
        e57::StructureNode rootNode = m_imageFile->root();

        // Required E57Root elements according to ASTM E2807 standard
        e57::StringNode formatNameNode(*m_imageFile, "ASTM E57 3D Imaging Data File");
        rootNode.set("formatName", formatNameNode);

        e57::StringNode guidNode(*m_imageFile, generateGUID().toStdString());
        rootNode.set("guid", guidNode);

        // Add version information (required for valid E57 files)
        e57::IntegerNode versionMajorNode(*m_imageFile, 1, 0, 255);
        rootNode.set("versionMajor", versionMajorNode);

        e57::IntegerNode versionMinorNode(*m_imageFile, 0, 0, 255);
        rootNode.set("versionMinor", versionMinorNode);

        // Add creation date/time
        QString creationDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        e57::StringNode dateTimeNode(*m_imageFile, creationDateTime.toStdString());
        rootNode.set("creationDateTime", dateTimeNode);

        // Add coordinate metadata (required for proper E57 structure)
        e57::StringNode coordinateMetadataNode(*m_imageFile, "");
        rootNode.set("coordinateMetadata", coordinateMetadataNode);

        // Create the data3D vector immediately to ensure proper file structure
        // This is critical for libE57Format to recognize the file as valid
        // Use false for allowHeteroChildren as per E57 standard (all children should be Data3D StructureNodes)
        e57::VectorNode data3DVector(*m_imageFile, false); // allowHeteroChildren = false for Data3D
        rootNode.set("data3D", data3DVector);

        qDebug() << "E57WriterLib: Initialized E57Root with required elements and data3D vector";
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "initializeE57Root");
        return false;
    }
}

bool E57WriterLib::addScan(const QString& scanName)
{
    if (!m_fileOpen) {
        setError("No file is currently open for writing");
        return false;
    }

    try {
        // Task W1.2.1: Get the E57Root node
        e57::StructureNode rootNode = m_imageFile->root();

        // Task W1.2.2: Create /data3D VectorNode if it doesn't exist
        if (!createData3DVectorNode()) {
            return false;
        }

        // Task W1.2.3: Create a new StructureNode for this scan
        if (!createScanStructureNode(scanName)) {
            return false;
        }

        m_scanCount++;
        qDebug() << "E57WriterLib: Successfully added scan:" << scanName;
        emit scanAdded(true, scanName);
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "addScan");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in addScan: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::createData3DVectorNode()
{
    try {
        e57::StructureNode rootNode = m_imageFile->root();

        // The data3D vector should already exist from initializeE57Root()
        if (rootNode.isDefined("data3D")) {
            // Get existing data3D node
            e57::VectorNode existingNode = static_cast<e57::VectorNode>(rootNode.get("data3D"));
            m_data3DNode = std::make_shared<e57::VectorNode>(existingNode);
            qDebug() << "E57WriterLib: Using existing /data3D VectorNode";
        } else {
            // This should not happen if initializeE57Root() was called correctly
            setError("data3D vector not found - initializeE57Root() may have failed");
            return false;
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createData3DVectorNode");
        return false;
    }
}

bool E57WriterLib::createScanStructureNode(const QString& scanName)
{
    try {
        // Task W1.2.3: Create a new StructureNode for the scan header
        e57::StructureNode scanHeaderNode(*m_imageFile);

        // Task W1.2.4: Populate the Data3D StructureNode with mandatory header elements
        e57::StringNode guidNode(*m_imageFile, generateGUID().toStdString());
        scanHeaderNode.set("guid", guidNode);

        e57::StringNode nameNode(*m_imageFile, scanName.toStdString());
        scanHeaderNode.set("name", nameNode);

        // Add the scan to the /data3D vector
        m_data3DNode->append(scanHeaderNode);

        // Store reference for later use (e.g., adding points)
        m_currentScanNode = std::make_shared<e57::StructureNode>(scanHeaderNode);

        qDebug() << "E57WriterLib: Created scan structure node with name:" << scanName;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createScanStructureNode");
        return false;
    }
}

QString E57WriterLib::generateGUID() const
{
    // Generate a new UUID in the format expected by E57
    QUuid uuid = QUuid::createUuid();
    return uuid.toString(); // Returns format like "{12345678-1234-1234-1234-123456789abc}"
}

bool E57WriterLib::defineXYZPrototype()
{
    if (!m_fileOpen) {
        setError("No file is currently open for writing");
        return false;
    }

    if (!m_currentScanNode) {
        setError("No scan has been added yet. Call addScan() first.");
        return false;
    }

    try {
        // Task W1.3.2: Create a StructureNode to serve as the prototype
        e57::StructureNode prototypeStructure(*m_imageFile);

        // Task W1.3.3: Add three FloatNode children for XYZ coordinates
        e57::FloatNode xNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianX", xNode);

        e57::FloatNode yNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianY", yNode);

        e57::FloatNode zNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianZ", zNode);

        // Task W1.3.1: Create the required 'codecs' VectorNode for the CompressedVectorNode
        // Even if empty for uncompressed data, it's often expected by the E57 standard
        e57::VectorNode codecsNode(*m_imageFile, true); // allowHeterogeneousChildren = true for codecs

        // Task W1.3.1: Create a proper CompressedVectorNode with the prototype and codecs
        // This is required for libE57Format to recognize the file as valid and write to disk
        e57::CompressedVectorNode pointsNode(*m_imageFile, prototypeStructure, codecsNode);

        // Add the points CompressedVectorNode to the scan
        m_currentScanNode->set("points", pointsNode);

        qDebug() << "E57WriterLib: Successfully defined XYZ prototype and created CompressedVectorNode for current scan";
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "defineXYZPrototype");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in defineXYZPrototype: %1").arg(ex.what()));
        return false;
    }
}



bool E57WriterLib::closeFile()
{
    if (!m_fileOpen) {
        return true; // Already closed
    }

    try {
        if (m_imageFile) {
            // Ensure the file structure is properly finalized before closing
            // This is critical for libE57Format to write data to disk

            // The data3D vector should already exist from initializeE57Root()
            // Just close the file - this triggers the actual write to disk
            m_imageFile->close();
            m_imageFile.reset();
        }

        m_fileOpen = false;
        m_currentScanNode.reset();
        m_data3DNode.reset();

        qDebug() << "E57WriterLib: Successfully closed E57 file:" << m_currentFilePath;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "closeFile");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in closeFile: %1").arg(ex.what()));
        return false;
    }
}

QString E57WriterLib::getLastError() const
{
    return m_lastError;
}

bool E57WriterLib::isFileOpen() const
{
    return m_fileOpen;
}

QString E57WriterLib::getCurrentFilePath() const
{
    return m_currentFilePath;
}

void E57WriterLib::setError(const QString& errorMessage)
{
    m_lastError = errorMessage;
    qWarning() << "E57WriterLib Error:" << errorMessage;
    emit errorOccurred(errorMessage);
}

void E57WriterLib::handleE57Exception(const std::exception& ex, const QString& context)
{
    QString errorMsg = QString("E57 Exception in %1: %2").arg(context, ex.what());
    setError(errorMsg);
}
