#include "sidebarwidget.h"
#include "projecttreemodel.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include <QStandardItem>
#include <QHeaderView>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QTreeView(parent)
    , m_model(nullptr)
{
    setupUI();
}

void SidebarWidget::setupUI()
{
    m_model = new ProjectTreeModel(this);
    setModel(m_model);
    setHeaderHidden(true);
    setMinimumWidth(200);
    setMaximumWidth(400);
    
    // Styling similar to modern IDEs
    setStyleSheet(R"(
        QTreeView {
            background-color: #2b2b2b;
            color: #ffffff;
            font-size: 14px;
            border: none;
            outline: none;
        }
        QTreeView::item {
            height: 30px;
            border: none;
            padding-left: 4px;
        }
        QTreeView::item:selected {
            background-color: #3d4348;
            color: #ffffff;
        }
        QTreeView::item:hover {
            background-color: #404040;
        }
        QTreeView::branch {
            background: transparent;
        }
        QTreeView::branch:has-children:!has-siblings:closed,
        QTreeView::branch:closed:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/branch-closed.png);
        }
        QTreeView::branch:open:has-children:!has-siblings,
        QTreeView::branch:open:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/branch-open.png);
        }
    )");
    
    // Set selection behavior
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void SidebarWidget::setProject(const QString &projectName, const QString &projectPath)
{
    m_currentProjectPath = projectPath;
    m_model->setProject(projectName, projectPath);
    expandAll();
}

void SidebarWidget::clearProject()
{
    m_currentProjectPath.clear();
    m_model->clear();
}

void SidebarWidget::setSQLiteManager(SQLiteManager *manager)
{
    m_model->setSQLiteManager(manager);
}

void SidebarWidget::refreshFromDatabase()
{
    if (m_model) {
        m_model->refreshScans();
        expandAll();
    }
}

void SidebarWidget::addScan(const ScanInfo &scan)
{
    if (m_model) {
        m_model->addScan(scan);
        expandAll();
    }
}
