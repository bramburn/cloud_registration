#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Point Cloud Application");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Point Cloud Solutions");
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
