#ifndef MOCKMAINVIEW_H
#define MOCKMAINVIEW_H

#include <memory>

#include "../../src/IMainView.h"
#include "MockPointCloudViewer.h"

#include <gmock/gmock.h>

/**
 * @brief MockMainView - Mock implementation of IMainView for testing
 *
 * This mock class implements the IMainView interface using Google Mock's
 * MOCK_METHOD macro. It allows tests to verify that the presenter correctly
 * updates the UI and interacts with the main view without requiring actual
 * Qt widgets or UI components.
 *
 * Sprint 5 Testing Requirements:
 * - Enables unit testing of MainPresenter without UI dependencies
 * - Allows verification of correct UI update method calls
 * - Provides controllable user input simulation for testing
 * - Verifies correct parameter passing for UI operations
 */
class MockMainView : public IMainView
{
    Q_OBJECT

public:
    explicit MockMainView(QObject* parent = nullptr)
        : IMainView(parent), m_mockViewer(std::make_unique<MockPointCloudViewer>())
    {
    }

    virtual ~MockMainView() = default;

    // Message display methods
    MOCK_METHOD(void, displayErrorMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayInfoMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayWarningMessage, (const QString& title, const QString& message), (override));

    // Status and title updates
    MOCK_METHOD(void, updateStatusBar, (const QString& text), (override));
    MOCK_METHOD(void, setWindowTitle, (const QString& title), (override));

    // Progress dialog
    MOCK_METHOD(void, showProgressDialog, (bool show, const QString& title, const QString& message), (override));
    MOCK_METHOD(void, updateProgress, (int percentage, const QString& message), (override));

    // UI state management
    MOCK_METHOD(void, setActionsEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setProjectTitle, (const QString& projectName), (override));

    // Scan management
    MOCK_METHOD(void, updateScanList, (const QStringList& scanNames), (override));
    MOCK_METHOD(void, highlightScan, (const QString& scanName), (override));

    // View switching
    MOCK_METHOD(void, showProjectHub, (), (override));
    MOCK_METHOD(void, showProjectView, (), (override));

    // Statistics display
    MOCK_METHOD(void, updateMemoryUsage, (size_t totalBytes), (override));
    MOCK_METHOD(void, updateRenderingStats, (float fps, int visiblePoints), (override));

    // File dialogs
    MOCK_METHOD(QString, askForOpenFilePath, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(QString,
                askForSaveFilePath,
                (const QString& title, const QString& filter, const QString& defaultName),
                (override));
    MOCK_METHOD(bool, askForConfirmation, (const QString& title, const QString& message), (override));

    // Viewer access - return our mock viewer
    IPointCloudViewer* getViewer() override
    {
        return m_mockViewer.get();
    }

    // Access to the mock viewer for test setup
    MockPointCloudViewer* getMockViewer()
    {
        return m_mockViewer.get();
    }

    // Helper methods for tests to emit signals
    void emitNewProjectRequested()
    {
        emit newProjectRequested();
    }

    void emitOpenProjectRequested()
    {
        emit openProjectRequested();
    }

    void emitCloseProjectRequested()
    {
        emit closeProjectRequested();
    }

    void emitImportScansRequested()
    {
        emit importScansRequested();
    }

    void emitOpenFileRequested(const QString& filePath)
    {
        emit openFileRequested(filePath);
    }

    void emitSaveFileRequested(const QString& filePath)
    {
        emit saveFileRequested(filePath);
    }

    void emitScanActivated(const QString& scanId)
    {
        emit scanActivated(scanId);
    }

    void emitViewerSettingsChanged()
    {
        emit viewerSettingsChanged();
    }

    void emitExitRequested()
    {
        emit exitRequested();
    }

    // Test helper methods to set up common scenarios
    void setupSuccessfulFileDialog(const QString& filePath)
    {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, askForOpenFilePath(_, _)).WillByDefault(Return(filePath));

        ON_CALL(*this, askForSaveFilePath(_, _, _)).WillByDefault(Return(filePath));
    }

    void setupCancelledFileDialog()
    {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, askForOpenFilePath(_, _)).WillByDefault(Return(QString()));

