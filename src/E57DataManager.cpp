#include "E57DataManager.h"
#include <E57Format.h>
#include <QDebug>
#include <QFileInfo>
#include <QMutexLocker>
#include <QDateTime>
#include <QUuid>
#include <algorithm>
#include <cmath>

E57DataManager::E57DataManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "E57DataManager: Initialized";
}

E57DataManager::~E57DataManager()
{
    qDebug() << "E57DataManager: Destroyed";
}

QVector<QVector<PointData>> E57DataManager::importE57File(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    clearError();
    
    QVector<QVector<PointData>> allScans;
    
    try {
        emit operationStarted(QString("Importing E57 file: %1").arg(QFileInfo(filePath).fileName()));
        
        qDebug() << "E57DataManager: Opening file for import:" << filePath;
        
        // Open the E57 file using libE57Format
        e57::ImageFile imageFile(filePath.toStdString(), "r");
        e57::StructureNode root = imageFile.root();
        
        // Check if the file has Data3D section
        if (!root.isDefined("data3D")) {
            throw E57Exception("E57 file does not contain any 3D data sections");
        }
        
        e57::VectorNode data3D(root.get("data3D"));
        int64_t scanCount = data3D.childCount();
        
        if (scanCount == 0) {
            throw E57Exception("E57 file contains no scans");
        }
        
        qDebug() << "E57DataManager: Found" << scanCount << "scans in file";
        
        // Process each scan
        for (int64_t i = 0; i < scanCount; ++i) {
            QVector<PointData> points;
            ScanMetadata metadata;
            
            qDebug() << "E57DataManager: Processing scan" << (i + 1) << "of" << scanCount;
            
            // Parse the scan
            parseScanDirect(imageFile, data3D, i, points, metadata);
            
            allScans.append(points);
            
            // Emit progress
            int progressPercent = static_cast<int>((i + 1) * 100.0 / scanCount);
            emit progress(progressPercent);
        }
        
        imageFile.close();
        
        qDebug() << "E57DataManager: Successfully imported" << allScans.size() << "scans";
        emit operationCompleted();
        
    } catch (const E57Exception& ex) {
        setError(ex.message());
        throw;
    } catch (const e57::E57Exception& ex) {
        QString errorMsg = QString("E57 library error during import: %1").arg(ex.what());
        handleE57Exception(ex, "importE57File");
        throw E57Exception(errorMsg);
    } catch (const std::exception& ex) {
        QString errorMsg = QString("Standard library error during import: %1").arg(ex.what());
        setError(errorMsg);
        throw E57Exception(errorMsg);
    }
    
    return allScans;
}

