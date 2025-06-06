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
#include "e57writer_lib.h"

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

    // Check for Qt DLLs in the application directory
    QString appDir = QCoreApplication::applicationDirPath();
    qDebug() << "Checking for Qt DLLs in:" << appDir;

    QStringList requiredDlls = {
        "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6OpenGL.dll", "Qt6OpenGLWidgets.dll"
    };

    for (const QString& dll : requiredDlls) {
        QString dllPath = appDir + "/" + dll;
        if (QFile::exists(dllPath)) {
            qDebug() << "Found:" << dll;
        } else {
            qWarning() << "Missing:" << dll;
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

        // Sprint 1 Decoupling: Create E57 parser and inject into main window
        qDebug() << "Creating E57 parser...";
        E57ParserLib* e57Parser = new E57ParserLib();
        qDebug() << "E57 parser created";

        // Sprint 2 Decoupling: Create E57 writer (for future use)
        qDebug() << "Creating E57 writer...";
        E57WriterLib* e57Writer = new E57WriterLib();
        qDebug() << "E57 writer created";

        // Create and show main window with dependency injection
        qDebug() << "Creating main window with injected E57 parser...";
        MainWindow window(e57Parser);
        qDebug() << "Main window created, showing...";
        window.show();
        qDebug() << "Main window shown successfully";

        // Note: E57Writer is created but not currently injected into MainWindow
        // This is prepared for future sprints that may require E57 export functionality

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
