#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include "../src/projectmanager.h"

class ProjectManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up application properties for QSettings
        QCoreApplication::setOrganizationName("CloudRegistrationApp");
        QCoreApplication::setApplicationName("CloudRegistration");
        
        m_tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_tempDir->isValid());
        m_testBasePath = m_tempDir->path();
        
        m_projectManager = std::make_unique<ProjectManager>();
    }

    void TearDown() override
    {
        m_projectManager.reset();
        m_tempDir.reset();
    }

    void createTestProject(const QString &name, const QString &basePath)
    {
        QString projectPath = m_projectManager->createProject(name, basePath);
        ASSERT_FALSE(projectPath.isEmpty());
    }

    void corruptMetadataFile(const QString &projectPath)
    {
        QString metadataPath = ProjectManager::getMetadataFilePath(projectPath);
        QFile file(metadataPath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write("{ invalid json");
        file.close();
    }

    std::unique_ptr<ProjectManager> m_projectManager;
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QString m_testBasePath;
};

// Test Case 2.1: Create a new project with valid name and new folder
TEST_F(ProjectManagerTest, CreateValidProject)
{
    QString projectName = "TestProject1";
    QString projectPath = m_projectManager->createProject(projectName, m_testBasePath);
    
    EXPECT_FALSE(projectPath.isEmpty());
    EXPECT_TRUE(QDir(projectPath).exists());
    EXPECT_TRUE(m_projectManager->isValidProject(projectPath));
    
    // Verify metadata file contents
    QString metadataPath = ProjectManager::getMetadataFilePath(projectPath);
    EXPECT_TRUE(QFile::exists(metadataPath));
    
    QFile file(metadataPath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    EXPECT_EQ(obj["projectName"].toString(), projectName);
    EXPECT_FALSE(obj["projectID"].toString().isEmpty());
    EXPECT_FALSE(obj["creationDate"].toString().isEmpty());
    EXPECT_EQ(obj["fileFormatVersion"].toString(), QString("1.0.0"));
}

// Test Case 2.2: Attempt to create a project in a restricted/non-writable directory
TEST_F(ProjectManagerTest, CreateProjectInvalidLocation)
{
    EXPECT_THROW(
        m_projectManager->createProject("TestProject", "/invalid/path/that/does/not/exist"),
        ProjectCreationException
    );
}

// Test Case 2.3: Verify contents of the created project_meta.json
TEST_F(ProjectManagerTest, VerifyProjectMetadata)
{
    QString projectName = "MetadataTestProject";
    QString projectPath = m_projectManager->createProject(projectName, m_testBasePath);
    
    ProjectInfo info = m_projectManager->loadProject(projectPath);
    
    EXPECT_EQ(info.projectName, projectName);
    EXPECT_FALSE(info.projectId.isEmpty());
    EXPECT_FALSE(info.creationDate.isEmpty());
    EXPECT_EQ(info.fileFormatVersion, QString("1.0.0"));
    EXPECT_EQ(info.projectPath, projectPath);
    EXPECT_TRUE(info.isValid());
}

// Test Case 3.1: Open a valid existing project
TEST_F(ProjectManagerTest, OpenValidProject)
{
    // Create a test project first
    QString projectName = "ValidTestProject";
    QString projectPath = m_projectManager->createProject(projectName, m_testBasePath);
    
    // Load the project
    ProjectInfo info = m_projectManager->loadProject(projectPath);
    
    EXPECT_EQ(info.projectName, projectName);
    EXPECT_FALSE(info.projectId.isEmpty());
    EXPECT_FALSE(info.creationDate.isEmpty());
    EXPECT_EQ(info.fileFormatVersion, QString("1.0.0"));
    EXPECT_EQ(info.projectPath, projectPath);
}

// Test Case 3.2: Attempt to open a folder that is not a valid project
TEST_F(ProjectManagerTest, OpenInvalidProject)
{
    QString invalidPath = QDir(m_testBasePath).absoluteFilePath("NonExistentProject");
    
    EXPECT_THROW(
        m_projectManager->loadProject(invalidPath),
        ProjectLoadException
    );
}

// Test Case 3.3: Attempt to open a project with a corrupted project_meta.json
TEST_F(ProjectManagerTest, OpenCorruptedProject)
{
    // Create a valid project first
    QString projectName = "CorruptedTestProject";
    QString projectPath = m_projectManager->createProject(projectName, m_testBasePath);
    
    // Corrupt the metadata file
    corruptMetadataFile(projectPath);
    
    EXPECT_THROW(
        m_projectManager->loadProject(projectPath),
        ProjectLoadException
    );
}

TEST_F(ProjectManagerTest, CreateProjectInvalidName)
{
    EXPECT_THROW(
        m_projectManager->createProject("", m_testBasePath),
        ProjectCreationException
    );
    
    EXPECT_THROW(
        m_projectManager->createProject("   ", m_testBasePath),
        ProjectCreationException
    );
}

TEST_F(ProjectManagerTest, MetadataValidation)
{
    QJsonObject validMetadata;
    validMetadata["projectID"] = "12345678-1234-1234-1234-123456789abc";
    validMetadata["projectName"] = "Test Project";
    validMetadata["creationDate"] = "2025-01-01T00:00:00Z";
    validMetadata["fileFormatVersion"] = "1.0.0";
    
    EXPECT_TRUE(m_projectManager->validateProjectMetadata(validMetadata));
    
    // Test missing field
    QJsonObject invalidMetadata = validMetadata;
    invalidMetadata.remove("projectName");
    EXPECT_FALSE(m_projectManager->validateProjectMetadata(invalidMetadata));
    
    // Test invalid UUID
    QJsonObject invalidUuidMetadata = validMetadata;
    invalidUuidMetadata["projectID"] = "invalid-uuid";
    EXPECT_FALSE(m_projectManager->validateProjectMetadata(invalidUuidMetadata));
}

TEST_F(ProjectManagerTest, IsProjectDirectory)
{
    QString projectName = "DirectoryTestProject";
    QString projectPath = m_projectManager->createProject(projectName, m_testBasePath);
    
    EXPECT_TRUE(ProjectManager::isProjectDirectory(projectPath));
    
    QString nonProjectPath = QDir(m_testBasePath).absoluteFilePath("NotAProject");
    QDir().mkpath(nonProjectPath);
    EXPECT_FALSE(ProjectManager::isProjectDirectory(nonProjectPath));
}
