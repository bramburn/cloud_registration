#ifndef MAINWINDOW_SIMPLE_H
#define MAINWINDOW_SIMPLE_H

#include <QMainWindow>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    
    QLabel *m_centralLabel;
};

#endif // MAINWINDOW_SIMPLE_H
