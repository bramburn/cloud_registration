#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QCoreApplication>
#include <QSignalSpy>
#include "../src/recentprojectsmanager.h"

class RecentProjectsManagerTest : public ::testing::Test
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
        
        m_recentManager = std::make_unique<RecentProjectsManager>();
        
        // Clear any existing recent projects
        m_recentManager->clearRecentProjects();
    }

    void TearDown() override
    {
        m_recentManager.reset();
        m_tempDir.reset();
    }

    QString createTestProjectPath(const QString &name)
    {
        QString projectPath = QDir(m_testBasePath).absoluteFilePath(name);
        QDir().mkpath(projectPath);
        return projectPath;
    }

    std::unique_ptr<RecentProjectsManager> m_recentManager;
    std::unique_ptr<QTemporaryDir> m_tempDir;
    QString m_testBasePath;
};

// Test Case 4.1: Verify empty list on first launch
TEST_F(RecentProjectsManagerTest, EmptyListOnFirstLaunch)
{
    QStringList recentProjects = m_recentManager->getRecentProjects();
    EXPECT_TRUE(recentProjects.isEmpty());
}

// Test Case 4.2: Create/open several projects and verify they appear in the recent list in correct order
TEST_F(RecentProjectsManagerTest, AddProjectsInCorrectOrder)
{
    QString project1 = createTestProjectPath("Project1");
    QString project2 = createTestProjectPath("Project2");
    QString project3 = createTestProjectPath("Project3");
    
    QSignalSpy spy(m_recentManager.get(), &RecentProjectsManager::recentProjectsChanged);
    
    m_recentManager->addProject(project1);
    m_recentManager->addProject(project2);
    m_recentManager->addProject(project3);
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    EXPECT_EQ(recentProjects.size(), 3);
    EXPECT_EQ(recentProjects[0], project3); // Most recent first
    EXPECT_EQ(recentProjects[1], project2);
    EXPECT_EQ(recentProjects[2], project1);
    
    // Verify signals were emitted
    EXPECT_EQ(spy.count(), 3);
}

// Test Case 4.3: Open a project from the recent list (move to top)
TEST_F(RecentProjectsManagerTest, MoveProjectToTop)
{
    QString project1 = createTestProjectPath("Project1");
    QString project2 = createTestProjectPath("Project2");
    QString project3 = createTestProjectPath("Project3");
    
    m_recentManager->addProject(project1);
    m_recentManager->addProject(project2);
    m_recentManager->addProject(project3);
    
    // Re-open project1 (should move to top)
    m_recentManager->addProject(project1);
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    EXPECT_EQ(recentProjects.size(), 3);
    EXPECT_EQ(recentProjects[0], project1); // Should be at top now
    EXPECT_EQ(recentProjects[1], project3);
    EXPECT_EQ(recentProjects[2], project2);
}

// Test Case 4.4: Verify list size limit is enforced
TEST_F(RecentProjectsManagerTest, EnforceListSizeLimit)
{
    // Add more than the maximum number of projects
    for (int i = 1; i <= 15; ++i) {
        QString projectPath = createTestProjectPath(QString("Project%1").arg(i));
        m_recentManager->addProject(projectPath);
    }
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    // Should be limited to 10 projects
    EXPECT_EQ(recentProjects.size(), 10);
    
    // Verify the most recent projects are kept
    for (int i = 0; i < 10; ++i) {
        QString expectedProject = createTestProjectPath(QString("Project%1").arg(15 - i));
        EXPECT_EQ(recentProjects[i], expectedProject);
    }
}

TEST_F(RecentProjectsManagerTest, RemoveProject)
{
    QString project1 = createTestProjectPath("Project1");
    QString project2 = createTestProjectPath("Project2");
    QString project3 = createTestProjectPath("Project3");
    
    m_recentManager->addProject(project1);
    m_recentManager->addProject(project2);
    m_recentManager->addProject(project3);
    
    QSignalSpy spy(m_recentManager.get(), &RecentProjectsManager::recentProjectsChanged);
    
    m_recentManager->removeProject(project2);
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    EXPECT_EQ(recentProjects.size(), 2);
    EXPECT_FALSE(recentProjects.contains(project2));
    EXPECT_TRUE(recentProjects.contains(project1));
    EXPECT_TRUE(recentProjects.contains(project3));
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(RecentProjectsManagerTest, ClearRecentProjects)
{
    QString project1 = createTestProjectPath("Project1");
    QString project2 = createTestProjectPath("Project2");
    
    m_recentManager->addProject(project1);
    m_recentManager->addProject(project2);
    
    EXPECT_EQ(m_recentManager->getRecentProjects().size(), 2);
    
    QSignalSpy spy(m_recentManager.get(), &RecentProjectsManager::recentProjectsChanged);
    
    m_recentManager->clearRecentProjects();
    
    EXPECT_TRUE(m_recentManager->getRecentProjects().isEmpty());
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(RecentProjectsManagerTest, SetRecentProjects)
{
    QString project1 = createTestProjectPath("Project1");
    QString project2 = createTestProjectPath("Project2");
    QString project3 = createTestProjectPath("Project3");
    
    QStringList newProjects = {project1, project2, project3};
    
    QSignalSpy spy(m_recentManager.get(), &RecentProjectsManager::recentProjectsChanged);
    
    m_recentManager->setRecentProjects(newProjects);
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    EXPECT_EQ(recentProjects.size(), 3);
    EXPECT_EQ(recentProjects, newProjects);
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(RecentProjectsManagerTest, GetProjectDisplayName)
{
    QString projectPath = "/path/to/MyProject";
    QString displayName = RecentProjectsManager::getProjectDisplayName(projectPath);
    EXPECT_EQ(displayName, "MyProject");
}

TEST_F(RecentProjectsManagerTest, HandleDuplicates)
{
    QString project1 = createTestProjectPath("Project1");
    
    m_recentManager->addProject(project1);
    m_recentManager->addProject(project1); // Add same project again
    
    QStringList recentProjects = m_recentManager->getRecentProjects();
    
    // Should only appear once
    EXPECT_EQ(recentProjects.size(), 1);
    EXPECT_EQ(recentProjects[0], project1);
}

TEST_F(RecentProjectsManagerTest, HandleInvalidPaths)
{
    // Adding empty path should be ignored
    m_recentManager->addProject("");
    EXPECT_TRUE(m_recentManager->getRecentProjects().isEmpty());
    
    // Adding non-existent path should be handled gracefully
    m_recentManager->addProject("/non/existent/path");
    // Should still work, but path won't be canonical
    EXPECT_EQ(m_recentManager->getRecentProjects().size(), 0); // Empty because canonical path is empty
}
