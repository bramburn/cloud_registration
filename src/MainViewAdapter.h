#ifndef MAINVIEWADAPTER_H
#define MAINVIEWADAPTER_H

#include "IMainView.h"
#include <QObject>

class MainWindow;

/**
 * @brief Adapter class that implements IMainView interface and delegates to MainWindow
 * 
 * This adapter solves the multiple inheritance problem by using composition instead.
 * It implements the IMainView interface and forwards all calls to the actual MainWindow.
 * This allows MainWindow to remain a pure QMainWindow while still providing the
 * IMainView interface for the MVP pattern.
 */
class MainViewAdapter : public IMainView
{
    Q_OBJECT

public:
    explicit MainViewAdapter(MainWindow* mainWindow, QObject* parent = nullptr);
    ~MainViewAdapter() override = default;

    // IMainView interface implementation - all methods delegate to MainWindow
    void displayErrorMessage(const QString& title, const QString& message) override;
    void displayInfoMessage(const QString& title, const QString& message) override;
    void displayWarningMessage(const QString& title, const QString& message) override;
    void updateStatusBar(const QString& text) override;
    void setWindowTitle(const QString& title) override;
    IPointCloudViewer* getViewer() override;
    void showProgressDialog(bool show, const QString& title = QString(), const QString& message = QString()) override;
    void updateProgress(int percentage, const QString& message) override;
    void setActionsEnabled(bool enabled) override;
    void setProjectTitle(const QString& projectName) override;
    void updateScanList(const QStringList& scanNames) override;
    void highlightScan(const QString& scanName) override;
    void showProjectHub() override;
    void showProjectView() override;
    void updateMemoryUsage(size_t totalBytes) override;
    void updateRenderingStats(float fps, int visiblePoints) override;
    QString askForOpenFilePath(const QString& title, const QString& filter) override;
    QString askForSaveFilePath(const QString& title, const QString& filter, const QString& defaultName = QString()) override;
    bool askForConfirmation(const QString& title, const QString& message) override;

signals:
    // Forward signals from MainWindow to presenter
    void newProjectRequested();
    void openProjectRequested();
    void closeProjectRequested();
    void importScansRequested();
    void fileOpenRequested();
    void scanActivated(const QString& scanId);
    void viewChangeRequested(const QString& viewName);

private:
    MainWindow* m_mainWindow;  // Not owned by this class
};

#endif // MAINVIEWADAPTER_H
