#include "quality/PDFReportGenerator.h"
#include <QPainter>
#include <QPagedPaintDevice>
#include <QTextDocument>
#include <QTextCursor>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

/**
 * @brief Private implementation data for PDFReportGenerator
 */
struct PDFReportGenerator::PrivateData {
    std::unique_ptr<QPrinter> printer;
    QString templatePath;
    QMap<QString, QPixmap> customCharts;
    int currentPageY = 0;
    int pageHeight = 0;
    int pageWidth = 0;
};

PDFReportGenerator::PDFReportGenerator(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<PrivateData>())
{
}

PDFReportGenerator::~PDFReportGenerator() = default;

bool PDFReportGenerator::generateReport(const QualityReport& report, const ReportOptions& options)
{
    emit progressUpdated(0, "Initializing PDF report generation...");
    
    if (!initializePrinter(options.outputPath)) {
        return false;
    }
    
    QPainter painter(d->printer.get());
    if (!painter.isActive()) {
        m_lastError = "Failed to initialize PDF painter";
        return false;
    }
    
    try {
        // Set up page dimensions
        QRect pageRect = d->printer->pageRect(QPrinter::DevicePixel);
        d->pageWidth = pageRect.width() - 2 * PAGE_MARGIN;
        d->pageHeight = pageRect.height() - 2 * PAGE_MARGIN;
        d->currentPageY = PAGE_MARGIN;
        
        emit progressUpdated(10, "Generating report header...");
        generateReportHeader(painter, options);
        
        emit progressUpdated(20, "Generating executive summary...");
        generateExecutiveSummary(painter, report);
        
        emit progressUpdated(40, "Generating metrics table...");
        generateMetricsTable(painter, report.metrics);
        
        if (options.includeCharts) {
            emit progressUpdated(60, "Generating charts...");
            generateChartsSection(painter, report);
        }
        
        if (options.includeRecommendations) {
            emit progressUpdated(80, "Generating recommendations...");
            generateRecommendationsSection(painter, report.recommendations);
        }
        
        if (options.includeDetailedMetrics) {
            emit progressUpdated(90, "Generating detailed analysis...");
            generateDetailedAnalysis(painter, report);
        }
        
        generateFooter(painter, options);
        
        painter.end();
        
        emit progressUpdated(100, "PDF report generation completed");
        emit reportGenerated(options.outputPath);
        
        qDebug() << "PDFReportGenerator: Report generated successfully:" << options.outputPath;
        return true;
        
    } catch (const std::exception& e) {
        m_lastError = QString("Error generating report: %1").arg(e.what());
        qWarning() << m_lastError;
        return false;
    }
}

bool PDFReportGenerator::generateMultiScanReport(const QList<QualityReport>& reports, const ReportOptions& options)
{
    if (reports.isEmpty()) {
        m_lastError = "No reports provided for multi-scan report";
        return false;
    }
    
    emit progressUpdated(0, "Generating multi-scan report...");
    
    // For now, generate individual reports and combine
    // In a full implementation, this would create a comprehensive multi-scan analysis
    bool success = true;
    
    for (int i = 0; i < reports.size(); ++i) {
        ReportOptions scanOptions = options;
        QFileInfo fileInfo(options.outputPath);
        QString baseName = fileInfo.completeBaseName();
        QString extension = fileInfo.suffix();
        QString dir = fileInfo.absolutePath();
        
        scanOptions.outputPath = QString("%1/%2_scan%3.%4")
                                 .arg(dir)
                                 .arg(baseName)
                                 .arg(i + 1)
                                 .arg(extension);
        
        if (!generateReport(reports[i], scanOptions)) {
            success = false;
            break;
        }
        
        int progress = ((i + 1) * 100) / reports.size();
        emit progressUpdated(progress, QString("Generated report %1 of %2").arg(i + 1).arg(reports.size()));
    }
    
    return success;
}

void PDFReportGenerator::setReportTemplate(const QString& templatePath)
{
    d->templatePath = templatePath;
}

void PDFReportGenerator::addCustomChart(const QString& chartName, const QPixmap& chartImage)
{
    d->customCharts[chartName] = chartImage;
}

