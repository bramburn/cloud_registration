#include "projecthubwidget.h"
#include "createprojectdialog.h"
#include "projectmanager.h"
#include "recentprojectsmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QTimer>

ProjectHubWidget::ProjectHubWidget(QWidget *parent)
    : QWidget(parent)
    , m_recentManager(new RecentProjectsManager(this))
    , m_projectManager(new ProjectManager(this))
    , m_validationTimer(new QTimer(this))
{
    setupUI();
    setupStyles();
    
    // Setup validation timer for recent projects
    m_validationTimer->setSingleShot(true);
    m_validationTimer->setInterval(1000); // Validate after 1 second of inactivity
    connect(m_validationTimer, &QTimer::timeout, this, &ProjectHubWidget::validateRecentProjects);
    
    refreshRecentProjects();
}

void ProjectHubWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 40, 50, 40);
    
    // Title section
    m_titleLabel = new QLabel("Project Hub", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleLabel");
    
    auto *subtitleLabel = new QLabel("Create, open, or continue working on your projects", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setObjectName("subtitleLabel");
    
    // Action buttons section
    auto *buttonWidget = new QWidget();
    auto *buttonLayout = new QGridLayout(buttonWidget);
    buttonLayout->setSpacing(20);
    
    m_createNewBtn = new QPushButton("Create New Project", this);
    m_createNewBtn->setObjectName("primaryButton");
    m_createNewBtn->setMinimumHeight(60);
    m_createNewBtn->setCursor(Qt::PointingHandCursor);
    
    m_openProjectBtn = new QPushButton("Open Existing Project", this);
    m_openProjectBtn->setObjectName("secondaryButton");
    m_openProjectBtn->setMinimumHeight(60);
    m_openProjectBtn->setCursor(Qt::PointingHandCursor);
    
    buttonLayout->addWidget(m_createNewBtn, 0, 0);
    buttonLayout->addWidget(m_openProjectBtn, 0, 1);
    
    // Recent projects section
    m_recentLabel = new QLabel("Recent Projects", this);
    m_recentLabel->setObjectName("sectionLabel");
    
    m_recentProjectsList = new QListWidget(this);
    m_recentProjectsList->setObjectName("recentProjectsList");
    m_recentProjectsList->setMaximumHeight(250);
    m_recentProjectsList->setAlternatingRowColors(true);
    
    // Status label for feedback
    m_statusLabel = new QLabel("", this);
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    
    // Layout assembly
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(buttonWidget);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_recentLabel);
    mainLayout->addWidget(m_recentProjectsList);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addStretch();
    
    // Connect signals
    connect(m_createNewBtn, &QPushButton::clicked, this, &ProjectHubWidget::onCreateNewProject);
    connect(m_openProjectBtn, &QPushButton::clicked, this, &ProjectHubWidget::onOpenProject);
    connect(m_recentProjectsList, &QListWidget::itemClicked, 
            this, &ProjectHubWidget::onRecentProjectClicked);
    connect(m_recentProjectsList, &QListWidget::itemDoubleClicked, 
            this, &ProjectHubWidget::onRecentProjectDoubleClicked);
}

void ProjectHubWidget::setupStyles()
{
    setStyleSheet(R"(
        #titleLabel {
            font-size: 32px;
            font-weight: bold;
            color: #2c3e50;
            margin: 20px;
        }
        #subtitleLabel {
            font-size: 16px;
            color: #7f8c8d;
            margin-bottom: 10px;
        }
        #sectionLabel {
            font-size: 18px;
            font-weight: bold;
            color: #34495e;
            margin-top: 20px;
            margin-bottom: 10px;
        }
        #primaryButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
            padding: 15px;
        }
        #primaryButton:hover {
            background-color: #2980b9;
        }
        #primaryButton:pressed {
            background-color: #21618c;
        }
        #secondaryButton {
            background-color: #95a5a6;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
            padding: 15px;
        }
        #secondaryButton:hover {
            background-color: #7f8c8d;
        }
        #secondaryButton:pressed {
            background-color: #6c7b7d;
        }
        #recentProjectsList {
            background-color: white;
            border: 1px solid #bdc3c7;
            border-radius: 6px;
            font-size: 14px;
            padding: 5px;
        }
        #recentProjectsList::item {
            padding: 10px;
            border-bottom: 1px solid #ecf0f1;
        }
        #recentProjectsList::item:hover {
            background-color: #f8f9fa;
        }
        #recentProjectsList::item:selected {
            background-color: #3498db;
            color: white;
        }
        #statusLabel {
            font-size: 14px;
            padding: 10px;
            border-radius: 4px;
            margin-top: 10px;
        }
    )");
}