        ON_CALL(*this, askForSaveFilePath(_, _, _)).WillByDefault(Return(QString()));
    }

    void setupConfirmationDialog(bool userConfirms)
    {
        using ::testing::_;
        using ::testing::Return;

        ON_CALL(*this, askForConfirmation(_, _)).WillByDefault(Return(userConfirms));
    }

    void setupEmptyViewer()
    {
        m_mockViewer->setupEmptyViewer();
    }

    void setupLoadedViewer(size_t numPoints = 300)
    {
        m_mockViewer->setupLoadedViewer(numPoints);
    }

    // Verification helpers for common UI interactions
    void verifyErrorDisplayed(const QString& expectedTitle = QString(), const QString& expectedMessage = QString())
    {
        using ::testing::_;

        if (expectedTitle.isEmpty() && expectedMessage.isEmpty())
        {
            EXPECT_CALL(*this, displayErrorMessage(_, _)).Times(::testing::AtLeast(1));
        }
        else if (expectedTitle.isEmpty())
        {
            EXPECT_CALL(*this, displayErrorMessage(_, expectedMessage)).Times(1);
        }
        else if (expectedMessage.isEmpty())
        {
            EXPECT_CALL(*this, displayErrorMessage(expectedTitle, _)).Times(1);
        }
        else
        {
            EXPECT_CALL(*this, displayErrorMessage(expectedTitle, expectedMessage)).Times(1);
        }
    }

    void verifyInfoDisplayed(const QString& expectedTitle = QString(), const QString& expectedMessage = QString())
    {
        using ::testing::_;

        if (expectedTitle.isEmpty() && expectedMessage.isEmpty())
        {
            EXPECT_CALL(*this, displayInfoMessage(_, _)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, displayInfoMessage(expectedTitle, expectedMessage)).Times(1);
        }
    }

    void verifyStatusUpdated(const QString& expectedText = QString())
    {
        using ::testing::_;

        if (expectedText.isEmpty())
        {
            EXPECT_CALL(*this, updateStatusBar(_)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, updateStatusBar(expectedText)).Times(1);
        }
    }

    void verifyWindowTitleSet(const QString& expectedTitle = QString())
    {
        using ::testing::_;

        if (expectedTitle.isEmpty())
        {
            EXPECT_CALL(*this, setWindowTitle(_)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, setWindowTitle(expectedTitle)).Times(1);
        }
    }

    void verifyProgressDialogShown(bool shouldShow = true)
    {
        using ::testing::_;

        EXPECT_CALL(*this, showProgressDialog(shouldShow, _, _)).Times(::testing::AtLeast(1));
    }

    void verifyProgressUpdated()
    {
        using ::testing::_;

        EXPECT_CALL(*this, updateProgress(_, _)).Times(::testing::AtLeast(1));
    }

    void verifyActionsEnabled(bool enabled)
    {
        EXPECT_CALL(*this, setActionsEnabled(enabled)).Times(::testing::AtLeast(1));
    }

    void verifyScanListUpdated(const QStringList& expectedScans = QStringList())
    {
        using ::testing::_;

        if (expectedScans.isEmpty())
        {
            EXPECT_CALL(*this, updateScanList(_)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, updateScanList(expectedScans)).Times(1);
        }
    }

    void verifyProjectViewShown()
    {
        EXPECT_CALL(*this, showProjectView()).Times(1);
    }

    void verifyProjectHubShown()
    {
        EXPECT_CALL(*this, showProjectHub()).Times(1);
    }

    void verifyFileDialogCalled(const QString& expectedTitle = QString(), const QString& expectedFilter = QString())
    {
        using ::testing::_;

        if (expectedTitle.isEmpty() && expectedFilter.isEmpty())
        {
            EXPECT_CALL(*this, askForOpenFilePath(_, _)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, askForOpenFilePath(expectedTitle, expectedFilter)).Times(1);
        }
    }

    void verifyConfirmationAsked(const QString& expectedTitle = QString(), const QString& expectedMessage = QString())
    {
        using ::testing::_;

        if (expectedTitle.isEmpty() && expectedMessage.isEmpty())
        {
            EXPECT_CALL(*this, askForConfirmation(_, _)).Times(::testing::AtLeast(1));
        }
        else
        {
            EXPECT_CALL(*this, askForConfirmation(expectedTitle, expectedMessage)).Times(1);
        }
    }

private:
    std::unique_ptr<MockPointCloudViewer> m_mockViewer;
};

#endif  // MOCKMAINVIEW_H
