#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

/**
 * @brief ProjectTreeModel - Model for project tree view
 * 
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class ProjectTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit ProjectTreeModel(QObject *parent = nullptr);
    virtual ~ProjectTreeModel() = default;

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    // Stub implementation
};

#endif // PROJECTTREEMODEL_H