bool PDFReportGenerator::initializePrinter(const QString& outputPath)
{
    QFileInfo fileInfo(outputPath);
    if (!fileInfo.dir().exists()) {
        m_lastError = "Output directory does not exist";
        return false;
    }
    
    d->printer = std::make_unique<QPrinter>(QPrinter::HighResolution);
    d->printer->setOutputFormat(QPrinter::PdfFormat);
    d->printer->setOutputFileName(outputPath);
    d->printer->setPageSize(QPageSize::A4);
    d->printer->setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
    
    return true;
}

void PDFReportGenerator::generateReportHeader(QPainter& painter, const ReportOptions& options)
{
    QFont titleFont("Arial", 24, QFont::Bold);
    QFont subtitleFont("Arial", 14);
    QFont infoFont("Arial", 10);
    
    // Draw company logo if provided
    if (!options.logoPath.isEmpty() && QFileInfo::exists(options.logoPath)) {
        QPixmap logo(options.logoPath);
        if (!logo.isNull()) {
            QRect logoRect(PAGE_MARGIN, d->currentPageY, 100, 50);
            painter.drawPixmap(logoRect, logo);
        }
    }
    
    // Draw title
    QRect titleRect(PAGE_MARGIN + 120, d->currentPageY, d->pageWidth - 120, 60);
    drawText(painter, titleRect, "Point Cloud Registration Quality Report", titleFont, Qt::AlignLeft | Qt::AlignVCenter);
    
    d->currentPageY += 80;
    
    // Draw project information
    QRect infoRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 100);
    QString projectInfo = QString("Project: %1\nCompany: %2\nOperator: %3\nGenerated: %4")
                         .arg(options.projectName)
                         .arg(options.companyName)
                         .arg(options.operatorName)
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    
    drawText(painter, infoRect, projectInfo, infoFont);
    
    d->currentPageY += 120;
    
    // Draw separator line
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(PAGE_MARGIN, d->currentPageY, PAGE_MARGIN + d->pageWidth, d->currentPageY);
    d->currentPageY += SECTION_SPACING;
}

void PDFReportGenerator::generateExecutiveSummary(QPainter& painter, const QualityReport& report)
{
    QFont sectionFont("Arial", 16, QFont::Bold);
    QFont textFont("Arial", 11);
    
    // Section title
    QRect titleRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 30);
    drawText(painter, titleRect, "Executive Summary", sectionFont);
    d->currentPageY += 40;
    
    // Summary content
    QString summaryText = QString(
        "This report presents the quality assessment results for the point cloud registration process. "
        "The analysis evaluated alignment accuracy, coverage overlap, and geometric quality metrics.\n\n"
        "Key Results:\n"
        "• Registration Quality Grade: %1\n"
        "• Root Mean Square Error: %2 meters\n"
        "• Overlap Coverage: %3%\n"
        "• Confidence Score: %4%\n\n"
        "%5"
    ).arg(report.metrics.qualityGrade)
     .arg(report.metrics.rootMeanSquaredError, 0, 'f', 4)
     .arg(report.metrics.overlapPercentage, 0, 'f', 1)
     .arg(report.metrics.confidenceScore * 100, 0, 'f', 1)
     .arg(report.summary);
    
    QRect summaryRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 200);
    drawText(painter, summaryRect, summaryText, textFont);
    d->currentPageY += 220;
}

void PDFReportGenerator::generateMetricsTable(QPainter& painter, const QualityMetrics& metrics)
{
    QFont sectionFont("Arial", 16, QFont::Bold);
    QFont tableFont("Arial", 10);
    
    // Section title
    QRect titleRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 30);
    drawText(painter, titleRect, "Quality Metrics", sectionFont);
    d->currentPageY += 40;
    
    // Prepare table data
    QStringList headers = {"Metric", "Value", "Unit"};
    QList<QStringList> rows;
    
    rows << QStringList{"Root Mean Square Error", QString::number(metrics.rootMeanSquaredError, 'f', 4), "meters"};
    rows << QStringList{"Standard Deviation", QString::number(metrics.standardDeviation, 'f', 4), "meters"};
    rows << QStringList{"Maximum Error", QString::number(metrics.maxError, 'f', 4), "meters"};
    rows << QStringList{"Minimum Error", QString::number(metrics.minError, 'f', 4), "meters"};
    rows << QStringList{"Overlap Percentage", QString::number(metrics.overlapPercentage, 'f', 1), "%"};
    rows << QStringList{"Average Point Density", QString::number(metrics.averagePointDensity, 'f', 2), "points/voxel"};
    rows << QStringList{"Density Variation", QString::number(metrics.densityVariation, 'f', 3), "ratio"};
    rows << QStringList{"Planarity", QString::number(metrics.planarity, 'f', 3), "ratio"};
    rows << QStringList{"Sphericity", QString::number(metrics.sphericity, 'f', 3), "ratio"};
    rows << QStringList{"Linearity", QString::number(metrics.linearity, 'f', 3), "ratio"};
    rows << QStringList{"Confidence Score", QString::number(metrics.confidenceScore, 'f', 3), "ratio"};
    rows << QStringList{"Quality Grade", metrics.qualityGrade, "-"};
    
    QRect tableRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, rows.size() * 25 + 30);
    drawTable(painter, tableRect, headers, rows);
    d->currentPageY += tableRect.height() + SECTION_SPACING;
}

