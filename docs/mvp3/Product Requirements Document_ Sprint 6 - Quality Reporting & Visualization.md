# **Product Requirements Document: Sprint 6 - Quality Reporting & Visualization**

This PRD details the requirements for **Sprint 6: Quality Reporting &
Visualization**, a crucial step towards achieving the Minimum
Competitive Product (MCP). The primary goal of this sprint is to expose
advanced quality assessment and reporting tools to the user, providing
visual and document-based feedback on point cloud registration results.

## Sprint 6 Overview

- **Sprint Goal:** Provide users with visual feedback on registration
  quality through a colorized deviation map in the 3D viewer and enable
  the generation of comprehensive PDF quality reports.

- **Context:** This sprint builds upon the completed MVP workflows
  (manual and automatic ICP/target-based alignment). It integrates
  DifferenceAnalysis for visual deviation maps and leverages the
  QualityAssessment and PDFReportGenerator components for reporting.

## Sub-Sprint 6.1: Colorized Deviation Map Integration & Visualization

### Goal

Display a colorized deviation map between registered scans directly in
the 3D viewer, providing immediate visual feedback on registration
quality.

### User Stories

- **As a user,** after performing a registration, I want to see a
  colorized map of the deviation between the aligned scans in the 3D
  viewer so I can quickly identify areas of good or poor alignment.

- **As a user,** I want to understand what the colors represent (e.g., a
  color scale/legend) to interpret the deviation map accurately.

- **As a user,** I want to be able to toggle the deviation map
  visualization on and off.

### UI/UX

- **ViewerToolbar (or PointCloudViewerWidget controls)**:

  - Add a new QAction (e.g., \"Show Deviation Map\") or QCheckBox (e.g.,
    \"Enable Deviation Map\") to the viewer\'s toolbar or a dedicated
    \"Visualization\" panel. This control should be enabled only when at
    least two scans are loaded and a registration result (from previous
    sprints) is available.

- **PointCloudViewerWidget**:

  - When the \"Show Deviation Map\" is toggled on, the viewer will
    render the transformed point cloud(s) with points colored according
    to their deviation distance.

  - Implement a simple color scale legend overlay (e.g., on the corner
    of the screen) to visually represent deviation values (e.g., green
    for low deviation, red for high deviation).

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleShowDeviationMapToggled(bool enabled) slot,
    connected to the UI toggle.

  - When enabled:

    - Retrieve the necessary point cloud data (std::vector\<Point3D\>)
      for the source and target scans from PointCloudLoadManager
      (transformed source, original target).

    - Retrieve the relevant TransformationMatrix from
      RegistrationProject for the active registration.

    - Call AlignmentEngine::analyzeDeviation(sourcePoints, targetPoints,
      transform) (a new method in AlignmentEngine) to get the deviation
      distances and color values.

    - Pass the calculated colorized point data to
      PointCloudViewerWidget::loadColorizedPointCloud(std::vector\<PointFullData\>&
      points).

  - When disabled:

    - Instruct PointCloudViewerWidget to revert to its default rendering
      (e.g., original colors or uniform color).

- **PointCloudViewerWidget**:

  - Needs a new public method: loadColorizedPointCloud(const
    std::vector\<PointFullData\>& colorizedPoints). This method will
    update the internal vertex buffers to use the provided color
    information and enable color rendering.

  - Needs to retain the original point data for the point cloud(s) to
    revert to when the deviation map is toggled off.

  - Needs to implement the overlay for the color legend (using QPainter
    in paintOverlayGL()).

- **AlignmentEngine**:

  - Add analyzeDeviation(const PointCloud& source, const PointCloud&
    target, const QMatrix4x4& transform) method.

  - This method will internally call
    Analysis::DifferenceAnalysis::calculateDistances() and
    Analysis::DifferenceAnalysis::generateColorMapValues().

  - It will transform the output QVector\<float\> (distances) into
    std::vector\<PointFullData\> where each point has x, y, z and r, g,
    b (derived from the color map).

  - Needs to return the colorized point data.

### Backend (Core Logic/Services)

- **Analysis::DifferenceAnalysis**:

  - calculateDistances(const std::vector\<Point3D\>& sourcePoints, const
    std::vector\<Point3D\>& targetPoints, const QMatrix4x4& transform):
    Already exists. This will compute the distances.

  - generateColorMapValues(const QVector\<float\>& distances, float
    maxDistance): Already exists. This will generate normalized color
    values (\[0,1\] range for red-green-blue spectrum mapping).

  - Needs a helper to convert QVector\<float\> of normalized color
    values into actual RGB QColor or uint8_t components. This should be
    a utility function within DifferenceAnalysis (e.g.,
    mapDistanceToColor(float normalizedDistance)).

- **PointCloudLoadManager**:

  - Must be able to provide the raw (or base-transformed) point cloud
    data for both source and target scans from the active registration
    (getLoadedPointFullData(const QString& scanId)).

### Tests

- **Unit Tests:**

  - tests/analysis/test_difference_analysis.cpp (new test file, if not
    already existing from Sprint 9):

    - Test calculateDistances() correctness.

    - Test generateColorMapValues() correctness (e.g., for known min/max
      distances, outputs fall within \[0,1\] range).

    - Test mapDistanceToColor() utility function to ensure correct color
      mapping.

  - test_alignmentengine.cpp:

    - Test analyzeDeviation() correctly calls DifferenceAnalysis and
      produces PointFullData with valid colors.

  - test_pointcloudviewerwidget.cpp:

    - Test loadColorizedPointCloud() updates the viewer\'s buffer and
      enables color rendering.

    - Test the drawing of the color legend overlay.

- **Integration Tests:**

  - Extend tests/integration/automatic_alignment_e2e_test.cpp or
    tests/integration/target_based_alignment_e2e_test.cpp.

  - Simulate a complete registration.

  - Simulate toggling \"Show Deviation Map\" on.

  - Verify (mocked UI verification) that
    PointCloudViewerWidget::loadColorizedPointCloud() is called and the
    deviation map is rendered.

  - Verify the color legend appears.

### Acceptance Criteria

- A UI control exists to toggle the colorized deviation map.

- When enabled, registered point clouds are displayed with a color scale
  representing deviation.

- A clear color legend (e.g., green for low, red for high deviation) is
  displayed in the viewer overlay.

- The deviation map accurately reflects the output of
  DifferenceAnalysis.

## Sub-Sprint 6.2: PDF Report Generation UI & Backend Hookup

### Goal

Enable users to generate a professional PDF quality report for the
current project registration results.

### User Stories

- **As a user,** after assessing registration quality, I want to click a
  button to generate a detailed PDF report summarising the results.

- **As a user,** I want to be prompted to save the PDF report to a
  specified location.

### UI/UX

- **Menu Bar**: The \"Quality\" menu in the main application should
  contain a \"Generate Quality Report\...\" action (already outlined in
  mainwindow.h as m_generateReportAction).

