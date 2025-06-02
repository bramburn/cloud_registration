#ifndef RELINKFILEDIALOG_H
#define RELINKFILEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>

namespace SceneRegistration {

class RelinkFileDialog : public QDialog {
    Q_OBJECT

public:
    explicit RelinkFileDialog(QWidget* parent = nullptr);
    
    void setScanInfo(const QString& scanName, const QString& originalPath);
    QString getSelectedFilePath() const;
    
    static QString relinkScanFile(QWidget* parent, const QString& scanName, 
                                 const QString& originalPath);

private slots:
    void onBrowseClicked();
    void onPathChanged();
    void onAccept();

private:
    void setupUI();
    void updateValidation();
    
    QLabel* m_instructionLabel;
    QLabel* m_scanNameLabel;
    QLabel* m_originalPathLabel;
    QLabel* m_newPathLabel;
    QLineEdit* m_pathEdit;
    QPushButton* m_browseButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QLabel* m_validationLabel;
    
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_pathLayout;
    QHBoxLayout* m_buttonLayout;
    
    QString m_scanName;
    QString m_originalPath;
    QString m_selectedPath;
    bool m_isValidPath;
};

} // namespace SceneRegistration

#endif // RELINKFILEDIALOG_H
