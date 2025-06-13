#include "quality/PDFReportGenerator.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFont>
#include <QPainter>
#include <QPrinter>
#include <QStandardPaths>

PDFReportGenerator::PDFReportGenerator(QObject* parent) : QObject(parent) {}

PDFReportGenerator::~PDFReportGenerator() = default;

// Static factory method for ReportOptions
PDFReportGenerator::ReportOptions PDFReportGenerator::ReportOptions::createDefault(const QString& projectName)
{
    ReportOptions options;

    // Set project-specific defaults
    if (!projectName.isEmpty()) {
        options.projectName = projectName;
        options.reportTitle = QString("Quality Report - %1").arg(projectName);
    } else {
        options.projectName = "Untitled Project";
        options.reportTitle = "Point Cloud Registration Quality Report";
    }

    // Set other sensible defaults
    options.operatorName = "Default User";
    options.companyName = "CloudRegistration";
    options.includeCharts = false;
    options.includeScreenshots = false;
    options.includeRecommendations = true;
    options.includeDetailedMetrics = true;
    options.logoPath = ""; // Empty by default
    options.outputPath = ""; // Will be set by dialog

    return options;
}

// Sprint 6.2: New signature that emits signals
void PDFReportGenerator::generatePdfReport(const QualityReport& report, const ReportOptions& options)
{
    // Call the legacy method and emit appropriate signals
    bool success = generatePdfReport(report, options.outputPath, options);

    if (success) {
        emit reportGenerated(options.outputPath);
    } else {
        emit reportError("Failed to generate PDF report");
    }
}

bool PDFReportGenerator::generatePdfReport(const QualityReport& report,
                                           const QString& outputPath,
                                           const ReportOptions& options)
{
    try
    {
        emit reportProgress(0, "Initializing PDF generation");

        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(outputPath);
        printer.setPageSize(QPageSize::A4);
        printer.setPageMargins(QMarginsF(m_layout.leftMargin, m_layout.topMargin,
                                        m_layout.rightMargin, m_layout.bottomMargin),
                              QPageLayout::Point);

        QPainter painter;
        if (!painter.begin(&printer)) {
            emit reportError("Failed to start PDF painter.");
            return false;
        }

        emit reportProgress(10, "Drawing header");
        m_currentY = m_layout.topMargin;
        m_pageNumber = 1;

        drawHeader(painter, report, options);
        addVerticalSpace(20);

        emit reportProgress(30, "Drawing summary section");
        drawSummarySection(painter, report);
        addVerticalSpace(20);

        // Conditionally include detailed metrics based on options
        if (options.includeDetailedMetrics) {
            emit reportProgress(50, "Drawing metrics table");
            drawMetricsTable(painter, report);
            addVerticalSpace(20);
        }

        // Conditionally include charts based on options
        if (options.includeCharts) {
            emit reportProgress(70, "Drawing charts section");
            drawChartsSection(painter, report);
            addVerticalSpace(20);
        }

        // Conditionally include recommendations based on options
        if (options.includeRecommendations) {
            emit reportProgress(85, "Drawing recommendations");
            drawRecommendationsSection(painter, report);
            addVerticalSpace(20);
        }

        emit reportProgress(95, "Drawing footer");
        drawFooter(painter, options);

        painter.end();

        emit reportProgress(100, "PDF generation completed");
        emit reportGenerated(outputPath);
        return true;
    }
    catch (const std::exception& e)
    {
        emit reportError(QString("PDF generation failed: %1").arg(e.what()));
        return false;
    }
    catch (...)
    {
        emit reportError("PDF generation failed with unknown error");
        return false;
    }
}

