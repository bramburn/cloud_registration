#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void exitApplication();
    void showAbout();

private:
    void setupMenuBar();
    void setupStatusBar();
};

#endif // MAINWINDOW_H
