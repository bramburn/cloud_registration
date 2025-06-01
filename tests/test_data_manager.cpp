#include "test_data_manager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>

TestDataManager::TestDataManager(QObject *parent) : QObject(parent)
{
    m_testDataPath = "tests/data";
}

void TestDataManager::setupTestDataRepository()
{
    qDebug() << "Setting up test data repository at:" << m_testDataPath;
    
    // Create directory structure
    QStringList categories = {"basic", "advanced", "corrupted", "large", "edge_cases"};
    
    for (const QString &category : categories) {
        QString categoryPath = m_testDataPath + "/" + category;
        QDir().mkpath(categoryPath);
        qDebug() << "Created category directory:" << categoryPath;
    }
    
    emit testDataSetupComplete();
}

void TestDataManager::curateTestFiles()
{
    qDebug() << "Curating test files...";
    
    // Define curated test file specifications
    QList<QPair<QString, QString>> curatedFiles = {
        // Basic valid files
        {"sample_small.e57", "Basic E57 file with 1000 points, bitPackCodec"},
        {"sample_small.las", "Basic LAS file with 1000 points, PDRF 1"},
        
        // Format variations
        {"multi_scan.e57", "E57 file with multiple data3D sections"},
        {"pdrf0.las", "LAS file using Point Data Record Format 0"},
        {"pdrf2.las", "LAS file using Point Data Record Format 2"},
        {"pdrf3.las", "LAS file using Point Data Record Format 3"},
        
        // Edge cases
        {"extreme_coords.las", "LAS file with extreme coordinate scale/offset"},
        {"many_vlrs.las", "LAS file with numerous Variable Length Records"},
        {"large_file.e57", "Large E57 file with 1M+ points"},
        
        // Error test cases
        {"corrupted_header.e57", "E57 file with corrupted header"},
        {"invalid_xml.e57", "E57 file with malformed XML structure"},
        {"truncated.las", "Truncated LAS file"},
        {"wrong_extension.txt", "Non-point-cloud file with wrong extension"}
    };
    
    for (const auto &fileInfo : curatedFiles) {
        TestFileInfo info;
        info.fileName = fileInfo.first;
        info.description = fileInfo.second;
        info.category = determineCategory(fileInfo.first);
        info.isValid = !fileInfo.second.contains("corrupted") && 
                      !fileInfo.second.contains("invalid") &&
                      !fileInfo.second.contains("truncated");
        
        QString fullPath = m_testDataPath + "/" + info.category + "/" + info.fileName;
        if (QFile::exists(fullPath)) {
            info.fileSize = QFileInfo(fullPath).size();
            calculateFileChecksum(fullPath);
        } else {
            qDebug() << "Test file not found (will be generated):" << fullPath;
        }
        
        m_testFiles.append(info);
    }
    
    qDebug() << "Curated" << m_testFiles.size() << "test files";
}

QString TestDataManager::determineCategory(const QString &fileName)
{
    if (fileName.contains("corrupted") || fileName.contains("invalid") || fileName.contains("truncated")) {
        return "corrupted";
    } else if (fileName.contains("large") || fileName.contains("stress")) {
        return "large";
    } else if (fileName.contains("extreme") || fileName.contains("edge") || fileName.contains("many")) {
        return "edge_cases";
    } else if (fileName.contains("advanced") || fileName.contains("multi")) {
        return "advanced";
    } else {
        return "basic";
    }
}

bool TestDataManager::addTestFile(const QString &filePath, const QString &category)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return false;
    }
    
    TestFileInfo info;
    info.fileName = fileInfo.fileName();
    info.category = category;
    info.fileSize = fileInfo.size();
    info.isValid = validateFileStructure(filePath);
    
    calculateFileChecksum(filePath);
    
    // Copy file to appropriate category directory
    QString targetPath = m_testDataPath + "/" + category + "/" + info.fileName;
    QDir().mkpath(QFileInfo(targetPath).absolutePath());
    
    if (QFile::copy(filePath, targetPath)) {
        m_testFiles.append(info);
        qDebug() << "Added test file:" << targetPath;
        return true;
    } else {
        qWarning() << "Failed to copy test file to:" << targetPath;
        return false;
    }
}

bool TestDataManager::removeTestFile(const QString &fileName)
{
    for (int i = 0; i < m_testFiles.size(); ++i) {
        if (m_testFiles[i].fileName == fileName) {
            QString filePath = m_testDataPath + "/" + m_testFiles[i].category + "/" + fileName;
            if (QFile::remove(filePath)) {
                m_testFiles.removeAt(i);
                qDebug() << "Removed test file:" << filePath;
                return true;
            } else {
                qWarning() << "Failed to remove test file:" << filePath;
                return false;
            }
        }
    }
    
    qWarning() << "Test file not found:" << fileName;
    return false;
}