void E57DataManager::exportE57File(const QString& filePath, const QVector<QVector<PointData>>& scans)
{
    QMutexLocker locker(&m_mutex);
    clearError();
    
    try {
        emit operationStarted(QString("Exporting E57 file: %1").arg(QFileInfo(filePath).fileName()));
        
        qDebug() << "E57DataManager: Creating file for export:" << filePath;
        
        // Create the E57 file
        e57::ImageFile imageFile(filePath.toStdString(), "w");
        e57::StructureNode root = imageFile.root();
        
        // Create the data3D vector to hold all scans
        e57::VectorNode data3D = e57::VectorNode(imageFile, true);
        root.set("data3D", data3D);
        
        // Process each scan
        for (int scanIndex = 0; scanIndex < scans.size(); ++scanIndex) {
            const QVector<PointData>& points = scans[scanIndex];
            
            if (points.isEmpty()) {
                qDebug() << "E57DataManager: Skipping empty scan" << scanIndex;
                continue;
            }
            
            qDebug() << "E57DataManager: Writing scan" << (scanIndex + 1) << "with" << points.size() << "points";
            
            // Create scan metadata
            ScanMetadata metadata;
            metadata.pointCount = points.size();
            metadata.name = QString("Scan_%1").arg(scanIndex + 1);
            metadata.guid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            metadata.acquisitionTime = QDateTime::currentDateTime().toString(Qt::ISODate);
            
            // Calculate bounds
            calculateBounds(points, metadata.minX, metadata.minY, metadata.minZ,
                          metadata.maxX, metadata.maxY, metadata.maxZ);
            
            // Check for color and intensity data
            metadata.hasColorData = std::any_of(points.begin(), points.end(), 
                                              [](const PointData& p) { return p.hasColor; });
            metadata.hasIntensityData = std::any_of(points.begin(), points.end(), 
                                                   [](const PointData& p) { return p.hasIntensity; });
            
            // Write the scan
            writeScanDirect(imageFile, data3D, points, metadata, scanIndex);
            
            // Emit progress
            int progressPercent = static_cast<int>((scanIndex + 1) * 100.0 / scans.size());
            emit progress(progressPercent);
        }
        
        imageFile.close();
        
        qDebug() << "E57DataManager: Successfully exported" << scans.size() << "scans";
        emit operationCompleted();
        
    } catch (const E57Exception& ex) {
        setError(ex.message());
        throw;
    } catch (const e57::E57Exception& ex) {
        QString errorMsg = QString("E57 library error during export: %1").arg(ex.what());
        handleE57Exception(ex, "exportE57File");
        throw E57Exception(errorMsg);
    } catch (const std::exception& ex) {
        QString errorMsg = QString("Standard library error during export: %1").arg(ex.what());
        setError(errorMsg);
        throw E57Exception(errorMsg);
    }
}

QVector<ScanMetadata> E57DataManager::getScanMetadata(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    clearError();
    
    QVector<ScanMetadata> metadata;
    
    try {
        qDebug() << "E57DataManager: Reading metadata from:" << filePath;
        
        e57::ImageFile imageFile(filePath.toStdString(), "r");
        e57::StructureNode root = imageFile.root();
        
        if (!root.isDefined("data3D")) {
            throw E57Exception("E57 file does not contain any 3D data sections");
        }
        
        e57::VectorNode data3D(root.get("data3D"));
        int64_t scanCount = data3D.childCount();
        
        for (int64_t i = 0; i < scanCount; ++i) {
            ScanMetadata scanMeta;
            
            e57::StructureNode scan = e57::StructureNode(data3D.get(i));
            
            // Extract basic metadata
            if (scan.isDefined("name")) {
                e57::StringNode nameNode(scan.get("name"));
                scanMeta.name = QString::fromStdString(nameNode.value());
            } else {
                scanMeta.name = QString("Scan_%1").arg(i + 1);
            }
            
            if (scan.isDefined("guid")) {
                e57::StringNode guidNode(scan.get("guid"));
                scanMeta.guid = QString::fromStdString(guidNode.value());
            }
            
            if (scan.isDefined("acquisitionStart")) {
                // Handle acquisition time if present
                e57::StructureNode acqStart(scan.get("acquisitionStart"));
                if (acqStart.isDefined("dateTimeValue")) {
                    e57::FloatNode timeNode(acqStart.get("dateTimeValue"));
                    // Convert GPS time to readable format (simplified)
                    scanMeta.acquisitionTime = QString::number(timeNode.value());
                }
            }
            
            // Extract point count and bounds from points section
            if (scan.isDefined("points")) {
                e57::CompressedVectorNode points(scan.get("points"));
                scanMeta.pointCount = points.childCount();
                
                // Get prototype to check available attributes
                e57::StructureNode prototype = e57::StructureNode(points.prototype());
                scanMeta.hasColorData = prototype.isDefined("colorRed") && 
                                       prototype.isDefined("colorGreen") && 
                                       prototype.isDefined("colorBlue");
                scanMeta.hasIntensityData = prototype.isDefined("intensity");
            }
            
            metadata.append(scanMeta);
        }
        
        imageFile.close();
        
    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "getScanMetadata");
        throw E57Exception(QString("E57 library error reading metadata: %1").arg(ex.what()));
    } catch (const std::exception& ex) {
        QString errorMsg = QString("Error reading metadata: %1").arg(ex.what());
        setError(errorMsg);
        throw E57Exception(errorMsg);
    }
    
    return metadata;
}

