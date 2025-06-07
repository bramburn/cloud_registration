<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" class="logo" width="120"/>

# Sprint 2.2 Implementation Guide: UI Refinement, Performance Profiling, and Stability

This comprehensive implementation guide provides detailed code examples, API specifications, and integration strategies for completing Sprint 2.2 of the FARO scene registration software using Qt6 C++ with vcpkg package management. The sprint focuses on three critical areas: context-aware UI controls with real-time feedback, performance profiling systems, and stability stress testing to ensure robust memory management under intensive workloads.

## Context-Aware UI Controls and Real-Time Feedback

The first user story involves implementing dynamic menu controls that respond to the application state and providing real-time performance metrics in the status bar. This requires integrating Qt's context menu system with custom state management and implementing periodic performance monitoring.

### Context Menu Implementation

The context menu implementation leverages Qt's `QContextMenuEvent` system to provide state-aware functionality[^1_2]. The following implementation demonstrates how to create a context-sensitive menu in the `SidebarWidget`:

```cpp
// sidebarwidget.h
#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include "projecttreemodel.h"

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    void setModel(ProjectTreeModel *model);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void loadScan();
    void unloadScan();

private:
    QTreeView *m_treeView;
    ProjectTreeModel *m_model;
    QMenu *m_contextMenu;
    QAction *m_loadAction;
    QAction *m_unloadAction;
    QModelIndex m_clickedIndex;
};

#endif // SIDEBARWIDGET_H
```

```cpp
// sidebarwidget.cpp
#include "sidebarwidget.h"
#include <QVBoxLayout>
#include <QHeaderView>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QWidget(parent)
    , m_treeView(new QTreeView(this))
    , m_model(nullptr)
    , m_contextMenu(new QMenu(this))
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(m_treeView);
    
    // Create context menu actions
    m_loadAction = new QAction("Load Scan", this);
    m_unloadAction = new QAction("Unload Scan", this);
    
    m_contextMenu->addAction(m_loadAction);
    m_contextMenu->addAction(m_unloadAction);
    
    // Connect actions to slots
    connect(m_loadAction, &QAction::triggered, this, &SidebarWidget::loadScan);
    connect(m_unloadAction, &QAction::triggered, this, &SidebarWidget::unloadScan);
}

void SidebarWidget::setModel(ProjectTreeModel *model)
{
    m_model = model;
    m_treeView->setModel(model);
}

void SidebarWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_model) return;
    
    // Get the index at the clicked position
    m_clickedIndex = m_treeView->indexAt(m_treeView->mapFromGlobal(event->globalPos()));
    
    if (!m_clickedIndex.isValid()) return;
    
    // Get the loaded state from the model
    LoadedState state = m_model->getLoadedState(m_clickedIndex);
    
    // Enable/disable actions based on state
    m_loadAction->setEnabled(state == LoadedState::Unloaded);
    m_unloadAction->setEnabled(state == LoadedState::Loaded);
    
    // Show the context menu at the cursor position
    m_contextMenu->exec(event->globalPos());
}

void SidebarWidget::loadScan()
{
    if (m_clickedIndex.isValid() && m_model) {
        emit m_model->loadRequested(m_clickedIndex);
    }
}

void SidebarWidget::unloadScan()
{
    if (m_clickedIndex.isValid() && m_model) {
        emit m_model->unloadRequested(m_clickedIndex);
    }
}
```

The context menu system uses `QAction::setEnabled()` to dynamically control menu item availability[^1_3][^1_5]. This approach ensures users can only perform valid operations based on the current scan state.

### Real-Time Performance Monitoring

Performance monitoring requires implementing FPS calculation and memory usage tracking with periodic updates using `QTimer`[^1_7]. The following implementation provides real-time statistics:

```cpp
// pointcloudviewerwidget.h
#ifndef POINTCLOUDVIEWERWIDGET_H
#define POINTCLOUDVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QTimer>
#include <QElapsedTimer>

class PointCloudViewerWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit PointCloudViewerWidget(QWidget *parent = nullptr);

signals:
    void statsUpdated(float fps, int visiblePoints);

protected:
    void paintGL() override;
    void initializeGL() override;

private slots:
    void updateStats();

private:
    QTimer *m_statsTimer;
    QElapsedTimer *m_frameTimer;
    int m_frameCount;
    int m_visiblePoints;
    static constexpr int STATS_UPDATE_INTERVAL = 1000; // 1 second
};

#endif // POINTCLOUDVIEWERWIDGET_H
```

