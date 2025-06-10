#include <QApplication>
#include "mainwindow_simple.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Cloud Registration");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Cloud Registration Team");

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
