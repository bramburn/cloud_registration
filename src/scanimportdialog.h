#ifndef SCANIMPORTDIALOG_H
#define SCANIMPORTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>

enum class ImportMode;

class ScanImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanImportDialog(QWidget *parent = nullptr);
    
    QStringList selectedFiles() const { return m_selectedFiles; }
    ImportMode importMode() const;
    
    void setProjectPath(const QString &projectPath);

private slots:
    void browseFiles();
    void removeSelectedFiles();
    void updateUI();
    void accept() override;

private:
    void setupUI();
    void setupConnections();
    void addFilesToList(const QStringList &files);
    bool validateSelection();
    
    QListWidget *m_fileList;
    QRadioButton *m_copyModeRadio;
    QRadioButton *m_moveModeRadio;
    QPushButton *m_browseButton;
    QPushButton *m_removeButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QLabel *m_infoLabel;
    QLabel *m_targetLabel;
    
    QStringList m_selectedFiles;
    QString m_projectPath;
};

#endif // SCANIMPORTDIALOG_H