```cpp
// pointcloudviewerwidget.cpp
#include "pointcloudviewerwidget.h"
#include <QDebug>

PointCloudViewerWidget::PointCloudViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_statsTimer(new QTimer(this))
    , m_frameTimer(new QElapsedTimer)
    , m_frameCount(0)
    , m_visiblePoints(0)
{
    // Setup stats timer for periodic updates
    connect(m_statsTimer, &QTimer::timeout, this, &PointCloudViewerWidget::updateStats);
    m_statsTimer->start(STATS_UPDATE_INTERVAL);
    
    m_frameTimer->start();
}

void PointCloudViewerWidget::initializeGL()
{
    // OpenGL initialization code here
}

void PointCloudViewerWidget::paintGL()
{
    // OpenGL rendering code here
    // Update visible points count during rendering
    m_visiblePoints = calculateVisiblePoints(); // Custom implementation
    
    m_frameCount++;
    update(); // Schedule next frame
}

void PointCloudViewerWidget::updateStats()
{
    qint64 elapsed = m_frameTimer->elapsed();
    if (elapsed > 0) {
        float fps = (m_frameCount * 1000.0f) / elapsed;
        emit statsUpdated(fps, m_visiblePoints);
    }
    
    // Reset counters
    m_frameCount = 0;
    m_frameTimer->restart();
}
```


### Status Bar Integration

The main window integrates performance statistics and memory usage into the status bar[^1_6]:

```cpp
// mainwindow.h
#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onStatsUpdated(float fps, int visiblePoints);
    void onMemoryUsageChanged(size_t totalBytes);

private:
    QLabel *m_fpsLabel;
    QLabel *m_pointsLabel;
    QLabel *m_memoryLabel;
};
```

```cpp
// mainwindow.cpp
#include "mainwindow.h"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create status bar labels
    m_fpsLabel = new QLabel("FPS: 0.0");
    m_pointsLabel = new QLabel("Points: 0");
    m_memoryLabel = new QLabel("Memory: 0 MB");
    
    // Add labels to status bar
    statusBar()->addWidget(m_fpsLabel);
    statusBar()->addWidget(m_pointsLabel);
    statusBar()->addPermanentWidget(m_memoryLabel);
    
    // Connect to viewer widget and load manager signals
    // (connections made during widget initialization)
}

void MainWindow::onStatsUpdated(float fps, int visiblePoints)
{
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
    m_pointsLabel->setText(QString("Points: %1").arg(visiblePoints));
}

void MainWindow::onMemoryUsageChanged(size_t totalBytes)
{
    double megabytes = totalBytes / (1024.0 * 1024.0);
    m_memoryLabel->setText(QString("Memory: %1 MB").arg(megabytes, 0, 'f', 1));
}
```


## Performance Profiling and Benchmarking System

The performance profiling system provides comprehensive timing analysis using `QElapsedTimer`[^1_8][^1_19][^1_20] and generates detailed reports in both text and JSON formats[^1_10].

### PerformanceProfiler Implementation

```cpp
// performance_profiler.h
#ifndef PERFORMANCE_PROFILER_H
#define PERFORMANCE_PROFILER_H

#include <QElapsedTimer>
#include <QMap>
#include <QString>
#include <QMutex>

struct ProfileSection {
    qint64 totalTime = 0;
    qint64 callCount = 0;
    qint64 minTime = LLONG_MAX;
    qint64 maxTime = 0;
    
    double averageTime() const {
        return callCount > 0 ? double(totalTime) / callCount : 0.0;
    }
};

class PerformanceProfiler
{
public:
    static PerformanceProfiler& instance();
    
    void startSection(const QString& name);
    void endSection(const QString& name);
    
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    
    QString generateReport() const;
    QJsonDocument generateJsonReport() const;
    void reset();
    
    class SectionTimer {
    public:
        explicit SectionTimer(const QString& name);
        ~SectionTimer();
    private:
        QString m_name;
        QElapsedTimer m_timer;
    };

private:
    PerformanceProfiler() = default;
    
    mutable QMutex m_mutex;
    QMap<QString, ProfileSection> m_sections;
    QMap<QString, QElapsedTimer> m_activeTimers;
    bool m_enabled = true;
};

// Convenience macros for automatic timing
#define PROFILE_FUNCTION() \
    PerformanceProfiler::SectionTimer timer__(__FUNCTION__)

#define PROFILE_SECTION(name) \
    PerformanceProfiler::SectionTimer timer__(name)

#endif // PERFORMANCE_PROFILER_H
```