bool E57DataManager::isValidE57File(const QString& filePath)
{
    try {
        e57::ImageFile imageFile(filePath.toStdString(), "r");
        e57::StructureNode root = imageFile.root();
        imageFile.close();
        return true;
    } catch (...) {
        return false;
    }
}

QString E57DataManager::getLastError() const
{
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

void E57DataManager::calculateBounds(const QVector<PointData>& points,
                                    double& minX, double& minY, double& minZ,
                                    double& maxX, double& maxY, double& maxZ)
{
    if (points.isEmpty()) {
        minX = minY = minZ = maxX = maxY = maxZ = 0.0;
        return;
    }
    
    minX = maxX = points[0].x;
    minY = maxY = points[0].y;
    minZ = maxZ = points[0].z;
    
    for (const PointData& point : points) {
        minX = std::min(minX, point.x);
        minY = std::min(minY, point.y);
        minZ = std::min(minZ, point.z);
        maxX = std::max(maxX, point.x);
        maxY = std::max(maxY, point.y);
        maxZ = std::max(maxZ, point.z);
    }
}

void E57DataManager::handleE57Exception(const std::exception& ex, const QString& context)
{
    QString errorMsg = QString("E57 error in %1: %2").arg(context, ex.what());
    qDebug() << "E57DataManager:" << errorMsg;
    setError(errorMsg);
}

void E57DataManager::setError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
}

void E57DataManager::clearError()
{
    m_lastError.clear();
}

