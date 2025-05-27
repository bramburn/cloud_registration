#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QThread>
#include <vector>

class PointCloudViewerWidget;
class E57Parser;
class LasParser;
struct LasHeaderMetadata;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFileClicked();
    void onLoadingFinished(bool success, const QString& message);
    void onParsingProgressUpdated(int percentage);
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points);

    // Settings dialog slot
    void onLoadingSettingsTriggered();

    // Header metadata slot
    void onLasHeaderParsed(const LasHeaderMetadata& metadata);

    // View control slots
    void onTopViewClicked();
    void onLeftViewClicked();
    void onRightViewClicked();
    void onBottomViewClicked();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();

    // Helper methods for cleanup and UI updates
    void cleanupParsingThread();
    void cleanupProgressDialog();
    void updateUIAfterParsing(bool success, const QString& message);

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_openFileButton;
    QPushButton *m_topViewButton;
    QPushButton *m_leftViewButton;
    QPushButton *m_rightViewButton;
    QPushButton *m_bottomViewButton;
    PointCloudViewerWidget *m_viewer;
    QProgressDialog *m_progressDialog;

    // Menu actions
    QAction *m_loadingSettingsAction;
    QAction *m_topViewAction;
    QAction *m_leftViewAction;
    QAction *m_rightViewAction;
    QAction *m_bottomViewAction;

    // Data processing
    E57Parser *m_e57Parser;
    LasParser *m_lasParser;

    // Threading
    QThread *m_parserThread;

    // State
    QString m_currentFilePath;
    bool m_isLoading;
};

#endif // MAINWINDOW_H