void ProjectHubWidget::onCreateNewProject()
{
    CreateProjectDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString projectName = dialog.projectName().trimmed();
        QString basePath = dialog.projectPath();

        // Validation
        if (projectName.isEmpty()) {
            showErrorMessage("Invalid Project Name", "Project name cannot be empty.");
            return;
        }

        // Check for invalid characters in project name
        QString invalidChars = "<>:\"/\\|?*";
        for (const QChar &ch : invalidChars) {
            if (projectName.contains(ch)) {
                showErrorMessage("Invalid Project Name",
                    QString("Project name contains invalid character: '%1'").arg(ch));
                return;
            }
        }

        QString fullProjectPath = QDir(basePath).absoluteFilePath(projectName);

        // Check if directory already exists
        if (QDir(fullProjectPath).exists()) {
            auto reply = QMessageBox::question(this, "Directory Exists",
                QString("Directory '%1' already exists. Do you want to use it anyway?").arg(fullProjectPath),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return;
            }
        }

        // Check write permissions
        QFileInfo dirInfo(basePath);
        if (!dirInfo.isWritable()) {
            showErrorMessage("Permission Denied",
                "You don't have write permissions to the selected location.");
            return;
        }

        try {
            QString projectPath = m_projectManager->createProject(projectName, basePath);
            if (!projectPath.isEmpty()) {
                m_recentManager->addProject(projectPath);
                refreshRecentProjects();
                showSuccessMessage(QString("Project '%1' created successfully!").arg(projectName));

                // Small delay before opening to show success message
                QTimer::singleShot(1000, [this, projectPath]() {
                    emit projectOpened(projectPath);
                });
            }
        } catch (const std::exception &e) {
            showErrorMessage("Project Creation Failed", e.what());
        }
    }
}

void ProjectHubWidget::onOpenProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(this, "Select Project Folder");
    if (!projectPath.isEmpty()) {
        openProjectFromPath(projectPath);
    }
}

void ProjectHubWidget::onRecentProjectClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString projectPath = item->data(Qt::UserRole).toString();
    // Single click just selects, double click opens
}

void ProjectHubWidget::onRecentProjectDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString projectPath = item->data(Qt::UserRole).toString();
    openProjectFromPath(projectPath);
}

void ProjectHubWidget::openProjectFromPath(const QString &projectPath)
{
    if (m_projectManager->isValidProject(projectPath)) {
        m_recentManager->addProject(projectPath);
        refreshRecentProjects();
        emit projectOpened(projectPath);
    } else {
        showErrorMessage("Invalid Project", "Selected folder is not a valid project.");
    }
}

void ProjectHubWidget::refreshRecentProjects()
{
    m_recentProjectsList->clear();

    const QStringList recentProjects = m_recentManager->getRecentProjects();

    for (const QString &projectPath : recentProjects) {
        QString displayName = RecentProjectsManager::getProjectDisplayName(projectPath);

        auto *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, projectPath);
        item->setToolTip(projectPath);

        // Check if project still exists
        if (!QDir(projectPath).exists()) {
            item->setForeground(QBrush(QColor(128, 128, 128))); // Gray out invalid projects
            item->setToolTip(projectPath + " (Project not found)");
        }

        m_recentProjectsList->addItem(item);
    }

    // Start validation timer
    m_validationTimer->start();
}

void ProjectHubWidget::validateRecentProjects()
{
    QStringList recentProjects = m_recentManager->getRecentProjects();
    QStringList validProjects;

    for (const QString &projectPath : recentProjects) {
        if (m_projectManager->isValidProject(projectPath)) {
            validProjects.append(projectPath);
        }
    }

    // Update the list if any invalid projects were found
    if (validProjects.size() != recentProjects.size()) {
        m_recentManager->setRecentProjects(validProjects);
        refreshRecentProjects();
    }
}

void ProjectHubWidget::showErrorMessage(const QString &title, const QString &message)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("background-color: #e74c3c; color: white;");
    m_statusLabel->show();

    QTimer::singleShot(5000, [this]() {
        m_statusLabel->hide();
    });

    QMessageBox::warning(this, title, message);
}

void ProjectHubWidget::showSuccessMessage(const QString &message)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("background-color: #27ae60; color: white;");
    m_statusLabel->show();

    QTimer::singleShot(3000, [this]() {
        m_statusLabel->hide();
    });
}