void PDFReportGenerator::generateChartsSection(QPainter& painter, const QualityReport& report)
{
    QFont sectionFont("Arial", 16, QFont::Bold);
    
    // Section title
    QRect titleRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 30);
    drawText(painter, titleRect, "Quality Analysis Charts", sectionFont);
    d->currentPageY += 40;
    
    // Check if we need a new page
    if (d->currentPageY + CHART_HEIGHT * 2 > d->pageHeight) {
        newPage(painter);
    }
    
    // Generate and draw charts
    int chartX = PAGE_MARGIN;
    int chartY = d->currentPageY;
    
    // Quality grade chart
    QPixmap gradeChart = generateQualityGradeChart(report.metrics.qualityGrade, report.metrics.confidenceScore);
    if (!gradeChart.isNull()) {
        QRect gradeRect(chartX, chartY, CHART_WIDTH / 2, CHART_HEIGHT / 2);
        painter.drawPixmap(gradeRect, gradeChart);
    }
    
    // Overlap visualization
    QPixmap overlapChart = generateOverlapVisualization(report.metrics.overlapPercentage);
    if (!overlapChart.isNull()) {
        QRect overlapRect(chartX + CHART_WIDTH / 2 + 20, chartY, CHART_WIDTH / 2, CHART_HEIGHT / 2);
        painter.drawPixmap(overlapRect, overlapChart);
    }
    
    d->currentPageY += CHART_HEIGHT / 2 + 40;
    
    // Error distribution chart
    if (!report.errorDistribution.empty()) {
        QPixmap errorChart = generateErrorDistributionChart(report.errorDistribution);
        if (!errorChart.isNull()) {
            QRect errorRect(PAGE_MARGIN, d->currentPageY, CHART_WIDTH, CHART_HEIGHT / 2);
            painter.drawPixmap(errorRect, errorChart);
            d->currentPageY += CHART_HEIGHT / 2 + SECTION_SPACING;
        }
    }
}

void PDFReportGenerator::generateRecommendationsSection(QPainter& painter, const QStringList& recommendations)
{
    QFont sectionFont("Arial", 16, QFont::Bold);
    QFont textFont("Arial", 11);

    // Section title
    QRect titleRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 30);
    drawText(painter, titleRect, "Recommendations", sectionFont);
    d->currentPageY += 40;

    // Recommendations list
    QString recommendationsText;
    for (int i = 0; i < recommendations.size(); ++i) {
        recommendationsText += QString("• %1\n").arg(recommendations[i]);
    }

    QRect recommendationsRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, recommendations.size() * 20 + 20);
    drawText(painter, recommendationsRect, recommendationsText, textFont);
    d->currentPageY += recommendationsRect.height() + SECTION_SPACING;
}

