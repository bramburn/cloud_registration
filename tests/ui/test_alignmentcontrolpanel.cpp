#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QtTest>

#include "ui/AlignmentControlPanel.h"
#include "registration/AlignmentEngine.h"

/**
 * @brief Test class for AlignmentControlPanel finalization functionality
 * 
 * Tests the Accept and Cancel button functionality as specified in MVP3 S3.1
 */
class TestAlignmentControlPanel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Test cases for MVP3 S3.1 requirements
    void testAcceptButtonEnabledOnlyWhenValid();
    void testCancelButtonEnabledWhenActive();
    void testAcceptButtonSignalEmission();
    void testCancelButtonSignalEmission();
    void testButtonStatesWithDifferentAlignmentStates();

private:
    QApplication* m_app;
    AlignmentControlPanel* m_panel;
    AlignmentEngine* m_engine;
};

void TestAlignmentControlPanel::initTestCase()
{
    // Create QApplication if it doesn't exist
    if (!QApplication::instance())
    {
        int argc = 0;
        char** argv = nullptr;
        m_app = new QApplication(argc, argv);
    }
    else
    {
        m_app = nullptr;
    }
}

void TestAlignmentControlPanel::cleanupTestCase()
{
    if (m_app)
    {
        delete m_app;
        m_app = nullptr;
    }
}

void TestAlignmentControlPanel::init()
{
    m_panel = new AlignmentControlPanel();
    m_engine = new AlignmentEngine();
    m_panel->setAlignmentEngine(m_engine);
}

void TestAlignmentControlPanel::cleanup()
{
    delete m_panel;
    delete m_engine;
    m_panel = nullptr;
    m_engine = nullptr;
}

void TestAlignmentControlPanel::testAcceptButtonEnabledOnlyWhenValid()
{
    // Test Case 1: Accept button enablement logic
    // Expected: Accept button enabled only for Valid state
    
    // Initially should be disabled (Idle state)
    QVERIFY(!m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
    
    // Simulate different alignment states
    AlignmentEngine::AlignmentResult result;
    
    // Test Idle state
    result.state = AlignmentEngine::AlignmentState::Idle;
    m_panel->updateAlignmentResult(result);
    QVERIFY(!m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
    
    // Test Insufficient state
    result.state = AlignmentEngine::AlignmentState::Insufficient;
    m_panel->updateAlignmentResult(result);
    QVERIFY(!m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
    
    // Test Computing state
    result.state = AlignmentEngine::AlignmentState::Computing;
    m_panel->updateAlignmentResult(result);
    QVERIFY(!m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
    
    // Test Valid state - should be enabled
    result.state = AlignmentEngine::AlignmentState::Valid;
    m_panel->updateAlignmentResult(result);
    QVERIFY(m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
    
    // Test Error state
    result.state = AlignmentEngine::AlignmentState::Error;
    m_panel->updateAlignmentResult(result);
    QVERIFY(!m_panel->findChild<QPushButton*>("Accept Alignment")->isEnabled());
}

void TestAlignmentControlPanel::testCancelButtonEnabledWhenActive()
{
    // Test Case 2: Cancel button enablement logic
    // Expected: Cancel button enabled when alignment mode is active (not Idle)
    
    AlignmentEngine::AlignmentResult result;
    
    // Test Idle state - should be disabled
    result.state = AlignmentEngine::AlignmentState::Idle;
    m_panel->updateAlignmentResult(result);
    QVERIFY(!m_panel->findChild<QPushButton*>("Cancel")->isEnabled());
    
    // Test active states - should be enabled
    result.state = AlignmentEngine::AlignmentState::Insufficient;
    m_panel->updateAlignmentResult(result);
    QVERIFY(m_panel->findChild<QPushButton*>("Cancel")->isEnabled());
    
    result.state = AlignmentEngine::AlignmentState::Computing;
    m_panel->updateAlignmentResult(result);
    QVERIFY(m_panel->findChild<QPushButton*>("Cancel")->isEnabled());
    
    result.state = AlignmentEngine::AlignmentState::Valid;
    m_panel->updateAlignmentResult(result);
    QVERIFY(m_panel->findChild<QPushButton*>("Cancel")->isEnabled());
    
    result.state = AlignmentEngine::AlignmentState::Error;
    m_panel->updateAlignmentResult(result);
    QVERIFY(m_panel->findChild<QPushButton*>("Cancel")->isEnabled());
}

void TestAlignmentControlPanel::testAcceptButtonSignalEmission()
{
    // Test that clicking Accept button emits the correct signal
    QSignalSpy spy(m_panel, &AlignmentControlPanel::acceptAlignmentRequested);
    
    // Enable the button by setting Valid state
    AlignmentEngine::AlignmentResult result;
    result.state = AlignmentEngine::AlignmentState::Valid;
    m_panel->updateAlignmentResult(result);
    
    // Find and click the Accept button
    QPushButton* acceptButton = m_panel->findChild<QPushButton*>("Accept Alignment");
    QVERIFY(acceptButton != nullptr);
    QVERIFY(acceptButton->isEnabled());
    
    acceptButton->click();
    
    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
}

void TestAlignmentControlPanel::testCancelButtonSignalEmission()
{
    // Test that clicking Cancel button emits the correct signal
    QSignalSpy spy(m_panel, &AlignmentControlPanel::cancelAlignmentRequested);
    
    // Enable the button by setting an active state
    AlignmentEngine::AlignmentResult result;
    result.state = AlignmentEngine::AlignmentState::Valid;
    m_panel->updateAlignmentResult(result);
    
    // Find and click the Cancel button
    QPushButton* cancelButton = m_panel->findChild<QPushButton*>("Cancel");
    QVERIFY(cancelButton != nullptr);
    QVERIFY(cancelButton->isEnabled());
    
    cancelButton->click();
    
    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
}

void TestAlignmentControlPanel::testButtonStatesWithDifferentAlignmentStates()
{
    // Comprehensive test of button states across all alignment states
    struct TestCase {
        AlignmentEngine::AlignmentState state;
        bool acceptEnabled;
        bool cancelEnabled;
        QString description;
    };
    
    QList<TestCase> testCases = {
        {AlignmentEngine::AlignmentState::Idle, false, false, "Idle state"},
        {AlignmentEngine::AlignmentState::Insufficient, false, true, "Insufficient state"},
        {AlignmentEngine::AlignmentState::Computing, false, true, "Computing state"},
        {AlignmentEngine::AlignmentState::Valid, true, true, "Valid state"},
        {AlignmentEngine::AlignmentState::Error, false, true, "Error state"}
    };
    
    for (const auto& testCase : testCases)
    {
        AlignmentEngine::AlignmentResult result;
        result.state = testCase.state;
        m_panel->updateAlignmentResult(result);
        
        QPushButton* acceptButton = m_panel->findChild<QPushButton*>("Accept Alignment");
        QPushButton* cancelButton = m_panel->findChild<QPushButton*>("Cancel");
        
        QVERIFY2(acceptButton != nullptr, "Accept button not found");
        QVERIFY2(cancelButton != nullptr, "Cancel button not found");
        
        QVERIFY2(acceptButton->isEnabled() == testCase.acceptEnabled, 
                 qPrintable(QString("Accept button state incorrect for %1").arg(testCase.description)));
        QVERIFY2(cancelButton->isEnabled() == testCase.cancelEnabled,
                 qPrintable(QString("Cancel button state incorrect for %1").arg(testCase.description)));
    }
}

QTEST_MAIN(TestAlignmentControlPanel)
#include "test_alignmentcontrolpanel.moc"