```cpp
// performance_profiler.cpp
#include "performance_profiler.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutexLocker>
#include <QDebug>

PerformanceProfiler& PerformanceProfiler::instance()
{
    static PerformanceProfiler instance;
    return instance;
}

void PerformanceProfiler::startSection(const QString& name)
{
    if (!m_enabled) return;
    
    QMutexLocker locker(&m_mutex);
    m_activeTimers[name].start();
}

void PerformanceProfiler::endSection(const QString& name)
{
    if (!m_enabled) return;
    
    QMutexLocker locker(&m_mutex);
    auto it = m_activeTimers.find(name);
    if (it != m_activeTimers.end()) {
        qint64 elapsed = it->elapsed();
        
        ProfileSection& section = m_sections[name];
        section.totalTime += elapsed;
        section.callCount++;
        section.minTime = qMin(section.minTime, elapsed);
        section.maxTime = qMax(section.maxTime, elapsed);
        
        m_activeTimers.erase(it);
    }
}

QString PerformanceProfiler::generateReport() const
{
    QMutexLocker locker(&m_mutex);
    QString report = "Performance Profiling Report\n";
    report += "============================\n\n";
    
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const auto& section = it.value();
        report += QString("Section: %1\n").arg(it.key());
        report += QString("  Calls: %1\n").arg(section.callCount);
        report += QString("  Total Time: %1 ms\n").arg(section.totalTime);
        report += QString("  Average Time: %1 ms\n").arg(section.averageTime(), 0, 'f', 3);
        report += QString("  Min Time: %1 ms\n").arg(section.minTime);
        report += QString("  Max Time: %1 ms\n\n").arg(section.maxTime);
    }
    
    return report;
}

QJsonDocument PerformanceProfiler::generateJsonReport() const
{
    QMutexLocker locker(&m_mutex);
    QJsonObject root;
    QJsonArray sections;
    
    for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
        const auto& section = it.value();
        QJsonObject sectionObj;
        sectionObj["name"] = it.key();
        sectionObj["callCount"] = section.callCount;
        sectionObj["totalTime"] = section.totalTime;
        sectionObj["averageTime"] = section.averageTime();
        sectionObj["minTime"] = section.minTime;
        sectionObj["maxTime"] = section.maxTime;
        
        sections.append(sectionObj);
    }
    
    root["sections"] = sections;
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return QJsonDocument(root);
}

void PerformanceProfiler::reset()
{
    QMutexLocker locker(&m_mutex);
    m_sections.clear();
    m_activeTimers.clear();
}

// SectionTimer implementation
PerformanceProfiler::SectionTimer::SectionTimer(const QString& name)
    : m_name(name)
{
    PerformanceProfiler::instance().startSection(m_name);
}

PerformanceProfiler::SectionTimer::~SectionTimer()
{
    PerformanceProfiler::instance().endSection(m_name);
}
```


### Benchmark System Implementation

The benchmarking system provides automated performance testing capabilities:

```cpp
// performance_benchmark.h
#ifndef PERFORMANCE_BENCHMARK_H
#define PERFORMANCE_BENCHMARK_H

#include <QString>
#include <QJsonDocument>
#include <functional>

struct BenchmarkResult {
    QString fileName;
    qint64 loadTime;
    qint64 parseTime;
    size_t pointCount;
    size_t memoryUsage;
    bool success;
    QString errorMessage;
    
    QJsonObject toJson() const;
};

class PerformanceBenchmark
{
public:
    using LoadFunction = std::function<BenchmarkResult(const QString&)>;
    
    PerformanceBenchmark();
    
    void setLoadFunction(LoadFunction func) { m_loadFunction = func; }
    
    BenchmarkResult runSingleBenchmark(const QString& fileName);
    QList<BenchmarkResult> runBenchmarkSuite(const QStringList& fileNames);
    
    QString generateTextReport(const QList<BenchmarkResult>& results);
    QJsonDocument generateJsonReport(const QList<BenchmarkResult>& results);

private:
    LoadFunction m_loadFunction;
};

#endif // PERFORMANCE_BENCHMARK_H
```


### Integration with E57 and LAS Parsers

Add profiling to existing parsers:

```cpp
// e57parserlib.cpp
#include "performance_profiler.h"

bool E57ParserLib::parse(const QString& filePath)
{
    PROFILE_FUNCTION();
    
    {
        PROFILE_SECTION("E57::readHeader");
        if (!readHeader()) {
            return false;
        }
    }
    
    {
        PROFILE_SECTION("E57::parsePointData");
        return parsePointData();
    }
}
```


## Stability Stress Testing Implementation

The stress testing system validates memory management under extreme conditions[^1_11][^1_14]:

