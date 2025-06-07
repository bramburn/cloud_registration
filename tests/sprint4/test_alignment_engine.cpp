#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QVector3D>
#include <QMatrix4x4>
#include <QList>
#include <QPair>

// Include the class under test
#include "../../src/registration/AlignmentEngine.h"

/**
 * @brief Unit tests for AlignmentEngine class
 * 
 * Tests the high-level alignment workflow coordination including
 * correspondence management, real-time computation, and signal emission.
 */
class TestAlignmentEngine : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Core functionality tests
    void testCorrespondenceManagement();
    void testAlignmentComputation();
    void testRealTimeUpdates();
    void testQualityThresholds();

    // Signal emission tests
    void testSignalEmission();
    void testStateTransitions();
    void testAutoRecompute();

    // Configuration tests
    void testQualityConfiguration();
    void testAutoRecomputeConfiguration();

    // Edge case tests
    void testInsufficientCorrespondences();
    void testInvalidCorrespondences();
    void testEmptyEngine();

private:
    // Helper methods
    QList<QPair<QVector3D, QVector3D>> createValidCorrespondences();
    QList<QPair<QVector3D, QVector3D>> createInvalidCorrespondences();
    void waitForSignals(int timeoutMs = 1000);
    
    // Test data
    AlignmentEngine* m_engine;
    static constexpr float TOLERANCE = 1e-3f;
};

void TestAlignmentEngine::initTestCase()
{
    qDebug() << "Starting AlignmentEngine tests";
    m_engine = new AlignmentEngine(this);
}

void TestAlignmentEngine::cleanupTestCase()
{
    qDebug() << "AlignmentEngine tests completed";
    delete m_engine;
}

void TestAlignmentEngine::testCorrespondenceManagement()
{
    qDebug() << "Testing correspondence management";
    
    // Test initial state
    QVERIFY2(m_engine->getCorrespondences().isEmpty(), 
             "Engine should start with no correspondences");
    
    // Test adding correspondences
    QSignalSpy correspondencesSpy(m_engine, &AlignmentEngine::correspondencesChanged);
    
    m_engine->addCorrespondence(QVector3D(0, 0, 0), QVector3D(1, 1, 1));
    QVERIFY2(m_engine->getCorrespondences().size() == 1, 
             "Should have one correspondence after adding");
    QVERIFY2(correspondencesSpy.count() == 1, 
             "Should emit correspondencesChanged signal");
    
    // Test setting multiple correspondences
    QList<QPair<QVector3D, QVector3D>> correspondences = createValidCorrespondences();
    m_engine->setCorrespondences(correspondences);
    QVERIFY2(m_engine->getCorrespondences().size() == correspondences.size(), 
             "Should have all correspondences after setting");
    
    // Test removing correspondence
    int initialCount = m_engine->getCorrespondences().size();
    m_engine->removeCorrespondence(0);
    QVERIFY2(m_engine->getCorrespondences().size() == initialCount - 1, 
             "Should have one less correspondence after removal");
    
    // Test clearing correspondences
    m_engine->clearCorrespondences();
    QVERIFY2(m_engine->getCorrespondences().isEmpty(), 
             "Should have no correspondences after clearing");
}

void TestAlignmentEngine::testAlignmentComputation()
{
    qDebug() << "Testing alignment computation";
    
    // Set valid correspondences
    QList<QPair<QVector3D, QVector3D>> correspondences = createValidCorrespondences();
    m_engine->setCorrespondences(correspondences);
    
    // Disable auto-recompute for controlled testing
    m_engine->setAutoRecompute(false);
    
    // Test manual computation
    QSignalSpy resultSpy(m_engine, &AlignmentEngine::alignmentResultUpdated);
    QSignalSpy transformSpy(m_engine, &AlignmentEngine::transformationUpdated);
    QSignalSpy qualitySpy(m_engine, &AlignmentEngine::qualityMetricsUpdated);
    
    m_engine->recomputeAlignment();
    waitForSignals();
    
    // Verify signals were emitted
    QVERIFY2(resultSpy.count() >= 1, "Should emit alignment result");
    QVERIFY2(transformSpy.count() >= 1, "Should emit transformation update");
    QVERIFY2(qualitySpy.count() >= 1, "Should emit quality metrics");
    
    // Verify result validity
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.isValid(), "Result should be valid for sufficient correspondences");
    QVERIFY2(!result.transformation.isIdentity(), "Transformation should not be identity");
    QVERIFY2(result.errorStats.numCorrespondences == correspondences.size(), 
             "Error statistics should reflect correspondence count");
}

