#ifndef CREATEPROJECTDIALOG_H
#define CREATEPROJECTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class CreateProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateProjectDialog(QWidget* parent = nullptr);

    QString projectName() const
    {
        return m_nameEdit->text();
    }
    QString projectPath() const
    {
        return m_pathEdit->text();
    }

private slots:
    void onBrowseClicked();
    void onAcceptClicked();
    void onNameChanged();
    void onPathChanged();

private:
    void setupUI();
    void validateInput();

    QLineEdit* m_nameEdit;
    QLineEdit* m_pathEdit;
    QPushButton* m_browseBtn;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    QLabel* m_errorLabel;
};

#endif  // CREATEPROJECTDIALOG_H