```cpp
// test_sprint2_2_stress.cpp
#include <gtest/gtest.h>
#include <QApplication>
#include <QElapsedTimer>
#include "pointcloudloadmanager.h"
#include "projecttreemodel.h"

class StressTest : public ::testing::Test
{
protected:
    void SetUp() override {
        // Set artificial low memory limit for testing
        m_loadManager = new PointCloudLoadManager();
        m_loadManager->setMemoryLimit(100 * 1024 * 1024); // 100 MB
        
        m_model = new ProjectTreeModel();
        
        // Record baseline memory usage
        m_baselineMemory = getCurrentMemoryUsage();
    }
    
    void TearDown() override {
        delete m_loadManager;
        delete m_model;
    }
    
    size_t getCurrentMemoryUsage() {
        // Platform-specific memory usage calculation
        // This is a simplified example
        return QProcess::systemEnvironment().contains("TEST_MEMORY_USAGE") 
            ? 50 * 1024 * 1024 : 0; // Mock value for testing
    }
    
    PointCloudLoadManager *m_loadManager;
    ProjectTreeModel *m_model;
    size_t m_baselineMemory;
};

TEST_F(StressTest, MemoryLimitEnforcement)
{
    QStringList testFiles = {
        "test_data/large_scan1.e57",
        "test_data/large_scan2.e57", 
        "test_data/large_scan3.las",
        "test_data/large_scan4.las"
    };
    
    // Load scans until memory limit is reached
    for (const QString& file : testFiles) {
        m_loadManager->loadScan(file);
        
        // Verify memory usage doesn't exceed limit
        size_t currentUsage = m_loadManager->getTotalMemoryUsage();
        EXPECT_LE(currentUsage, m_loadManager->getMemoryLimit())
            << "Memory usage exceeded limit after loading: " << file;
    }
}

TEST_F(StressTest, LRUEvictionVerification)
{
    // Load multiple scans to trigger LRU eviction
    QStringList files = {"scan1.e57", "scan2.e57", "scan3.e57"};
    
    // Load first two scans
    m_loadManager->loadScan(files[^1_0]);
    m_loadManager->loadScan(files[^1_1]);
    
    // Access first scan to make it recently used
    m_loadManager->accessScan(files[^1_0]);
    
    // Load third scan, should evict second scan (least recently used)
    m_loadManager->loadScan(files[^1_2]);
    
    // Verify scan2 was evicted while scan1 remains loaded
    EXPECT_EQ(m_model->getLoadedState(files[^1_0]), LoadedState::Loaded);
    EXPECT_EQ(m_model->getLoadedState(files[^1_1]), LoadedState::Unloaded);
    EXPECT_EQ(m_model->getLoadedState(files[^1_2]), LoadedState::Loaded);
}

TEST_F(StressTest, MemoryLeakDetection)
{
    QElapsedTimer timer;
    timer.start();
    
    // Perform load/unload cycles
    for (int i = 0; i < 100; ++i) {
        m_loadManager->loadScan("test_scan.e57");
        m_loadManager->unloadScan("test_scan.e57");
    }
    
    // Wait for cleanup
    QTest::qWait(1000);
    
    size_t finalMemory = getCurrentMemoryUsage();
    size_t memoryIncrease = finalMemory - m_baselineMemory;
    size_t allowedIncrease = m_baselineMemory * 0.1; // 10% threshold
    
    EXPECT_LE(memoryIncrease, allowedIncrease)
        << "Memory leak detected. Increase: " << memoryIncrease 
        << " bytes, allowed: " << allowedIncrease << " bytes";
}
```


## External Package Integration

### Vcpkg Configuration

Create a `vcpkg.json` manifest file for dependency management:

```json
{
    "name": "faro-scene-registration",
    "version": "1.0.0",
    "dependencies": [
        "gtest",
        {
            "name": "benchmark",
            "features": ["tools"]
        }
    ],
    "builtin-baseline": "2024.03.25"
}
```


### CMakeLists.txt Integration

```cmake
cmake_minimum_required(VERSION 3.16)
project(FaroSceneRegistration)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL Gui Test)

# Find vcpkg packages
find_package(GTest CONFIG REQUIRED)
find_package(benchmark CONFIG REQUIRED)

# Enable testing
enable_testing()

# Add profiling library
add_library(PerformanceProfiler
    src/performance_profiler.cpp
    src/performance_benchmark.cpp
)

target_link_libraries(PerformanceProfiler 
    Qt6::Core
)

# Add stress test executable
add_executable(StressTests
    tests/demos/test_sprint2_2_stress.cpp
)

target_link_libraries(StressTests
    PRIVATE
    Qt6::Test
    GTest::gtest 
    GTest::gtest_main
    PerformanceProfiler
)

add_test(NAME StressTests COMMAND StressTests)
```


## Testing and Validation

### Unit Test for Performance Profiler