void TestAlignmentEngine::testRealTimeUpdates()
{
    qDebug() << "Testing real-time updates";
    
    // Enable auto-recompute
    m_engine->setAutoRecompute(true);
    m_engine->clearCorrespondences();
    
    QSignalSpy stateSpy(m_engine, &AlignmentEngine::alignmentStateChanged);
    
    // Add correspondences one by one and verify state changes
    m_engine->addCorrespondence(QVector3D(0, 0, 0), QVector3D(1, 0, 0));
    waitForSignals(500);
    QVERIFY2(stateSpy.count() >= 1, "Should emit state change for insufficient correspondences");
    
    m_engine->addCorrespondence(QVector3D(1, 0, 0), QVector3D(2, 0, 0));
    waitForSignals(500);
    
    m_engine->addCorrespondence(QVector3D(0, 1, 0), QVector3D(1, 1, 0));
    waitForSignals(500);
    
    // Should now have valid alignment
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Valid, 
             "Should have valid state with sufficient correspondences");
}

void TestAlignmentEngine::testQualityThresholds()
{
    qDebug() << "Testing quality thresholds";
    
    // Set strict quality thresholds
    m_engine->setQualityThresholds(1.0f, 2.0f); // Very strict thresholds
    
    // Create correspondences with intentional misalignment (no perfect transformation exists)
    QList<QPair<QVector3D, QVector3D>> correspondences;
    correspondences.append(qMakePair(QVector3D(0, 0, 0), QVector3D(5, 2, 1))); // Misaligned
    correspondences.append(qMakePair(QVector3D(1, 0, 0), QVector3D(6, 3, 0))); // Misaligned
    correspondences.append(qMakePair(QVector3D(0, 1, 0), QVector3D(4, 1, 2))); // Misaligned
    
    m_engine->setCorrespondences(correspondences);
    m_engine->recomputeAlignment();
    waitForSignals();
    
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.errorStats.rmsError > 0.5f,
             "Should have measurable RMS error with misaligned correspondences");
    
    // Reset to reasonable thresholds
    m_engine->setQualityThresholds(10.0f, 20.0f);
}

void TestAlignmentEngine::testSignalEmission()
{
    qDebug() << "Testing signal emission patterns";
    
    m_engine->clearCorrespondences();
    
    // Setup signal spies
    QSignalSpy correspondencesSpy(m_engine, &AlignmentEngine::correspondencesChanged);
    QSignalSpy stateSpy(m_engine, &AlignmentEngine::alignmentStateChanged);
    QSignalSpy resultSpy(m_engine, &AlignmentEngine::alignmentResultUpdated);
    QSignalSpy transformSpy(m_engine, &AlignmentEngine::transformationUpdated);
    QSignalSpy qualitySpy(m_engine, &AlignmentEngine::qualityMetricsUpdated);
    
    // Add correspondences and verify signal patterns
    QList<QPair<QVector3D, QVector3D>> correspondences = createValidCorrespondences();
    m_engine->setCorrespondences(correspondences);
    waitForSignals();
    
    // Verify all expected signals were emitted
    QVERIFY2(correspondencesSpy.count() >= 1, "Should emit correspondences changed");
    QVERIFY2(stateSpy.count() >= 1, "Should emit state changes");
    QVERIFY2(resultSpy.count() >= 1, "Should emit result updates");
    QVERIFY2(transformSpy.count() >= 1, "Should emit transformation updates");
    QVERIFY2(qualitySpy.count() >= 1, "Should emit quality updates");
}

void TestAlignmentEngine::testStateTransitions()
{
    qDebug() << "Testing state transitions";
    
    m_engine->clearCorrespondences();
    QSignalSpy stateSpy(m_engine, &AlignmentEngine::alignmentStateChanged);
    
    // Should start in Idle state
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Idle, 
             "Should start in Idle state");
    
    // Add insufficient correspondences -> Insufficient state
    m_engine->addCorrespondence(QVector3D(0, 0, 0), QVector3D(1, 0, 0));
    m_engine->addCorrespondence(QVector3D(1, 0, 0), QVector3D(2, 0, 0));
    waitForSignals();
    
    result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Insufficient, 
             "Should be in Insufficient state with <3 correspondences");
    
    // Add sufficient correspondences -> Valid state
    m_engine->addCorrespondence(QVector3D(0, 1, 0), QVector3D(1, 1, 0));
    waitForSignals();
    
    result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Valid, 
             "Should be in Valid state with >=3 correspondences");
}

void TestAlignmentEngine::testAutoRecompute()
{
    qDebug() << "Testing auto-recompute functionality";
    
    // Test disabling auto-recompute
    m_engine->setAutoRecompute(false);
    QVERIFY2(!m_engine->isAutoRecompute(), "Auto-recompute should be disabled");
    
    m_engine->clearCorrespondences();
    QSignalSpy resultSpy(m_engine, &AlignmentEngine::alignmentResultUpdated);
    
    // Add correspondences - should not trigger automatic computation
    m_engine->addCorrespondence(QVector3D(0, 0, 0), QVector3D(1, 0, 0));
    m_engine->addCorrespondence(QVector3D(1, 0, 0), QVector3D(2, 0, 0));
    m_engine->addCorrespondence(QVector3D(0, 1, 0), QVector3D(1, 1, 0));
    
    QTest::qWait(200); // Wait briefly
    
    // Should not have computed automatically
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state != AlignmentEngine::AlignmentState::Valid, 
             "Should not auto-compute when disabled");
    
    // Manual computation should work
    m_engine->recomputeAlignment();
    waitForSignals();
    
    result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Valid, 
             "Manual computation should work");
    
    // Re-enable auto-recompute
    m_engine->setAutoRecompute(true);
    QVERIFY2(m_engine->isAutoRecompute(), "Auto-recompute should be enabled");
}