- **Enabling**: This action should be enabled only after a quality
  assessment has been performed (i.e., QualityAssessment has run, and
  m_lastQualityReport in MainWindow is set).

- **Save Dialog**: A standard file save dialog (QFileDialog) will
  appear, prompting the user for a save location and filename for the
  PDF.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - Implement handleGenerateReportClicked() slot, connected to
    m_generateReportAction.

  - Inside handleGenerateReportClicked():

    - Retrieve the QualityReport object from the last quality assessment
      (m_lastQualityReport would be set in MainWindow\'s
      onQualityAssessmentCompleted()).

    - If m_lastQualityReport is null, display an error message.

    - Call IMainView::askForSaveFilePath() to get the desired output
      path (defaulting to a sensible name like
      \[ProjectName\]\_QualityReport.pdf).

    - Instantiate PDFReportGenerator (if not already managed globally).

    - Prepare PDFReportGenerator::ReportOptions (e.g., outputPath,
      projectName, companyName, operatorName).

    - Call PDFReportGenerator::generatePdfReport(report, options).

    - Display success/failure messages to the user via
      IMainView::displayInfoMessage() or
      IMainView::displayErrorMessage().

- **MainWindow (as IMainView)**:

  - Ensure m_generateReportAction is enabled/disabled correctly. It
    should be enabled once a quality assessment has completed
    (onQualityAssessmentCompleted() slot).

  - The m_lastQualityReport member in MainWindow (which holds the result
    of QualityAssessment) must be set and accessible.

- **QualityAssessment**:

  - The assessRegistrationQuality() method already returns a
    QualityReport struct. This is the report that will be used for PDF
    generation.

### Backend (Core Logic/Services)

- **QualityAssessment**:

  - assessRegistrationQuality(): This method (already exists from Sprint
    6: analyse the previous response and produce an MCP.(1).md) performs
    the comprehensive assessment and returns a QualityReport.

- **PDFReportGenerator**:

  - generatePdfReport(const QualityReport& report, const ReportOptions&
    options): This method (already exists from Sprint 6: analyse the
    previous response and produce an MCP.(1).md) will generate the PDF.
    It should be able to accept ReportOptions to customize the report
    content.

- **ReportOptions**: This struct (defined in PDFReportGenerator.h)
  should contain fields for outputPath, projectName, companyName,
  reportTitle, operatorName, and flags for includeCharts,
  includeScreenshots, includeRecommendations, includeDetailedMetrics.

