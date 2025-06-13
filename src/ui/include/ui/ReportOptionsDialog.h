#ifndef REPORTOPTIONSDIALOG_H
#define REPORTOPTIONSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// Forward declarations
class IMainView;
class PDFReportGenerator;

/**
 * @brief ReportOptionsDialog - Modal dialog for configuring PDF report generation
 * 
 * Sprint 6.3: Enhanced Report Options & Status
 * Provides users with granular control over report content and real-time feedback
 * during PDF generation process.
 */
class ReportOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportOptionsDialog(IMainView* mainView, QWidget* parent = nullptr);
    ~ReportOptionsDialog() override = default;

    /**
     * @brief Get the current report options from the dialog
     * @return ReportOptions struct with user selections
     */
    PDFReportGenerator::ReportOptions getReportOptions() const;

    /**
     * @brief Set the report options in the dialog
     * @param options ReportOptions to populate the dialog with
     */
    void setReportOptions(const PDFReportGenerator::ReportOptions& options);

public slots:
    /**
     * @brief Update progress bar and status during report generation
     * @param percentage Progress percentage (0-100)
     * @param stage Current generation stage description
     */
    void onReportProgress(int percentage, const QString& stage);

    /**
     * @brief Handle report generation completion
     * @param success Whether generation was successful
     * @param message Success or error message
     */
    void onReportFinished(bool success, const QString& message);

signals:
    /**
     * @brief Emitted when user requests report generation
     * @param options Configured report options
     */
    void generateReportRequested(const PDFReportGenerator::ReportOptions& options);

private slots:
    /**
     * @brief Handle Generate Report button click
     */
    void onGenerateButtonClicked();

    /**
     * @brief Handle Browse button for output path
     */
    void onBrowseOutputPathClicked();

    /**
     * @brief Handle Browse button for logo path
     */
    void onBrowseLogoPathClicked();

    /**
     * @brief Handle Cancel button click
     */
    void onCancelButtonClicked();

private:
    /**
     * @brief Set up the dialog UI layout and widgets
     */
    void setupUI();

    /**
     * @brief Create the general information group
     * @return QGroupBox containing general info fields
     */
    QGroupBox* createGeneralInfoGroup();

    /**
     * @brief Create the content options group
     * @return QGroupBox containing content checkboxes
     */
    QGroupBox* createContentOptionsGroup();

    /**
     * @brief Create the output path group
     * @return QGroupBox containing output path selection
     */
    QGroupBox* createOutputPathGroup();

    /**
     * @brief Create the progress and action buttons group
     * @return QWidget containing progress bar and buttons
     */
    QWidget* createProgressAndButtonsGroup();

    /**
     * @brief Validate user inputs
     * @return true if all required fields are valid
     */
    bool validateInputs() const;

    /**
     * @brief Show validation error message
     * @param message Error message to display
     */
    void showValidationError(const QString& message);

    // UI Components - General Info Group
    QLineEdit* m_reportTitleEdit;
    QLineEdit* m_companyNameEdit;
    QLineEdit* m_operatorNameEdit;
    QLineEdit* m_logoPathEdit;
    QPushButton* m_browseLogoButton;

    // UI Components - Content Options Group
    QCheckBox* m_includeChartsCheckBox;
    QCheckBox* m_includeScreenshotsCheckBox;
    QCheckBox* m_includeRecommendationsCheckBox;
    QCheckBox* m_includeDetailedMetricsCheckBox;

    // UI Components - Output Path Group
    QLineEdit* m_outputPathEdit;
    QPushButton* m_browseOutputButton;

    // UI Components - Progress and Buttons
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QPushButton* m_generateButton;
    QPushButton* m_cancelButton;

    // Dependencies
    IMainView* m_mainView;

    // State
    bool m_isGenerating;
};

#endif // REPORTOPTIONSDIALOG_H