void PDFReportGenerator::generateDetailedAnalysis(QPainter& painter, const QualityReport& report)
{
    QFont sectionFont("Arial", 16, QFont::Bold);
    QFont textFont("Arial", 10);

    // Section title
    QRect titleRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 30);
    drawText(painter, titleRect, "Detailed Analysis", sectionFont);
    d->currentPageY += 40;

    QString detailedText = QString(
        "Assessment Time: %1\n"
        "Total Points Analyzed: %2\n"
        "Overlapping Points: %3\n\n"
        "Statistical Analysis:\n"
        "The registration process achieved a root mean square error of %4 meters, "
        "indicating %5 alignment quality. The overlap coverage of %6% suggests %7 "
        "spatial correspondence between the point clouds.\n\n"
        "Geometric Quality:\n"
        "Planarity: %8 (higher values indicate more planar surfaces)\n"
        "Sphericity: %9 (higher values indicate more spherical features)\n"
        "Linearity: %10 (higher values indicate more linear features)\n\n"
        "The overall confidence score of %11 reflects the reliability of the registration results."
    ).arg(report.assessmentTime.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(report.metrics.totalPoints)
     .arg(report.metrics.overlappingPoints)
     .arg(report.metrics.rootMeanSquaredError, 0, 'f', 4)
     .arg(report.metrics.qualityGrade == "A" || report.metrics.qualityGrade == "B" ? "excellent" :
          report.metrics.qualityGrade == "C" ? "acceptable" : "poor")
     .arg(report.metrics.overlapPercentage, 0, 'f', 1)
     .arg(report.metrics.overlapPercentage > 70 ? "good" :
          report.metrics.overlapPercentage > 50 ? "adequate" : "insufficient")
     .arg(report.metrics.planarity, 0, 'f', 3)
     .arg(report.metrics.sphericity, 0, 'f', 3)
     .arg(report.metrics.linearity, 0, 'f', 3)
     .arg(report.metrics.confidenceScore, 0, 'f', 3);

    QRect detailedRect(PAGE_MARGIN, d->currentPageY, d->pageWidth, 300);
    drawText(painter, detailedRect, detailedText, textFont);
    d->currentPageY += 320;
}

void PDFReportGenerator::generateFooter(QPainter& painter, const ReportOptions& options)
{
    QFont footerFont("Arial", 8);

    // Move to bottom of page
    int footerY = d->pageHeight + PAGE_MARGIN - 30;

    QString footerText = QString("Generated by FARO Scene Registration MVP - %1")
                        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    QRect footerRect(PAGE_MARGIN, footerY, d->pageWidth, 20);
    drawText(painter, footerRect, footerText, footerFont, Qt::AlignCenter);
}

QPixmap PDFReportGenerator::generateErrorDistributionChart(const std::vector<double>& errorDistribution)
{
    // Create a simple histogram chart
    QPixmap chart(CHART_WIDTH, CHART_HEIGHT);
    chart.fill(Qt::white);

    QPainter painter(&chart);
    painter.setRenderHint(QPainter::Antialiasing);

    if (errorDistribution.empty()) {
        painter.drawText(chart.rect(), Qt::AlignCenter, "No error data available");
        return chart;
    }

    // Draw title
    QFont titleFont("Arial", 12, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 10, CHART_WIDTH, 30), Qt::AlignCenter, "Error Distribution");

    // Simple bar chart representation
    int chartArea = CHART_HEIGHT - 80;
    int barWidth = CHART_WIDTH / std::min(static_cast<int>(errorDistribution.size()), 20);

    double maxError = *std::max_element(errorDistribution.begin(), errorDistribution.end());
    if (maxError > 0) {
        for (size_t i = 0; i < std::min(errorDistribution.size(), static_cast<size_t>(20)); ++i) {
            int barHeight = static_cast<int>((errorDistribution[i] / maxError) * chartArea);
            QRect barRect(i * barWidth + 20, CHART_HEIGHT - 40 - barHeight, barWidth - 2, barHeight);
            painter.fillRect(barRect, QColor(100, 150, 200));
        }
    }

    return chart;
}

QPixmap PDFReportGenerator::generateQualityGradeChart(const QString& grade, double confidenceScore)
{
    QPixmap chart(CHART_WIDTH / 2, CHART_HEIGHT / 2);
    chart.fill(Qt::white);

    QPainter painter(&chart);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw title
    QFont titleFont("Arial", 10, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 5, chart.width(), 20), Qt::AlignCenter, "Quality Grade");

    // Draw grade circle
    int centerX = chart.width() / 2;
    int centerY = chart.height() / 2 + 10;
    int radius = std::min(chart.width(), chart.height()) / 4;

    QColor gradeColor;
    if (grade == "A") gradeColor = QColor(0, 200, 0);
    else if (grade == "B") gradeColor = QColor(150, 200, 0);
    else if (grade == "C") gradeColor = QColor(255, 200, 0);
    else if (grade == "D") gradeColor = QColor(255, 150, 0);
    else gradeColor = QColor(255, 0, 0);

    painter.setBrush(gradeColor);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);

    // Draw grade text
    QFont gradeFont("Arial", 16, QFont::Bold);
    painter.setFont(gradeFont);
    painter.setPen(Qt::black);
    painter.drawText(QRect(centerX - radius, centerY - radius, radius * 2, radius * 2),
                    Qt::AlignCenter, grade);

    // Draw confidence score
    QFont scoreFont("Arial", 8);
    painter.setFont(scoreFont);
    QString scoreText = QString("Confidence: %1%").arg(confidenceScore * 100, 0, 'f', 1);
    painter.drawText(QRect(0, chart.height() - 20, chart.width(), 15), Qt::AlignCenter, scoreText);

    return chart;
}