void PDFReportGenerator::drawHeader(QPainter& painter, const QualityReport& report, const ReportOptions& options)
{
    QFont titleFont("Arial", m_fonts.title, QFont::Bold);
    QFont subtitleFont("Arial", m_fonts.subheading);

    painter.setFont(titleFont);
    painter.setPen(m_colors.primary);

    QRect titleRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 40);
    painter.drawText(titleRect, Qt::AlignCenter, options.reportTitle);
    m_currentY += 50;

    painter.setFont(subtitleFont);
    painter.setPen(m_colors.text);

    QString subtitle = QString("Generated: %1 | Project: %2")
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"))
                      .arg(report.projectName);

    QRect subtitleRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 20);
    painter.drawText(subtitleRect, Qt::AlignCenter, subtitle);
    m_currentY += 25;

    // Add operator and company information if provided
    if (!options.operatorName.isEmpty() || !options.companyName.isEmpty()) {
        QString operatorInfo;
        if (!options.operatorName.isEmpty() && !options.companyName.isEmpty()) {
            operatorInfo = QString("Operator: %1 | Company: %2")
                          .arg(options.operatorName)
                          .arg(options.companyName);
        } else if (!options.operatorName.isEmpty()) {
            operatorInfo = QString("Operator: %1").arg(options.operatorName);
        } else {
            operatorInfo = QString("Company: %1").arg(options.companyName);
        }

        QRect operatorRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 20);
        painter.drawText(operatorRect, Qt::AlignCenter, operatorInfo);
        m_currentY += 25;
    } else {
        m_currentY += 5; // Small spacing adjustment
    }

    // Draw separator line
    painter.setPen(QPen(m_colors.lightGrey, 2));
    painter.drawLine(m_layout.leftMargin, m_currentY,
                    m_layout.leftMargin + m_layout.contentWidth(), m_currentY);
    m_currentY += 10;
}

void PDFReportGenerator::drawSummarySection(QPainter& painter, const QualityReport& report)
{
    QFont headerFont("Arial", m_fonts.heading, QFont::Bold);
    QFont bodyFont("Arial", m_fonts.body);

    painter.setFont(headerFont);
    painter.setPen(m_colors.primary);

    QRect headerRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 25);
    painter.drawText(headerRect, Qt::AlignLeft, "Executive Summary");
    m_currentY += 35;

    painter.setFont(bodyFont);
    painter.setPen(m_colors.text);

    // Quality grade with color coding
    QColor gradeColor = m_colors.success;
    if (report.metrics.qualityGrade < 0.6f) gradeColor = m_colors.error;
    else if (report.metrics.qualityGrade < 0.8f) gradeColor = m_colors.warning;

    painter.setPen(gradeColor);
    QString gradeText = QString("Overall Quality Grade: %1/10.0")
                       .arg(report.metrics.qualityGrade * 10.0f, 0, 'f', 1);
    QRect gradeRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 20);
    painter.drawText(gradeRect, Qt::AlignLeft, gradeText);
    m_currentY += 25;

    painter.setPen(m_colors.text);
    QString summaryText = QString("RMS Error: %1 units | Average Point Density: %2 pts/m² | Overlap: %3%")
                         .arg(report.metrics.rmsError, 0, 'f', 4)
                         .arg(report.metrics.averagePointDensity, 0, 'f', 2)
                         .arg(report.metrics.overlapPercentage, 0, 'f', 1);

    QRect summaryRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 20);
    painter.drawText(summaryRect, Qt::AlignLeft, summaryText);
    m_currentY += 25;

}

