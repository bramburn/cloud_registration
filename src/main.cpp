#include <QApplication>
#include <QSurfaceFormat>
#include <QMetaType>
#include <vector>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Register custom types for thread-safe signal/slot communication
    qRegisterMetaType<std::vector<float>>("std::vector<float>");

    // Set up OpenGL format for better compatibility
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
