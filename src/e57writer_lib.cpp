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

        e57::StringNode formatNameNode(*m_imageFile, "ASTM E57 3D Imaging Data File");
        rootNode.set("formatName", formatNameNode);

        e57::StringNode guidNode(*m_imageFile, generateGUID().toStdString());
        rootNode.set("guid", guidNode);

        // Add creation date/time
        QString creationDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        e57::StringNode dateTimeNode(*m_imageFile, creationDateTime.toStdString());
        rootNode.set("creationDateTime", dateTimeNode);

        qDebug() << "E57WriterLib: Initialized E57Root with formatName and GUID";
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

        // Check if /data3D already exists
        if (rootNode.isDefined("data3D")) {
            // Get existing data3D node
            e57::VectorNode existingNode = static_cast<e57::VectorNode>(rootNode.get("data3D"));
            m_data3DNode = std::make_shared<e57::VectorNode>(existingNode);
            qDebug() << "E57WriterLib: Using existing /data3D VectorNode";
        } else {
            // Task W1.2.2: Create new /data3D VectorNode
            e57::VectorNode data3DNode(*m_imageFile, false); // false for allowHeterogeneousChildren
            rootNode.set("data3D", data3DNode);
            m_data3DNode = std::make_shared<e57::VectorNode>(data3DNode);
            qDebug() << "E57WriterLib: Created new /data3D VectorNode";
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
        // For now, let's create a placeholder structure to indicate the prototype is defined
        // The actual CompressedVectorNode creation will be implemented in Sprint W2
        // when we have point data to write

        // Task W1.3.2: Create a StructureNode to serve as the prototype
        e57::StructureNode prototypeStructure(*m_imageFile);

        // Task W1.3.3: Add three FloatNode children for XYZ coordinates
        e57::FloatNode xNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianX", xNode);

        e57::FloatNode yNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianY", yNode);

        e57::FloatNode zNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianZ", zNode);

        // For Sprint W1, we'll add the prototype as a placeholder structure
        // The actual CompressedVectorNode will be created when we have data to write
        m_currentScanNode->set("pointsPrototype", prototypeStructure);

        qDebug() << "E57WriterLib: Successfully defined XYZ prototype for current scan";
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