void PDFReportGenerator::drawMetricsTable(QPainter& painter, const QualityReport& report)
{
    QFont headerFont("Arial", m_fonts.heading, QFont::Bold);
    QFont bodyFont("Arial", m_fonts.body);
    QFont captionFont("Arial", m_fonts.caption);

    painter.setFont(headerFont);
    painter.setPen(m_colors.primary);

    QRect headerRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 25);
    painter.drawText(headerRect, Qt::AlignLeft, "Detailed Metrics");
    m_currentY += 35;

    // Table setup
    int tableWidth = m_layout.contentWidth();
    int colWidth = tableWidth / 2;
    int rowHeight = 25;
    int startX = m_layout.leftMargin;
    int startY = m_currentY;

    // Draw table border
    painter.setPen(QPen(m_colors.secondary, 1));
    painter.drawRect(startX, startY, tableWidth, rowHeight * 6);

    // Draw table headers
    painter.setFont(QFont("Arial", m_fonts.body, QFont::Bold));
    painter.setPen(m_colors.text);
    painter.fillRect(startX, startY, tableWidth, rowHeight, m_colors.lightGrey);

    QRect headerCol1(startX + 5, startY + 5, colWidth - 10, rowHeight - 10);
    QRect headerCol2(startX + colWidth + 5, startY + 5, colWidth - 10, rowHeight - 10);
    painter.drawText(headerCol1, Qt::AlignLeft | Qt::AlignVCenter, "Metric");
    painter.drawText(headerCol2, Qt::AlignLeft | Qt::AlignVCenter, "Value");

    // Draw vertical separator
    painter.drawLine(startX + colWidth, startY, startX + colWidth, startY + rowHeight * 6);

    // Table data
    painter.setFont(bodyFont);
    QStringList metrics = {
        "RMS Error", QString("%1 units").arg(report.metrics.rmsError, 0, 'f', 4),
        "Mean Error", QString("%1 units").arg(report.metrics.meanError, 0, 'f', 4),
        "Standard Deviation", QString("%1 units").arg(report.metrics.standardDeviation, 0, 'f', 4),
        "Point Density", QString("%1 pts/m²").arg(report.metrics.averagePointDensity, 0, 'f', 2),
        "Overlap Percentage", QString("%1%").arg(report.metrics.overlapPercentage, 0, 'f', 1)
    };

    for (int i = 0; i < 5; ++i) {
        int rowY = startY + (i + 1) * rowHeight;

        // Draw horizontal separator
        painter.drawLine(startX, rowY, startX + tableWidth, rowY);

        QRect metricRect(startX + 5, rowY + 5, colWidth - 10, rowHeight - 10);
        QRect valueRect(startX + colWidth + 5, rowY + 5, colWidth - 10, rowHeight - 10);

        painter.drawText(metricRect, Qt::AlignLeft | Qt::AlignVCenter, metrics[i * 2]);
        painter.drawText(valueRect, Qt::AlignLeft | Qt::AlignVCenter, metrics[i * 2 + 1]);
    }

    m_currentY += rowHeight * 6 + 10;

}

void PDFReportGenerator::drawChartsSection(QPainter& painter, const QualityReport& report)
{
    QFont headerFont("Arial", m_fonts.heading, QFont::Bold);

    painter.setFont(headerFont);
    painter.setPen(m_colors.primary);

    QRect headerRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 25);
    painter.drawText(headerRect, Qt::AlignLeft, "Quality Visualization");
    m_currentY += 35;

    // Placeholder for charts - would implement actual chart generation here
    painter.setFont(QFont("Arial", m_fonts.body));
    painter.setPen(m_colors.text);

    QRect chartRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 100);
    painter.drawRect(chartRect);
    painter.drawText(chartRect, Qt::AlignCenter, "Charts would be rendered here\n(Error Distribution, Quality Gauge, Overlap Analysis)");

    m_currentY += 110;
}

void PDFReportGenerator::drawRecommendationsSection(QPainter& painter, const QualityReport& report)
{
    QFont headerFont("Arial", m_fonts.heading, QFont::Bold);
    QFont bodyFont("Arial", m_fonts.body);

    painter.setFont(headerFont);
    painter.setPen(m_colors.primary);

    QRect headerRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 25);
    painter.drawText(headerRect, Qt::AlignLeft, "Recommendations");
    m_currentY += 35;

    painter.setFont(bodyFont);
    painter.setPen(m_colors.text);

    QStringList recommendations;
    float qualityScore = report.metrics.qualityGrade * 10.0f;

    if (qualityScore >= 8.0f) {
        recommendations << "• Excellent quality registration achieved"
                       << "• Consider this as a reference for future registrations"
                       << "• Quality metrics are within optimal ranges";
    } else if (qualityScore >= 6.0f) {
        recommendations << "• Good quality registration with room for improvement"
                       << "• Consider increasing point cloud density in sparse areas"
                       << "• Review overlap areas for better coverage";
    } else {
        recommendations << "• Registration quality needs improvement"
                       << "• Review scan positions and overlap areas"
                       << "• Consider additional scans for better coverage"
                       << "• Check for systematic errors in the registration process";
    }

    // Add custom recommendations from the report
    for (const QString& rec : report.recommendations) {
        recommendations << QString("• %1").arg(rec);
    }

    for (const QString& rec : recommendations) {
        QRect recRect(m_layout.leftMargin, m_currentY, m_layout.contentWidth(), 20);
        painter.drawText(recRect, Qt::AlignLeft, rec);
        m_currentY += 22;
    }
}

