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
#include <QMutex>
#include <QMutexLocker>
#include <QFileInfo>
#include <vector>
#include "mainwindow.h"
#include "e57parserlib.h"
#include "registration/WorkflowStateMachine.h"
#include "registration/Target.h"
#include "registration/TargetCorrespondence.h"
#include "registration/RegistrationProject.h"


// Enable logging
Q_LOGGING_CATEGORY(appLog, "CloudRegistration")

// Global log file and rotation settings
static QFile* g_logFile = nullptr;
static QTextStream* g_logStream = nullptr;
static QMutex g_logMutex;
static const qint64 MAX_LOG_SIZE = 5 * 1024 * 1024; // 5MB
static QString g_logFilePath;

void rotateLogFile() {
    if (!g_logFile) return;

    QFileInfo logInfo(*g_logFile);
    if (logInfo.size() < MAX_LOG_SIZE) return;

    // Close current log
    if (g_logStream) {
        delete g_logStream;
        g_logStream = nullptr;
    }
    if (g_logFile) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }

    // Rotate log files
    QString basePath = g_logFilePath;
    QString rotatedPath = basePath + ".1";

    // Remove old rotated log if exists
    if (QFile::exists(rotatedPath)) {
        QFile::remove(rotatedPath);
    }

    // Rename current log to .1
    QFile::rename(basePath, rotatedPath);

    // Create new log file
    g_logFile = new QFile(g_logFilePath);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        g_logStream = new QTextStream(g_logFile);
    }
}

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&g_logMutex);

    QString txt;
    QString level;
    switch (type) {
    case QtDebugMsg:
        txt = QString("DEBUG: %1").arg(msg);
        level = "DEBUG";
        break;
    case QtWarningMsg:
        txt = QString("WARN: %1").arg(msg);
        level = "WARN";
        break;
    case QtCriticalMsg:
        txt = QString("ERROR: %1").arg(msg);
        level = "ERROR";
        break;
    case QtFatalMsg:
        txt = QString("FATAL: %1").arg(msg);
        level = "FATAL";
        break;
    case QtInfoMsg:
        txt = QString("INFO: %1").arg(msg);
        level = "INFO";
        break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString category = context.category ? QString(context.category) : "default";
    QString logLine = QString("[%1] [%2] [%3] %4").arg(timestamp, level, category, msg);

    // Write to console (only for debug builds or important messages)
#ifdef QT_DEBUG
    fprintf(stderr, "%s\n", logLine.toLocal8Bit().constData());
#else
    if (type >= QtWarningMsg) {
        fprintf(stderr, "%s\n", logLine.toLocal8Bit().constData());
    }
#endif

    // Write to file if available
    if (g_logStream) {
        // Check for log rotation
        rotateLogFile();

        if (g_logStream) {
            *g_logStream << logLine << Qt::endl;
            g_logStream->flush();
        }
    }
}

void setupLogging()
{
    // Determine log directory based on platform and environment
    QString logDir;

    // Check if we're in a Docker container or have CLOUDREGISTRATION_LOG_DIR set
    QString envLogDir = qgetenv("CLOUDREGISTRATION_LOG_DIR");
    if (!envLogDir.isEmpty()) {
        logDir = envLogDir;
    } else {
        // Use platform-appropriate location
        QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (appDataDir.isEmpty()) {
            // Fallback to application directory
            logDir = QCoreApplication::applicationDirPath() + "/logs";
        } else {
            logDir = appDataDir + "/logs";
        }
    }

    // Create log directory if it doesn't exist
    QDir().mkpath(logDir);

    g_logFilePath = logDir + "/CloudRegistration.log";

    // Set up file logging
    g_logFile = new QFile(g_logFilePath);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        g_logStream = new QTextStream(g_logFile);
        qInstallMessageHandler(messageOutput);

        qInfo() << "=== CloudRegistration Started ===" << QDateTime::currentDateTime();
        qInfo() << "Version:" << QCoreApplication::applicationVersion();
        qInfo() << "Log file location:" << g_logFilePath;
        qInfo() << "Log directory:" << logDir;
        qInfo() << "Application directory:" << QCoreApplication::applicationDirPath();
        qInfo() << "Working directory:" << QDir::currentPath();
        qInfo() << "Data directory:" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

        // Log system information
        qInfo() << "Platform:" << QGuiApplication::platformName();
        qInfo() << "Qt Version:" << QT_VERSION_STR;
        qInfo() << "Qt Runtime Version:" << qVersion();

    } else {
        fprintf(stderr, "Failed to open log file: %s\n", g_logFilePath.toLocal8Bit().constData());
        fprintf(stderr, "Continuing without file logging...\n");
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

    // Register Sprint 2 registration workflow types
    qRegisterMetaType<RegistrationStep>("RegistrationStep");
    qRegisterMetaType<Target*>("Target*");
    qRegisterMetaType<SphereTarget*>("SphereTarget*");
    qRegisterMetaType<CheckerboardTarget*>("CheckerboardTarget*");
    qRegisterMetaType<NaturalPointTarget*>("NaturalPointTarget*");
    qRegisterMetaType<TargetCorrespondence>("TargetCorrespondence");
    qRegisterMetaType<ScanInfo>("ScanInfo");
    qRegisterMetaType<RegistrationProject::RegistrationResult>("RegistrationProject::RegistrationResult");
    qRegisterMetaType<RegistrationProject::RegistrationState>("RegistrationProject::RegistrationState");

    qDebug() << "Registered custom types including registration workflow types";

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
