#ifndef TEST_DATA_MANAGER_H
#define TEST_DATA_MANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QStringList>
#include <QList>

/**
 * @brief Test Data Manager for Sprint 2.4
 * 
 * Manages curated test data repository for comprehensive testing.
 * Implements Task 2.4.3.3-2.4.3.5 from Sprint 2.4 requirements.
 */
class TestDataManager : public QObject
{
    Q_OBJECT

public:
    explicit TestDataManager(QObject *parent = nullptr);
    
    void setupTestDataRepository();
    void curateTestFiles();
    void validateTestDataIntegrity();
    void updateCMakeForTestData();
    
    // Test file management
    bool addTestFile(const QString &filePath, const QString &category);
    bool removeTestFile(const QString &fileName);
    QStringList getTestFilesByCategory(const QString &category);
    
    // Metadata management
    void generateTestFileMetadata();
    QJsonObject getFileMetadata(const QString &fileName);

signals:
    void testDataSetupComplete();
    void metadataGenerated(const QString &outputPath);

private:
    struct TestFileInfo {
        QString fileName;
        QString category;
        qint64 fileSize;
        QString checksum;
        QString description;
        bool isValid;
    };
    
    void calculateFileChecksum(const QString &filePath);
    bool validateFileStructure(const QString &filePath);
    QString determineCategory(const QString &fileName);
    
    QString m_testDataPath;
    QList<TestFileInfo> m_testFiles;
};

#endif // TEST_DATA_MANAGER_H