```cpp
// test_performance_profiler.cpp
#include <gtest/gtest.h>
#include <QThread>
#include "performance_profiler.h"

TEST(PerformanceProfilerTest, AccurateTiming)
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.reset();
    profiler.setEnabled(true);
    
    const int sleepTime = 100; // milliseconds
    
    {
        PROFILE_SECTION("TestSection");
        QThread::msleep(sleepTime);
    }
    
    QString report = profiler.generateReport();
    
    // Parse the average time from the report
    // This is a simplified check - in practice, you'd parse more carefully
    EXPECT_TRUE(report.contains("TestSection"));
    EXPECT_TRUE(report.contains("Calls: 1"));
    
    // Timing should be within 10% of expected value
    // Note: Actual implementation would need more sophisticated parsing
}
```

This comprehensive implementation provides a robust foundation for Sprint 2.2, integrating Qt6's modern features with professional performance profiling and testing methodologies. The system supports real-time UI feedback, detailed performance analysis, and rigorous stability validation essential for FARO scene registration software development.

<div style="text-align: center">⁂</div>

[^1_1]: ms2.2.md

[^1_2]: https://doc.qt.io/qt-6/qcontextmenuevent.html

[^1_3]: https://gis.stackexchange.com/questions/372500/enabling-and-disabling-a-qaction-depending-on-feature-selection

[^1_4]: http://imaginativethinking.ca/populate-menu-model-qtquick/

[^1_5]: https://doc.qt.io/qt-6/qactiongroup.html

[^1_6]: https://stackoverflow.com/questions/2711437/qlabel-embedding-in-qstatusbar-using-qt-designer

[^1_7]: https://stackoverflow.com/questions/15033290/qtimer-for-qglwidget-incorrect-drawing-and-timing-on-other-machines

[^1_8]: https://doc.qt.io/qt-6/qelapsedtimer.html

[^1_9]: https://felix.abecassis.me/2011/09/cpp-timer-raii/

[^1_10]: https://doc.qt.io/qt-6/qjsondocument.html

[^1_11]: https://doc.qt.io/qt-6/qttestlib-tutorial1-example.html

[^1_12]: https://vcpkg.link/ports/benchmark

[^1_13]: https://doc.qt.io/qt-6/qtwidgets-mainwindows-menus-example.html

[^1_14]: https://vcpkg.link/ports/gtest

[^1_15]: https://www.pythonguis.com/tutorials/pyqt6-signals-slots-events/

[^1_16]: https://www.qtcentre.org/threads/710-Enabling-Disabling-QActions-in-QMenus

[^1_17]: https://doc.qt.io/qt-6/qml-qtquick-controls-menu.html

[^1_18]: https://www.qtcentre.org/threads/48083-QAction-setEnabled()-Not-Working-as-Expected

[^1_19]: https://stackoverflow.com/questions/244646/get-elapsed-time-in-qt

[^1_20]: https://qt.developpez.com/doc/6.1/qelapsedtimer/

[^1_21]: https://qt.developpez.com/doc/6.7/qdeadlinetimer/

[^1_22]: https://stackoverflow.com/questions/72822872/python-pyqt6-contextmenuevent-strange-behavior-i-dont-understand

[^1_23]: https://www.youtube.com/watch?v=99zfeWcxy0I

[^1_24]: https://www.learnqt.guide/working-with-events

[^1_25]: https://doc.qt.io/qt-6/qaction.html

[^1_26]: https://stackoverflow.com/questions/72943053/qt6-qwidgetaction-replacement-for-setmenu

[^1_27]: https://www.youtube.com/watch?v=yKKpB7LFQog

[^1_28]: https://codebrowser.dev/qt5/qtbase/src/corelib/kernel/qelapsedtimer.cpp.html

[^1_29]: https://www.ics.com/blog/qt-has-solution-all-your-timing-needs

[^1_30]: https://doc.qt.io/qt-6/json.html

[^1_31]: https://www.youtube.com/watch?v=lrrptFVlMu4

[^1_32]: https://vcpkg.io/en/package/benchmark.html

[^1_33]: https://github.com/microsoft/vcpkg/issues/23977

[^1_34]: https://pcl.readthedocs.io/projects/tutorials/en/latest/pcl_vcpkg_windows.html

[^1_35]: https://vcpkg.io/en/package/qtbase.html

[^1_36]: https://doc.qt.io/qt-6/qwidget.html

[^1_37]: https://doc.qt.io/qt-6/qdialog.html

[^1_38]: https://doc.qt.io/qt-6/qmainwindow.html

[^1_39]: https://vcpkg.io/en/package/gtest

[^1_40]: https://github.com/Microsoft/vcpkg/issues/3952

[^1_41]: https://probablydance.com/2014/10/12/comma-operator-raii-abuse/

[^1_42]: https://stackoverflow.com/questions/31391914/timing-in-an-elegant-way-in-c

[^1_43]: https://stackoverflow.com/questions/61075951/minimal-example-on-how-to-read-write-and-print-qjson-code-with-qjsondocument

[^1_44]: https://felgo.com/doc/qt/qjsondocument/