### Tests

- **Unit Tests:**

  - tests/quality/test_quality_reporting.cpp:

    - Expand testQualityReportWorkflow() to specifically mock
      MainPresenter\'s calls to PDFReportGenerator with a QualityReport
      and ReportOptions.

    - Verify the generatePdfReport method is called with the correct
      QualityReport and ReportOptions.

  - test_mainpresenter.cpp:

    - Add tests for handleGenerateReportClicked():

      - Verify IMainView::askForSaveFilePath() is called.

      - Verify PDFReportGenerator::generatePdfReport() is called with
        appropriate parameters.

      - Verify IMainView::displayInfoMessage() or displayErrorMessage()
        is called upon completion.

- **Integration Tests:**

  - Extend end-to-end tests (Sub-Sprint 6.4) to include generating a
    report after a successful registration and quality assessment.

  - Verify that m_generateReportAction becomes enabled after
    QualityAssessment runs.

  - Simulate clicking \"Generate Report\" and verify a file is created
    (mocked) and the success message is displayed.

### Acceptance Criteria

- The \"Generate Quality Report\" menu action is correctly
  enabled/disabled.

- Clicking the action prompts the user for a save location for the PDF.

- A PDF report is generated successfully based on the last quality
  assessment.

- Success/failure messages are displayed to the user after report
  generation.

## Sub-Sprint 6.3: Enhanced Report Options & Status

### Goal

Provide users with a dialog to customize the content of the PDF report
and display real-time progress during report generation.

### User Stories

- **As a user,** I want to choose what information is included in the
  PDF report (e.g., charts, detailed metrics, recommendations).

- **As a user,** I want to see a progress bar and status updates while
  the PDF report is being generated.

- **As a user,** I want to be able to set a company logo and name for
  branding in the report.

### UI/UX

- **ReportOptionsDialog (New UI Component, similar to ExportDialog)**:

  - This modal dialog will be launched when \"Generate Quality
    Report\...\" is clicked.

  - **General Info**: Fields for reportTitle, companyName, operatorName,
    logoPath (with a \"Browse\" button).

  - **Content Options (Checkboxes)**:

    - \"Include Charts\"

    - \"Include Screenshots\" (placeholder for future implementation)

    - \"Include Recommendations\"

    - \"Include Detailed Metrics\"

  - **Output Path**: Input field for outputPath with a \"Browse\"
    button.

  - **Buttons**: \"Generate\", \"Cancel\", \"Preview\" (optional, for
    viewing report layout without saving).

  - A QProgressBar and status label for real-time progress feedback.

### Frontend (Component/View Layer)

- **MainPresenter**:

  - In handleGenerateReportClicked():

    - Instead of directly calling PDFReportGenerator, instantiate and
      show the new ReportOptionsDialog.

    - Initialize the dialog with default ReportOptions (e.g., from
      UserPreferences or hardcoded defaults).

    - Connect ReportOptionsDialog::generateReportRequested(ReportOptions
      options) signal to a new
      MainPresenter::startReportGeneration(ReportOptions options) slot.

  - New slot startReportGeneration(ReportOptions options):

    - Call PDFReportGenerator::generatePdfReport(m_lastQualityReport,
      options).

    - Connect PDFReportGenerator::reportProgress to
      ReportOptionsDialog::onReportProgress.

    - Connect PDFReportGenerator::reportGenerated and reportError to
      ReportOptionsDialog::onReportFinished.

- **ReportOptionsDialog.h/.cpp (New UI Component)**:

  - Implement its setupUI() to include all options.

  - Implement getReportOptions() to retrieve values from its UI.

  - Implement onBrowseLogoPath(), onBrowseOutputPath().

  - Implement onGenerateClicked() to emit
    generateReportRequested(options).

  - Implement onReportProgress(int percentage, const QString& stage) to
    update its internal progress bar and status label.

  - Implement onReportFinished(bool success, const QString& message) to
    update the dialog\'s final state and close itself if successful.

- **PDFReportGenerator**:

  - Ensure generatePdfReport() (already exists) emits reportProgress(int
    percentage, const QString& stage), reportGenerated(const QString&
    filePath), and reportError(const QString& error) signals.

### Backend (Core Logic/Services)

- **PDFReportGenerator**:

  - Confirm PDFReportGenerator.h contains ReportOptions struct with all
    necessary fields (e.g., includeCharts, includeRecommendations,
    logoPath, companyName, reportTitle, operatorName).

  - Ensure generatePdfReport implementation correctly uses these options
    to customize the PDF content.

### Tests