void PDFReportGenerator::drawFooter(QPainter& painter, const ReportOptions& options)
{
    QFont footerFont("Arial", m_fonts.caption);
    painter.setFont(footerFont);
    painter.setPen(m_colors.secondary);

    QString footerText = QString("Report generated by %1 Quality Assessment Module")
                        .arg(options.companyName);

    QRect footerRect(m_layout.leftMargin, m_layout.pageHeight - m_layout.bottomMargin - 20,
                    m_layout.contentWidth(), 20);
    painter.drawText(footerRect, Qt::AlignCenter, footerText);
}

// Chart generation methods (placeholder implementations)
QPixmap PDFReportGenerator::generateErrorDistributionChart(const QualityMetrics& metrics)
{
    // Placeholder - would implement actual chart generation
    QPixmap chart(400, 300);
    chart.fill(Qt::white);
    QPainter painter(&chart);
    painter.drawText(chart.rect(), Qt::AlignCenter, "Error Distribution Chart");
    return chart;
}

QPixmap PDFReportGenerator::generateQualityGaugeChart(const QualityMetrics& metrics)
{
    // Placeholder - would implement actual gauge chart
    QPixmap chart(300, 300);
    chart.fill(Qt::white);
    QPainter painter(&chart);
    painter.drawText(chart.rect(), Qt::AlignCenter, "Quality Gauge Chart");
    return chart;
}

QPixmap PDFReportGenerator::generateOverlapChart(const QualityMetrics& metrics)
{
    // Placeholder - would implement actual overlap visualization
    QPixmap chart(400, 300);
    chart.fill(Qt::white);
    QPainter painter(&chart);
    painter.drawText(chart.rect(), Qt::AlignCenter, "Overlap Analysis Chart");
    return chart;
}

// Configuration methods
void PDFReportGenerator::setPageMargins(int left, int top, int right, int bottom)
{
    m_layout.leftMargin = left;
    m_layout.topMargin = top;
    m_layout.rightMargin = right;
    m_layout.bottomMargin = bottom;
}

void PDFReportGenerator::setFontSizes(int title, int heading, int body, int caption)
{
    m_fonts.title = title;
    m_fonts.heading = heading;
    m_fonts.body = body;
    m_fonts.caption = caption;
}

void PDFReportGenerator::setColors(const QColor& primary, const QColor& secondary, const QColor& accent)
{
    m_colors.primary = primary;
    m_colors.secondary = secondary;
    m_colors.accent = accent;
}

// Layout helper methods
int PDFReportGenerator::drawText(QPainter& painter, const QString& text, const QRect& rect, int flags)
{
    painter.drawText(rect, flags, text);
    return painter.fontMetrics().boundingRect(rect, flags, text).height();
}

void PDFReportGenerator::drawLine(QPainter& painter, int x1, int y1, int x2, int y2)
{
    painter.drawLine(x1, y1, x2, y2);
}

void PDFReportGenerator::drawBox(QPainter& painter, const QRect& rect, const QColor& fillColor)
{
    if (fillColor.isValid()) {
        painter.fillRect(rect, fillColor);
    }
    painter.drawRect(rect);
}

// Chart helper methods (placeholder implementations)
void PDFReportGenerator::drawBarChart(QPainter& painter, const QRect& rect,
                                     const QStringList& labels, const QList<float>& values,
                                     const QString& title)
{
    // Placeholder implementation
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter, QString("Bar Chart: %1").arg(title));
}

void PDFReportGenerator::drawGaugeChart(QPainter& painter, const QRect& rect,
                                       float value, float maxValue, const QString& title)
{
    // Placeholder implementation
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter,
                    QString("Gauge Chart: %1\nValue: %2/%3").arg(title).arg(value).arg(maxValue));
}

void PDFReportGenerator::drawPieChart(QPainter& painter, const QRect& rect,
                                     const QStringList& labels, const QList<float>& values,
                                     const QString& title)
{
    // Placeholder implementation
    painter.drawRect(rect);
    painter.drawText(rect, Qt::AlignCenter, QString("Pie Chart: %1").arg(title));
}

// Page management
void PDFReportGenerator::newPage(QPainter& painter, QPagedPaintDevice* device)
{
    device->newPage();
    m_currentY = m_layout.topMargin;
    m_pageNumber++;
}