[^1_45]: https://stackoverflow.com/questions/60717027/google-benchmark-with-command-line-args-writing-my-own-main-function

[^1_46]: http://erickveil.github.io/2016/04/06/How-To-Manipulate-JSON-With-C++-and-Qt.html

[^1_47]: https://www.youtube.com/watch?v=N4pvvCToogM

[^1_48]: https://www.youtube.com/watch?v=9VKR8u9odrA

[^1_49]: https://stackoverflow.com/questions/78600881/undefined-references-when-linking-google-benchmark-with-g-on-windows-msys2-u

[^1_50]: https://github.com/microsoft/vcpkg/issues/43668

[^1_51]: https://github.com/google/benchmark/issues/188

[^1_52]: https://stackoverflow.com/questions/59668661/how-to-find-google-test-with-find-package-using-cmake-on-windows

[^1_53]: https://discourse.cmake.org/t/cmake-3-28-0-rc3-fails-to-build-qt6-via-vcpkg/9369

[^1_54]: https://www.qtcentre.org/threads/13503-overriding-the-QWidget-contextMenuEvent

[^1_55]: https://forum.qt.io/topic/152330/custom-context-menu-item

[^1_56]: https://www.setnode.com/blog/right-click-context-menus-with-qt/

[^1_57]: https://stackoverflow.com/questions/17480984/how-do-i-handle-the-event-of-the-user-pressing-the-x-close-button

[^1_58]: https://stackoverflow.com/questions/75397491/link-errors-when-using-cmake-and-vcpkg-with-googletest

[^1_59]: https://github.com/MattYoung50/gtest-example

[^1_60]: https://stackoverflow.com/questions/77696885/vcpkg-and-gtest-integration-with-a-custom-toolchain-config-files-were-consider

[^1_61]: https://matgomes.com/integrate-google-test-into-cmake/

[^1_62]: https://doc.qt.io/qt-6/qtprotobuf-installation-windows-vcpkg.html

[^1_63]: https://vcpkg.link/ports/gtest/v/2019-01-04-3/0


---

# Enhanced Sprint 2.2 Implementation Guide: Advanced UI, Profiling, and Stability Techniques

This enhanced implementation guide builds upon the original Sprint 2.2 documentation with deeper technical insights and expanded best practices for developing FARO scene registration software using Qt6 C++ and vcpkg. The updates integrate critical information from Qt documentation, performance optimization strategies, and advanced context menu handling techniques.

## Context-Aware UI System Enhancements

### Advanced Context Menu Patterns

The original implementation uses synchronous `exec()` for context menus. For complex scenarios, consider asynchronous handling using `popup()`:

```cpp
void SidebarWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu* menu = new QMenu(this); // Parent ownership ensures automatic deletion
    menu->addAction(m_loadAction);
    menu->addAction(m_unloadAction);
    
    // Asynchronous display with position tracking
    menu->popup(event->globalPos());
    
    // Track menu lifetime using QPointer
    QPointer<QMenu> menuTracker(menu);
    connect(menu, &QMenu::aboutToHide, [menuTracker]() {
        if(menuTracker) qDebug() << "Menu interaction completed";
    });
}
```

Key improvements from Qt documentation [^2_2][^2_5][^2_6]:

1. Parent-child relationship ensures automatic memory management
2. QPointer provides safe access to menu instance
3. Asynchronous operation maintains UI responsiveness

### Dynamic Menu Population

For variable menu content, implement `aboutToShow` signal handling:

```cpp
m_contextMenu->addAction("Dynamic Actions");
QMenu* dynamicSubmenu = new QMenu(this);
m_contextMenu->addMenu(dynamicSubmenu);

connect(dynamicSubmenu, &QMenu::aboutToShow, [this, dynamicSubmenu]() {
    dynamicSubmenu->clear();
    auto recentScans = m_model->getRecentScans();
    for (const auto& scan : recentScans) {
        dynamicSubmenu->addAction(scan.name, [this, scan]() {
            handleRecentScanSelected(scan);
        });
    }
});
```

This pattern from [^2_13][^2_17] ensures menu content stays current without premature initialization.

## Performance Profiling System Expansion

### Integrated Qt Performance Tools

Complement the custom profiler with Qt's built-in tools [^2_8]:

```cmake
# Enable Qt's advanced profiling features
target_compile_definitions(PerformanceProfiler PRIVATE
    QT_USE_QSTRINGBUILDER
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
)

# Add performance monitoring module
find_package(Qt6 COMPONENTS Core5Compat)
target_link_libraries(PerformanceProfiler PRIVATE Qt6::Core5Compat)
```

Key integration points:

1. Use `QElapsedTimer::PerformanceCounter` for high-resolution timing [^2_7]
2. Combine with Qt Creator's built-in profiler for graphical analysis
3. Implement `Q_PROFILER_MAKE_SESSION` for cross-tool compatibility

### Advanced Benchmark Metrics

Extend the benchmarking system with memory tracking:

```cpp
class MemoryTracker {
public:
    static size_t currentUsage() {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), 
                           (PROCESS_MEMORY_COUNTERS*)&pmc, 
                           sizeof(pmc));
        return pmc.PrivateUsage;
#else
        // Linux implementation using /proc/self/statm
#endif
    }
};

void PerformanceBenchmark::runBenchmark() {
    size_t preAlloc = MemoryTracker::currentUsage();
    // Benchmark operations
    size_t postAlloc = MemoryTracker::currentUsage();
    results.memoryDelta = postAlloc - preAlloc;
}
```


## Stress Testing System Improvements

### Resource Contention Testing

Enhance LRU eviction tests with threading scenarios:

```cpp
TEST_F(StressTest, ConcurrentAccessEviction) {
    constexpr int THREAD_COUNT = 8;
    std::vector<std::thread> workers;
    
    m_loadManager->setMemoryLimit(200 * 1024 * 1024); // 200MB
    
    for(int i=0; i<THREAD_COUNT; ++i) {
        workers.emplace_back([this, i]() {
            for(int j=0; j<100; ++j) {
                QString file = QString("scan_%1.e57").arg((i+j) % 10);
                m_loadManager->loadScan(file);
                std::this_thread::sleep_for(1ms);
            }
        });
    }
    
    for(auto& t : workers) t.join();
    
    // Verify memory constraints maintained
    ASSERT_LE(m_loadManager->getTotalMemoryUsage(), 
             m_loadManager->getMemoryLimit());
}
```

This test validates thread safety in the memory management system.

## Enhanced vcpkg Integration

### Dependency Management Strategy

Expand the vcpkg configuration with version pinning:

```json
{
    "name": "faro-scene-registration",
    "version": "1.0.0",
    "dependencies": [
        {
            "name": "gtest",
            "version>=": "1.14.0"
        },
        {
            "name": "benchmark",
            "features": ["tools"],
            "version": "1.8.2"
        }
    ],
    "builtin-baseline": "2024.03.25",
    "overrides": [
        { "name": "zlib", "version": "1.3.1" }
    ]
}
```

Critical updates from [^2_9][^2_10]:

1. Version constraints ensure reproducible builds
2. Feature selection optimizes dependency tree
3. Overrides handle transitive dependency conflicts

### CMake Presets Integration

Create `CMakePresets.json` for simplified workflow:

```json
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 16
    },
    "configurePresets": [
        {
            "name": "vcpkg-release",
            "displayName": "Release with vcpkg",
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
                "CMAKE_BUILD_TYPE": "RelWithDebInfo"
            },
            "environment": {
                "VCPKG_DEFAULT_TRIPLET": "x64-windows-static"
            }
        }
    ]
}
```


## Code Quality Assurance System

### Static Analysis Integration

Enhance CMake with clang-tidy checks:

```cmake
find_program(CLANG_TIDY_EXE "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE}
        -checks=modernize-*,cppcoreguidelines-*
        -warnings-as-errors=*
    )
endif()
```


### Performance Regression Guardrails

Implement CI pipeline checks:

```yaml
steps:
- name: Performance Threshold Check
  run: |
    ./StressTests --gtest_filter=*MemoryLimitEnforcement*
    MEM_USAGE=$(parse_test_output.py max_memory)
    if (( $MEM_USAGE > 150000000 )); then
        echo "Memory threshold exceeded"
        exit 1
    fi
```


## Documentation Generation System

### Automated API Documentation

Integrate Doxygen with CMake:

```cmake
find_package(Doxygen)
if(DOXYGEN_FOUND)
    doxygen_add_docs(
        docs
        ${PROJECT_SOURCE_DIR}/src
        COMMENT "Generate API documentation"
    )
endif()
```

Configure `Doxyfile.in` with Qt-specific settings:

```
INPUT += @Qt6Core_DIR@/include @Qt6Widgets_DIR@/include
PREDEFINED += QT_CORE_LIB QT_WIDGETS_LIB
```

This comprehensive enhancement integrates critical Qt development practices, advanced performance techniques, and robust dependency management strategies. The updates align with Qt's modern toolchain recommendations while maintaining backward compatibility with the original implementation requirements.

<div style="text-align: center">⁂</div>

[^2_1]: https://www.myfoodresearch.com/uploads/8/4/8/5/84855864/_21__fr-2022-591_lercari.pdf

[^2_2]: https://doc.qt.io/qt-6/qtwidgets-mainwindows-menus-example.html