QPixmap PDFReportGenerator::generateOverlapVisualization(double overlapPercentage)
{
    QPixmap chart(CHART_WIDTH / 2, CHART_HEIGHT / 2);
    chart.fill(Qt::white);

    QPainter painter(&chart);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw title
    QFont titleFont("Arial", 10, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRect(0, 5, chart.width(), 20), Qt::AlignCenter, "Overlap Coverage");

    // Draw progress bar style visualization
    int barWidth = chart.width() - 40;
    int barHeight = 20;
    int barX = 20;
    int barY = chart.height() / 2;

    // Background bar
    painter.setBrush(QColor(220, 220, 220));
    painter.setPen(Qt::black);
    painter.drawRect(barX, barY, barWidth, barHeight);

    // Filled portion
    int filledWidth = static_cast<int>((overlapPercentage / 100.0) * barWidth);
    QColor fillColor = overlapPercentage > 70 ? QColor(0, 200, 0) :
                      overlapPercentage > 50 ? QColor(255, 200, 0) : QColor(255, 100, 100);
    painter.setBrush(fillColor);
    painter.drawRect(barX, barY, filledWidth, barHeight);

    // Percentage text
    QFont percentFont("Arial", 12, QFont::Bold);
    painter.setFont(percentFont);
    painter.setPen(Qt::black);
    QString percentText = QString("%1%").arg(overlapPercentage, 0, 'f', 1);
    painter.drawText(QRect(0, barY + 30, chart.width(), 20), Qt::AlignCenter, percentText);

    return chart;
}

QPixmap PDFReportGenerator::generateDensityChart(double averageDensity, double densityVariation)
{
    QPixmap chart(CHART_WIDTH, CHART_HEIGHT / 2);
    chart.fill(Qt::white);

    QPainter painter(&chart);
    painter.setRenderHint(QPainter::Antialiasing);

    // Simple density visualization - placeholder implementation
    painter.drawText(chart.rect(), Qt::AlignCenter,
                    QString("Avg Density: %1\nVariation: %2")
                    .arg(averageDensity, 0, 'f', 2)
                    .arg(densityVariation, 0, 'f', 3));

    return chart;
}

void PDFReportGenerator::drawText(QPainter& painter, const QRect& rect, const QString& text,
                                const QFont& font, Qt::Alignment alignment)
{
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(rect, alignment | Qt::TextWordWrap, text);
}

void PDFReportGenerator::drawTable(QPainter& painter, const QRect& rect, const QStringList& headers,
                                 const QList<QStringList>& rows)
{
    QFont tableFont("Arial", 9);
    painter.setFont(tableFont);

    int rowHeight = 25;
    int colWidth = rect.width() / headers.size();

    // Draw headers
    painter.setBrush(QColor(240, 240, 240));
    painter.setPen(Qt::black);

    for (int col = 0; col < headers.size(); ++col) {
        QRect headerRect(rect.x() + col * colWidth, rect.y(), colWidth, rowHeight);
        painter.drawRect(headerRect);
        painter.drawText(headerRect, Qt::AlignCenter, headers[col]);
    }

    // Draw rows
    painter.setBrush(Qt::white);
    for (int row = 0; row < rows.size(); ++row) {
        for (int col = 0; col < rows[row].size() && col < headers.size(); ++col) {
            QRect cellRect(rect.x() + col * colWidth, rect.y() + (row + 1) * rowHeight, colWidth, rowHeight);
            painter.drawRect(cellRect);
            painter.drawText(cellRect, Qt::AlignCenter, rows[row][col]);
        }
    }
}

QRect PDFReportGenerator::calculateTextRect(const QString& text, const QFont& font)
{
    QFontMetrics metrics(font);
    return metrics.boundingRect(text);
}

void PDFReportGenerator::newPage(QPainter& painter)
{
    d->printer->newPage();
    d->currentPageY = PAGE_MARGIN;
}