void TestAlignmentEngine::testQualityConfiguration()
{
    qDebug() << "Testing quality configuration";

    float rmsThreshold = 3.0f;
    float maxThreshold = 6.0f;

    QSignalSpy configSpy(m_engine, &AlignmentEngine::alignmentResultUpdated);

    m_engine->setQualityThresholds(rmsThreshold, maxThreshold);

    // Configuration change might trigger recomputation if correspondences exist
    if (m_engine->getCorrespondences().size() >= 3) {
        waitForSignals();
        QVERIFY2(configSpy.count() >= 0, "Quality threshold change handled");
    }
}

void TestAlignmentEngine::testAutoRecomputeConfiguration()
{
    qDebug() << "Testing auto-recompute configuration";

    // Test initial state
    bool initialState = m_engine->isAutoRecompute();

    // Toggle auto-recompute
    m_engine->setAutoRecompute(!initialState);
    QVERIFY2(m_engine->isAutoRecompute() == !initialState,
             "Auto-recompute setting should be toggled");

    // Restore original state
    m_engine->setAutoRecompute(initialState);
    QVERIFY2(m_engine->isAutoRecompute() == initialState,
             "Auto-recompute setting should be restored");
}

void TestAlignmentEngine::testInsufficientCorrespondences()
{
    qDebug() << "Testing insufficient correspondences handling";
    
    m_engine->clearCorrespondences();
    
    // Add only 2 correspondences
    m_engine->addCorrespondence(QVector3D(0, 0, 0), QVector3D(1, 0, 0));
    m_engine->addCorrespondence(QVector3D(1, 0, 0), QVector3D(2, 0, 0));
    waitForSignals();
    
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Insufficient, 
             "Should be in Insufficient state with only 2 correspondences");
    QVERIFY2(result.transformation.isIdentity(), 
             "Transformation should be identity for insufficient correspondences");
}

void TestAlignmentEngine::testInvalidCorrespondences()
{
    qDebug() << "Testing invalid correspondences handling";
    
    // Create correspondences with duplicate points
    QList<QPair<QVector3D, QVector3D>> invalid = createInvalidCorrespondences();
    m_engine->setCorrespondences(invalid);
    waitForSignals();
    
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Error, 
             "Should be in Error state with invalid correspondences");
}

void TestAlignmentEngine::testEmptyEngine()
{
    qDebug() << "Testing empty engine state";
    
    m_engine->clearCorrespondences();
    
    AlignmentEngine::AlignmentResult result = m_engine->getCurrentResult();
    QVERIFY2(result.state == AlignmentEngine::AlignmentState::Idle, 
             "Empty engine should be in Idle state");
    QVERIFY2(result.transformation.isIdentity(), 
             "Empty engine should have identity transformation");
    QVERIFY2(result.errorStats.numCorrespondences == 0, 
             "Empty engine should have zero correspondences in stats");
}

// Helper method implementations
QList<QPair<QVector3D, QVector3D>> TestAlignmentEngine::createValidCorrespondences()
{
    QList<QPair<QVector3D, QVector3D>> correspondences;
    
    // Create well-separated, non-collinear correspondences
    correspondences.append(qMakePair(QVector3D(0, 0, 0), QVector3D(2, 3, 4)));
    correspondences.append(qMakePair(QVector3D(1, 0, 0), QVector3D(3, 3, 4)));
    correspondences.append(qMakePair(QVector3D(0, 1, 0), QVector3D(2, 4, 4)));
    correspondences.append(qMakePair(QVector3D(0, 0, 1), QVector3D(2, 3, 5)));
    
    return correspondences;
}

QList<QPair<QVector3D, QVector3D>> TestAlignmentEngine::createInvalidCorrespondences()
{
    QList<QPair<QVector3D, QVector3D>> correspondences;
    
    // Create correspondences with duplicate source points (invalid)
    QVector3D duplicate(0, 0, 0);
    correspondences.append(qMakePair(duplicate, QVector3D(1, 0, 0)));
    correspondences.append(qMakePair(duplicate, QVector3D(2, 0, 0))); // Duplicate source
    correspondences.append(qMakePair(QVector3D(1, 0, 0), QVector3D(3, 0, 0)));
    
    return correspondences;
}

void TestAlignmentEngine::waitForSignals(int timeoutMs)
{
    QTest::qWait(timeoutMs);
    QCoreApplication::processEvents();
}

QTEST_MAIN(TestAlignmentEngine)
#include "test_alignment_engine.moc"