- **Unit Tests:**

  - tests/quality/test_pdf_report_generator.cpp (new test file, if not
    already existing from Sprint 6):

    - Test generatePdfReport() with various ReportOptions combinations
      (e.g., including/excluding charts, different company names).

    - Verify reportProgress, reportGenerated, reportError signals are
      emitted correctly.

  - test_reportoptionsdialog.cpp (new test file):

    - Verify UI layout and parameter retrieval.

    - Test onGenerateClicked() emits generateReportRequested with
      correct ReportOptions.

    - Test onReportProgress and onReportFinished update the dialog\'s UI
      correctly.

  - test_mainpresenter.cpp:

    - Test handleGenerateReportClicked() correctly launches
      ReportOptionsDialog and startReportGeneration().

    - Test startReportGeneration() calls PDFReportGenerator and connects
      signals correctly.

- **Integration Tests:**

  - Extend end-to-end test (Sub-Sprint 6.4).

  - Simulate triggering \"Generate Report\", interacting with
    ReportOptionsDialog (mocking input), and verifying report generation
    progress and final success/failure.

### Acceptance Criteria

- A ReportOptionsDialog allows users to customize report content.

- The dialog includes fields for project/company info and content
  checkboxes.

- Report generation progress is displayed in the dialog.

- The generated PDF report reflects the chosen options (e.g., contains
  charts if selected).

## Sub-Sprint 6.4: End-to-End Test & Workflow Integration

### Goal

Create a single, comprehensive automated integration test that simulates
the entire quality reporting workflow and ensures its seamless
integration within the application.

### User Stories (Internal/Developer)

- **As a developer,** I want an automated end-to-end test for the
  quality reporting MVP so that I can quickly verify the functionality
  is intact.

- **As a developer,** I want this test to cover the full interaction
  flow from registration to report generation, including visual
  feedback.

### Test Scope

This test will use mocked UI elements (e.g., file dialogs) but will
directly drive and verify the interaction between MainPresenter,
AlignmentEngine, QualityAssessment, PDFReportGenerator, and
PointCloudViewerWidget.

### Steps

1.  **Initialize Components:** Instantiate MainPresenter, MockMainView,
    MockE57Parser, MockPointCloudViewer, MockProjectManager,
    MockPointCloudLoadManager, AlignmentEngine, QualityAssessment, and
    PDFReportGenerator.

2.  **Mock Setup:** Configure mocks for successful parsing, loading, and
    alignment (using predefined transformations).

3.  **Simulate Project Creation & Scan Loading:**

    - Load two scans (e.g., Programmatically set their
      ScanInfo::transform in RegistrationProject to simulate a prior
      alignment).

    - Ensure these scans are loaded into PointCloudViewerWidget.

4.  **Simulate Quality Assessment:**

    - Call MainPresenter::handleQualityAssessmentClicked().

    - Verify QualityAssessment::assessRegistrationQuality() is called
      with appropriate Point data and transformations.

    - Verify MainWindow::onQualityAssessmentCompleted() is triggered,
      enabling the \"Generate Report\" action.

5.  **Simulate Deviation Map Toggle:**

    - Call MainPresenter::handleShowDeviationMapToggled(true).

    - Verify AlignmentEngine::analyzeDeviation() is called.

    - Verify PointCloudViewerWidget::loadColorizedPointCloud() is
      called.

    - Verify (mocked UI verification) that the deviation map is rendered
      and a legend is visible.

    - Call MainPresenter::handleShowDeviationMapToggled(false) and
      verify reversion.

6.  **Simulate Report Generation:**

    - Call MainPresenter::handleGenerateReportClicked().

    - Mock MockMainView::askForSaveFilePath() to return a valid PDF
      path.

    - Mock ReportOptionsDialog interaction to set desired options.

    - Verify PDFReportGenerator::generatePdfReport() is called with the
      correct QualityReport and ReportOptions.

    - Verify progress updates are sent and success/failure messages are
      displayed.

7.  **Verify Final State:** Assert that the final state of the
    application is consistent with a successful workflow (e.g., report
    generated, viewer state is correct).

### Tests

- **New File:**
  tests/integration/quality_reporting_workflow_e2e_test.cpp.

- **Dependencies:** All relevant components and mocks from previous
  sprints.

### Acceptance Criteria

- A new, dedicated end-to-end integration test file exists for quality
  reporting.

- The test runs automatically and passes successfully.

- The test covers the full quality reporting workflow: registration -\>
  assessment -\> deviation map -\> report generation.

- All critical interactions between components (MainPresenter,
  IMainView, IPointCloudViewer, AlignmentEngine, QualityAssessment,
  PDFReportGenerator, DifferenceAnalysis) are verified.