QStringList TestDataManager::getTestFilesByCategory(const QString &category)
{
    QStringList files;
    
    for (const TestFileInfo &info : m_testFiles) {
        if (info.category == category) {
            QString fullPath = m_testDataPath + "/" + category + "/" + info.fileName;
            if (QFile::exists(fullPath)) {
                files.append(fullPath);
            }
        }
    }
    
    return files;
}

void TestDataManager::generateTestFileMetadata()
{
    QJsonObject metadata;
    QJsonArray fileArray;
    
    for (const TestFileInfo &info : m_testFiles) {
        QJsonObject fileObj;
        fileObj["fileName"] = info.fileName;
        fileObj["category"] = info.category;
        fileObj["fileSize"] = info.fileSize;
        fileObj["checksum"] = info.checksum;
        fileObj["description"] = info.description;
        fileObj["isValid"] = info.isValid;
        
        QString fullPath = m_testDataPath + "/" + info.category + "/" + info.fileName;
        fileObj["exists"] = QFile::exists(fullPath);
        
        fileArray.append(fileObj);
    }
    
    metadata["testFiles"] = fileArray;
    metadata["totalFiles"] = m_testFiles.size();
    metadata["generatedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QString metadataPath = m_testDataPath + "/test_metadata.json";
    QFile file(metadataPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(metadata);
        file.write(doc.toJson());
        qDebug() << "Test metadata generated:" << metadataPath;
        emit metadataGenerated(metadataPath);
    } else {
        qWarning() << "Failed to write metadata to:" << metadataPath;
    }
}

QJsonObject TestDataManager::getFileMetadata(const QString &fileName)
{
    for (const TestFileInfo &info : m_testFiles) {
        if (info.fileName == fileName) {
            QJsonObject metadata;
            metadata["fileName"] = info.fileName;
            metadata["category"] = info.category;
            metadata["fileSize"] = info.fileSize;
            metadata["checksum"] = info.checksum;
            metadata["description"] = info.description;
            metadata["isValid"] = info.isValid;
            return metadata;
        }
    }
    
    return QJsonObject();
}

void TestDataManager::calculateFileChecksum(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(&file);
        QString checksum = hash.result().toHex();
        
        // Update checksum in test file info
        for (TestFileInfo &info : m_testFiles) {
            if (m_testDataPath + "/" + info.category + "/" + info.fileName == filePath) {
                info.checksum = checksum;
                break;
            }
        }
    }
}

bool TestDataManager::validateFileStructure(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    if (extension == "e57") {
        // Basic E57 validation
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray header = file.read(8);
            return header.startsWith("ASTM-E57");
        }
    } else if (extension == "las") {
        // Basic LAS validation
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray header = file.read(4);
            return header.startsWith("LASF");
        }
    }
    
    return false; // Unknown format or validation failed
}

void TestDataManager::validateTestDataIntegrity()
{
    qDebug() << "Validating test data integrity...";
    
    int validFiles = 0;
    int invalidFiles = 0;
    int missingFiles = 0;
    
    for (const TestFileInfo &info : m_testFiles) {
        QString fullPath = m_testDataPath + "/" + info.category + "/" + info.fileName;
        
        if (!QFile::exists(fullPath)) {
            missingFiles++;
            qWarning() << "Missing test file:" << fullPath;
            continue;
        }
        
        if (validateFileStructure(fullPath)) {
            validFiles++;
        } else {
            invalidFiles++;
            qWarning() << "Invalid test file structure:" << fullPath;
        }
    }
    
    qDebug() << "Test data integrity check completed:";
    qDebug() << "  Valid files:" << validFiles;
    qDebug() << "  Invalid files:" << invalidFiles;
    qDebug() << "  Missing files:" << missingFiles;
}

void TestDataManager::updateCMakeForTestData()
{
    qDebug() << "Updating CMake configuration for test data...";
    
    // This would generate CMake configuration to include test data paths
    // For now, just log the information
    
    qDebug() << "Test data categories and file counts:";
    QMap<QString, int> categoryCounts;
    
    for (const TestFileInfo &info : m_testFiles) {
        categoryCounts[info.category]++;
    }
    
    for (auto it = categoryCounts.begin(); it != categoryCounts.end(); ++it) {
        qDebug() << "  " << it.key() << ":" << it.value() << "files";
    }
}