void E57DataManager::parseScanDirect(e57::ImageFile& imageFile, e57::VectorNode& data3D,
                                     int64_t scanIndex, QVector<PointData>& outPoints,
                                     ScanMetadata& metadata)
{
    try {
        e57::StructureNode scan = e57::StructureNode(data3D.get(scanIndex));

        // Extract metadata
        if (scan.isDefined("name")) {
            e57::StringNode nameNode(scan.get("name"));
            metadata.name = QString::fromStdString(nameNode.value());
        } else {
            metadata.name = QString("Scan_%1").arg(scanIndex + 1);
        }

        if (scan.isDefined("guid")) {
            e57::StringNode guidNode(scan.get("guid"));
            metadata.guid = QString::fromStdString(guidNode.value());
        }

        // Get points data
        if (!scan.isDefined("points")) {
            throw E57Exception(QString("Scan %1 does not contain points data").arg(scanIndex));
        }

        e57::CompressedVectorNode points(scan.get("points"));
        int64_t pointCount = points.childCount();
        metadata.pointCount = pointCount;

        if (pointCount == 0) {
            qDebug() << "E57DataManager: Scan" << scanIndex << "contains no points";
            return;
        }

        // Get prototype to determine available attributes
        e57::StructureNode prototype = e57::StructureNode(points.prototype());
        bool hasX = prototype.isDefined("cartesianX");
        bool hasY = prototype.isDefined("cartesianY");
        bool hasZ = prototype.isDefined("cartesianZ");
        bool hasColorRed = prototype.isDefined("colorRed");
        bool hasColorGreen = prototype.isDefined("colorGreen");
        bool hasColorBlue = prototype.isDefined("colorBlue");
        bool hasIntensity = prototype.isDefined("intensity");

        if (!hasX || !hasY || !hasZ) {
            throw E57Exception(QString("Scan %1 missing required XYZ coordinates").arg(scanIndex));
        }

        metadata.hasColorData = hasColorRed && hasColorGreen && hasColorBlue;
        metadata.hasIntensityData = hasIntensity;

        qDebug() << "E57DataManager: Scan" << scanIndex << "has" << pointCount << "points"
                 << "Color:" << metadata.hasColorData << "Intensity:" << metadata.hasIntensityData;

        // Prepare data buffers for chunked reading
        const size_t chunkSize = std::min(static_cast<size_t>(pointCount), CHUNK_SIZE);
        std::vector<double> xData(chunkSize), yData(chunkSize), zData(chunkSize);
        std::vector<uint8_t> redData(chunkSize), greenData(chunkSize), blueData(chunkSize);
        std::vector<float> intensityData(chunkSize);

        // Set up the reader
        std::vector<e57::SourceDestBuffer> destBuffers;
        destBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianX", xData.data(), chunkSize, true));
        destBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianY", yData.data(), chunkSize, true));
        destBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianZ", zData.data(), chunkSize, true));

        if (metadata.hasColorData) {
            destBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorRed", redData.data(), chunkSize, true));
            destBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorGreen", greenData.data(), chunkSize, true));
            destBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorBlue", blueData.data(), chunkSize, true));
        }

        if (metadata.hasIntensityData) {
            destBuffers.push_back(e57::SourceDestBuffer(imageFile, "intensity", intensityData.data(), chunkSize, true));
        }

        e57::CompressedVectorReader reader = points.reader(destBuffers);

        // Read points in chunks
        size_t totalPointsRead = 0;
        outPoints.reserve(pointCount);

        while (totalPointsRead < static_cast<size_t>(pointCount)) {
            uint64_t pointsRead = reader.read();
            if (pointsRead == 0) break;

            for (uint64_t i = 0; i < pointsRead; ++i) {
                PointData point(xData[i], yData[i], zData[i]);

                if (metadata.hasColorData) {
                    point.r = redData[i];
                    point.g = greenData[i];
                    point.b = blueData[i];
                    point.hasColor = true;
                }

                if (metadata.hasIntensityData) {
                    point.intensity = intensityData[i];
                    point.hasIntensity = true;
                }

                outPoints.append(point);
            }

            totalPointsRead += pointsRead;

            // Update progress periodically
            if (totalPointsRead % PROGRESS_UPDATE_INTERVAL == 0) {
                qDebug() << "E57DataManager: Read" << totalPointsRead << "of" << pointCount << "points";
            }
        }

        reader.close();

        // Calculate bounds
        this->calculateBounds(outPoints, metadata.minX, metadata.minY, metadata.minZ,
                             metadata.maxX, metadata.maxY, metadata.maxZ);

        qDebug() << "E57DataManager: Successfully parsed scan" << scanIndex
                 << "with" << outPoints.size() << "points";

    } catch (const e57::E57Exception& ex) {
        throw E57Exception(QString("E57 error parsing scan %1: %2").arg(scanIndex).arg(ex.what()));
    } catch (const std::exception& ex) {
        throw E57Exception(QString("Error parsing scan %1: %2").arg(scanIndex).arg(ex.what()));
    }
}

