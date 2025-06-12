#include <gtest/gtest.h>
#include "core/performance_profiler.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QThread>
#include <QElapsedTimer>

class PerformanceProfilerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application if not already done
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = std::make_unique<QCoreApplication>(argc, argv);
        }

        profiler = &PerformanceProfiler::instance();
        profiler->reset(); // Start with clean state
    }

    void TearDown() override {
        profiler->reset();
    }

    std::unique_ptr<QCoreApplication> app;
    PerformanceProfiler* profiler;
};

// Test basic profiler functionality
TEST_F(PerformanceProfilerTest, BasicSectionTiming) {
    EXPECT_TRUE(profiler->isEnabled());

    profiler->startSection("TestSection");
    QThread::msleep(10); // Small delay
    profiler->endSection("TestSection");

    ProfileSection section = profiler->getSection("TestSection");
    EXPECT_EQ(section.name, "TestSection");
    EXPECT_EQ(section.callCount, 1);
    EXPECT_GT(section.elapsedMs, 0);
    EXPECT_GE(section.elapsedMs, 10); // Should be at least 10ms
}

TEST_F(PerformanceProfilerTest, MultipleSectionCalls) {
    const QString sectionName = "MultipleCallsSection";
    const int numCalls = 5;

    for (int i = 0; i < numCalls; ++i) {
        profiler->startSection(sectionName);
        QThread::msleep(5);
        profiler->endSection(sectionName);
    }

    ProfileSection section = profiler->getSection(sectionName);
    EXPECT_EQ(section.callCount, numCalls);
    EXPECT_GT(section.totalTime, 0);
    EXPECT_GT(section.averageTime(), 0);
    EXPECT_GT(section.maxTime, 0);
    EXPECT_LT(section.minTime, LLONG_MAX);
}

TEST_F(PerformanceProfilerTest, EnableDisableProfiling) {
    profiler->setEnabled(false);
    EXPECT_FALSE(profiler->isEnabled());

    profiler->startSection("DisabledSection");
    QThread::msleep(10);
    profiler->endSection("DisabledSection");

    ProfileSection section = profiler->getSection("DisabledSection");
    EXPECT_EQ(section.callCount, 0); // Should not have recorded anything

    profiler->setEnabled(true);
    EXPECT_TRUE(profiler->isEnabled());
}

TEST_F(PerformanceProfilerTest, ReportGeneration) {
    // Create some profiling data
    profiler->startSection("Section1");
    QThread::msleep(20);
    profiler->endSection("Section1");

    profiler->startSection("Section2");
    QThread::msleep(10);
    profiler->endSection("Section2");

    // Test report generation (to console)
    EXPECT_NO_THROW(profiler->generateReport());
}

TEST_F(PerformanceProfilerTest, Reset) {
    profiler->startSection("TestSection");
    QThread::msleep(10);
    profiler->endSection("TestSection");

    ProfileSection section = profiler->getSection("TestSection");
    EXPECT_EQ(section.callCount, 1);

    profiler->reset();

    ProfileSection resetSection = profiler->getSection("TestSection");
    EXPECT_EQ(resetSection.callCount, 0);
}

TEST_F(PerformanceProfilerTest, InvalidSectionHandling) {
    // Test ending a section that was never started
    EXPECT_NO_THROW(profiler->endSection("NonExistentSection"));

    ProfileSection section = profiler->getSection("NonExistentSection");
    EXPECT_EQ(section.callCount, 0);
}

TEST_F(PerformanceProfilerTest, NestedSections) {
    profiler->startSection("OuterSection");
    QThread::msleep(5);

    profiler->startSection("InnerSection");
    QThread::msleep(10);
    profiler->endSection("InnerSection");

    QThread::msleep(5);
    profiler->endSection("OuterSection");

    ProfileSection outer = profiler->getSection("OuterSection");
    ProfileSection inner = profiler->getSection("InnerSection");

    EXPECT_EQ(outer.callCount, 1);
    EXPECT_EQ(inner.callCount, 1);
    EXPECT_GT(outer.elapsedMs, inner.elapsedMs); // Outer should take longer
}