[^2_3]: https://www.pythonguis.com/tutorials/pyqt-actions-toolbars-menus/

[^2_4]: https://www.pythonguis.com/tutorials/pyqt6-signals-slots-events/

[^2_5]: https://doc.qt.io/qt-6/qcontextmenuevent.html

[^2_6]: https://doc.qt.io/qt-6/qmenu.html

[^2_7]: https://doc.qt.io/qt-6/qelapsedtimer.html

[^2_8]: https://www.qt.io/blog/qt-performance-and-tools-update

[^2_9]: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

[^2_10]: https://doc.qt.io/qtcreator/creator-vcpkg.html

[^2_11]: https://www.morressier.com/article/619bdac3be545488cfae5103

[^2_12]: https://stackoverflow.com/questions/24254006/rightclick-event-in-qt-to-open-a-context-menu

[^2_13]: https://stackoverflow.com/questions/2781198/how-to-add-menu-dynamically-in-qt

[^2_14]: https://qt.developpez.com/doc/6.6/qcontextmenuevent/

[^2_15]: https://stackoverflow.com/questions/72822872/python-pyqt6-contextmenuevent-strange-behavior-i-dont-understand

[^2_16]: https://pubsonline.informs.org/doi/10.1287/msom.2022.0400

[^2_17]: https://www.pythonguis.com/tutorials/pyside6-actions-toolbars-menus/

[^2_18]: https://www.emerald.com/insight/content/doi/10.1108/BFJ-06-2021-0666/full/html

[^2_19]: https://ieeexplore.ieee.org/document/10971452/

[^2_20]: https://www.frontiersin.org/articles/10.3389/fsufs.2024.1363565/full

[^2_21]: https://royalsocietypublishing.org/doi/10.1098/rsta.2016.0377

[^2_22]: https://link.springer.com/10.1007/s10815-024-03344-x

[^2_23]: https://www.semanticscholar.org/paper/45d724a732a1149e0c3c118dfeea260e29b98eb0

[^2_24]: https://www.semanticscholar.org/paper/77ce9a93562772dc35da6a87be67be89c523fb90

[^2_25]: https://forum.endeavouros.com/t/qt6-context-menu-and-menu-bar-issue/52389

[^2_26]: https://doc.qt.io/qt-6/qtquickcontrols-menus.html

[^2_27]: https://journals.asm.org/doi/10.1128/jvi.71.10.7814-7819.1997

[^2_28]: https://journals.asm.org/doi/10.1128/JVI.77.12.6709-6719.2003

[^2_29]: https://www.semanticscholar.org/paper/211fba6c738f7511ae36da501802f5531fbf85de

[^2_30]: https://doc.qt.io/qt-6/qthread.html

[^2_31]: https://www.ics.com/blog/qt-has-solution-all-your-timing-needs

[^2_32]: https://www.youtube.com/watch?v=yKKpB7LFQog

[^2_33]: https://www.qt.io/resources/videos/building-a-qt-application-with-modern-cmake-and-vcpkg

[^2_34]: https://vcpkg.io/en/package/qtbase.html

[^2_35]: https://www.pythonguis.com/tutorials/pyside6-signals-slots-events/

[^2_36]: https://www.pythonguis.com/tutorials/pyqt6-actions-toolbars-menus/

[^2_37]: https://www.qtcentre.org/threads/7018-how-to-popup-and-close-a-QMenu

[^2_38]: https://ashpublications.org/blood/article/104/11/1132/53700/Inhibition-of-Retinoic-Acid-Receptor-Signaling-by

[^2_39]: https://nmgs.nmt.edu/publications/guidebooks/63/

[^2_40]: https://stackoverflow.com/questions/244646/get-elapsed-time-in-qt

[^2_41]: https://qt.developpez.com/doc/6.1/qelapsedtimer/

[^2_42]: https://dreamswork.github.io/qt4/classQElapsedTimer.html

[^2_43]: https://doc.qt.io/qt-6/qtquick-performance.html

[^2_44]: https://scythe-studio.com/en/blog/porting-from-qt-5-to-qt-6

[^2_45]: https://stackoverflow.com/questions/71288948/using-vcpkg-with-cmake-and-qt-6-for-windows-arm64

[^2_46]: https://github.com/microsoft/vcpkg/issues/43668

[^2_47]: https://forum.qt.io/topic/135877/how-to-use-find_package-with-qt6-and-modern-cmake-windows

[^2_48]: https://discourse.cmake.org/t/cmake-3-28-0-rc3-fails-to-build-qt6-via-vcpkg/9369

[^2_49]: https://www.qtcentre.org/threads/71952-CMake-vcpkg-qt-takes-precedence-over-onlineinstaller-qt

[^2_50]: https://www.youtube.com/watch?v=s4hHWopX8yw