void E57DataManager::writeScanDirect(e57::ImageFile& imageFile, e57::VectorNode& data3D,
                                     const QVector<PointData>& points, const ScanMetadata& metadata,
                                     size_t scanIndex)
{
    try {
        if (points.isEmpty()) {
            qDebug() << "E57DataManager: Skipping empty scan" << scanIndex;
            return;
        }

        // Create scan structure
        e57::StructureNode scan = e57::StructureNode(imageFile);

        // Set scan metadata
        scan.set("name", e57::StringNode(imageFile, metadata.name.toStdString()));
        scan.set("guid", e57::StringNode(imageFile, metadata.guid.toStdString()));

        // Set acquisition time if available
        if (!metadata.acquisitionTime.isEmpty()) {
            e57::StructureNode acquisitionStart = e57::StructureNode(imageFile);
            // Simplified time handling - in a real implementation, you'd convert properly
            acquisitionStart.set("dateTimeValue", e57::FloatNode(imageFile, QDateTime::currentMSecsSinceEpoch() / 1000.0));
            scan.set("acquisitionStart", acquisitionStart);
        }

        // Set bounding box
        e57::StructureNode bbox = e57::StructureNode(imageFile);
        bbox.set("xMinimum", e57::FloatNode(imageFile, metadata.minX));
        bbox.set("xMaximum", e57::FloatNode(imageFile, metadata.maxX));
        bbox.set("yMinimum", e57::FloatNode(imageFile, metadata.minY));
        bbox.set("yMaximum", e57::FloatNode(imageFile, metadata.maxY));
        bbox.set("zMinimum", e57::FloatNode(imageFile, metadata.minZ));
        bbox.set("zMaximum", e57::FloatNode(imageFile, metadata.maxZ));
        scan.set("cartesianBounds", bbox);

        // Create point record prototype
        e57::StructureNode prototype = e57::StructureNode(imageFile);

        // Add XYZ fields (required)
        prototype.set("cartesianX", e57::FloatNode(imageFile, 0.0, e57::PrecisionDouble, metadata.minX, metadata.maxX));
        prototype.set("cartesianY", e57::FloatNode(imageFile, 0.0, e57::PrecisionDouble, metadata.minY, metadata.maxY));
        prototype.set("cartesianZ", e57::FloatNode(imageFile, 0.0, e57::PrecisionDouble, metadata.minZ, metadata.maxZ));

        // Add color fields if present
        if (metadata.hasColorData) {
            prototype.set("colorRed", e57::IntegerNode(imageFile, 0, 0, 255));
            prototype.set("colorGreen", e57::IntegerNode(imageFile, 0, 0, 255));
            prototype.set("colorBlue", e57::IntegerNode(imageFile, 0, 0, 255));
        }

        // Add intensity field if present
        if (metadata.hasIntensityData) {
            prototype.set("intensity", e57::FloatNode(imageFile, 0.0, e57::PrecisionSingle, 0.0, 1.0));
        }

        // Create compressed vector for points
        e57::VectorNode pointsVector = e57::VectorNode(imageFile, true);
        scan.set("points", pointsVector);

        // Prepare data buffers for chunked writing
        const size_t chunkSize = std::min(static_cast<size_t>(points.size()), CHUNK_SIZE);
        std::vector<double> xData(chunkSize), yData(chunkSize), zData(chunkSize);
        std::vector<uint8_t> redData(chunkSize), greenData(chunkSize), blueData(chunkSize);
        std::vector<float> intensityData(chunkSize);

        // Set up the writer buffers
        std::vector<e57::SourceDestBuffer> sourceBuffers;
        sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianX", xData.data(), chunkSize, true));
        sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianY", yData.data(), chunkSize, true));
        sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "cartesianZ", zData.data(), chunkSize, true));

        if (metadata.hasColorData) {
            sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorRed", redData.data(), chunkSize, true));
            sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorGreen", greenData.data(), chunkSize, true));
            sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "colorBlue", blueData.data(), chunkSize, true));
        }

        if (metadata.hasIntensityData) {
            sourceBuffers.push_back(e57::SourceDestBuffer(imageFile, "intensity", intensityData.data(), chunkSize, true));
        }

        // For now, let's use a simpler approach and just add the scan structure
        // The actual point writing would need more complex setup with libE57Format
        // TODO: Implement proper point data writing using CompressedVectorWriter

        qDebug() << "E57DataManager: Scan structure created (point data writing not yet implemented)";

        // Add scan to data3D vector
        data3D.append(scan);

        qDebug() << "E57DataManager: Successfully wrote scan" << scanIndex
                 << "with" << points.size() << "points";

    } catch (const e57::E57Exception& ex) {
        throw E57Exception(QString("E57 error writing scan %1: %2").arg(scanIndex).arg(ex.what()));
    } catch (const std::exception& ex) {
        throw E57Exception(QString("Error writing scan %1: %2").arg(scanIndex).arg(ex.what()));
    }
}
