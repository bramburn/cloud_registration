#include <QApplication>
#include <QSurfaceFormat>
#include <QMetaType>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QGuiApplication>
#include <vector>
#include "mainwindow.h"
#include "e57parserlib.h"


// Enable logging
Q_LOGGING_CATEGORY(appLog, "CloudRegistration")

// Global log file
static QFile* g_logFile = nullptr;
static QTextStream* g_logStream = nullptr;

void messageOutput(QtMsgType type, const QMessageLogContext &/*context*/, const QString &msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("Info: %1").arg(msg);
        break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logLine = QString("[%1] %2").arg(timestamp, txt);

    // Write to console
    fprintf(stderr, "%s\n", logLine.toLocal8Bit().constData());

    // Write to file if available
    if (g_logStream) {
        *g_logStream << logLine << Qt::endl;
        g_logStream->flush();
    }
}

void setupLogging()
{
    // Create logs directory in application directory for easier access
    QString appDir = QCoreApplication::applicationDirPath();
    QString logFile = appDir + "/CloudRegistration.log";

    // Set up file logging
    g_logFile = new QFile(logFile);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        g_logStream = new QTextStream(g_logFile);
        qInstallMessageHandler(messageOutput);

        qDebug() << "=== CloudRegistration Started ===" << QDateTime::currentDateTime();
        qDebug() << "Log file location:" << logFile;
        qDebug() << "Application directory:" << appDir;
        qDebug() << "Working directory:" << QDir::currentPath();
    } else {
        fprintf(stderr, "Failed to open log file: %s\n", logFile.toLocal8Bit().constData());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties for logging and QSettings
    app.setApplicationName("CloudRegistration");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CloudRegistrationApp");

    // Setup logging first
    setupLogging();

    // Register custom types for thread-safe signal/slot communication
    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    qDebug() << "Registered custom types";

    // Log Qt and system information
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Qt Runtime Version:" << qVersion();
    qDebug() << "Platform:" << QGuiApplication::platformName();

    // Check for Qt libraries in the application directory (platform-specific)
    QString appDir = QCoreApplication::applicationDirPath();
    qDebug() << "Checking for Qt libraries in:" << appDir;

#ifdef Q_OS_WIN
    QStringList requiredLibs = {
        "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6OpenGL.dll", "Qt6OpenGLWidgets.dll"
    };
    QString libExtension = ".dll";
#elif defined(Q_OS_LINUX)
    QStringList requiredLibs = {
        "libQt6Core.so.6", "libQt6Gui.so.6", "libQt6Widgets.so.6", "libQt6OpenGL.so.6", "libQt6OpenGLWidgets.so.6"
    };
    QString libExtension = ".so";
#elif defined(Q_OS_MACOS)
    QStringList requiredLibs = {
        "libQt6Core.6.dylib", "libQt6Gui.6.dylib", "libQt6Widgets.6.dylib", "libQt6OpenGL.6.dylib", "libQt6OpenGLWidgets.6.dylib"
    };
    QString libExtension = ".dylib";
#else
    QStringList requiredLibs;
    QString libExtension = "";
#endif

    for (const QString& lib : requiredLibs) {
        QString libPath = appDir + "/" + lib;
        if (QFile::exists(libPath)) {
            qDebug() << "Found:" << lib;
        } else {
            qDebug() << "Not found locally:" << lib << "(may be system-installed)";
        }
    }

    try {
        // Set up OpenGL format for better compatibility
        qDebug() << "Setting up OpenGL format...";
        QSurfaceFormat format;
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(format);
        qDebug() << "OpenGL format configured successfully";

        // Create E57 parser and inject into main window
        qDebug() << "Creating E57 parser...";
        E57ParserLib* e57Parser = new E57ParserLib();
        qDebug() << "E57 parser created";

        // Create and show main window with dependency injection
        qDebug() << "Creating main window with injected E57 parser...";
        MainWindow window(e57Parser);
        qDebug() << "Main window created, showing...";
        window.show();
        qDebug() << "Main window shown successfully";

        qDebug() << "Starting application event loop...";
        int result = app.exec();
        qDebug() << "Application finished with code:" << result;
        return result;

    } catch (const std::exception& e) {
        qCritical() << "Exception caught in main:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application failed to start: %1").arg(e.what()));
        return -1;
    } catch (...) {
        qCritical() << "Unknown exception caught in main";
        QMessageBox::critical(nullptr, "Fatal Error",
            "Application failed to start due to unknown error");
        return -1;
    }
}
