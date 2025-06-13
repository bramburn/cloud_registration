#include "quality/PDFReportGenerator.h"

#include <QDate>
#include <QDir>
#include <QFont>
#include <QPainter>
#include <QPrinter>
#include <QStandardPaths>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QTextTableFormat>

PDFReportGenerator::PDFReportGenerator(QObject* parent) : QObject(parent) {}

PDFReportGenerator::~PDFReportGenerator() = default;

bool PDFReportGenerator::generateReport(const QString& outputPath, const QualityMetrics& metrics)
{
    try
    {
        QPrinter printer(QPrinter::PrinterResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(outputPath);
        printer.setPageSize(QPageSize::A4);
        printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

        QTextDocument document;
        setupDocument(&document, metrics);

        document.print(&printer);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void PDFReportGenerator::setupDocument(QTextDocument* document, const QualityMetrics& metrics)
{
    QTextCursor cursor(document);

    // Set up fonts
    QTextCharFormat titleFormat;
    titleFormat.setFont(QFont("Arial", 18, QFont::Bold));
    titleFormat.setForeground(QColor(0, 0, 0));

    QTextCharFormat headerFormat;
    headerFormat.setFont(QFont("Arial", 14, QFont::Bold));
    headerFormat.setForeground(QColor(0, 0, 0));

    QTextCharFormat normalFormat;
    normalFormat.setFont(QFont("Arial", 10));
    normalFormat.setForeground(QColor(0, 0, 0));

    // Title
    cursor.insertText("CloudRegistration Quality Assessment Report\n\n", titleFormat);

    // Report metadata
    cursor.insertText("Generated: " + QDate::currentDate().toString("yyyy-MM-dd") + "\n", normalFormat);
    cursor.insertText("Project: " + metrics.projectName + "\n\n", normalFormat);

    // Summary section
    cursor.insertText("Executive Summary\n", headerFormat);
    cursor.insertText("=================\n\n", headerFormat);

    cursor.insertText("Overall Quality Score: " + QString::number(metrics.overallScore, 'f', 2) + "/10.0\n",
                      normalFormat);
    cursor.insertText("Registration Accuracy: " + QString::number(metrics.registrationAccuracy, 'f', 4) + " units\n",
                      normalFormat);
    cursor.insertText("Point Cloud Density: " + QString::number(metrics.pointCloudDensity, 'f', 2) + " points/m²\n",
                      normalFormat);
    cursor.insertText("Coverage Percentage: " + QString::number(metrics.coveragePercentage, 'f', 1) + "%\n\n",
                      normalFormat);

    // Detailed metrics
    cursor.insertText("Detailed Metrics\n", headerFormat);
    cursor.insertText("================\n\n", headerFormat);

    // Create a table for detailed metrics
    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setBorder(1);
    tableFormat.setCellPadding(5);
    tableFormat.setCellSpacing(0);

    QTextTable* table = cursor.insertTable(5, 2, tableFormat);

    // Table headers
    QTextTableCell cell = table->cellAt(0, 0);
    QTextCursor cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Metric", headerFormat);

    cell = table->cellAt(0, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Value", headerFormat);

    // Table data
    cell = table->cellAt(1, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Registration Accuracy", normalFormat);
    cell = table->cellAt(1, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(QString::number(metrics.registrationAccuracy, 'f', 4) + " units", normalFormat);

    cell = table->cellAt(2, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Point Cloud Density", normalFormat);
    cell = table->cellAt(2, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(QString::number(metrics.pointCloudDensity, 'f', 2) + " points/m²", normalFormat);

    cell = table->cellAt(3, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Coverage Percentage", normalFormat);
    cell = table->cellAt(3, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(QString::number(metrics.coveragePercentage, 'f', 1) + "%", normalFormat);

    cell = table->cellAt(4, 0);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText("Overall Score", normalFormat);
    cell = table->cellAt(4, 1);
    cellCursor = cell.firstCursorPosition();
    cellCursor.insertText(QString::number(metrics.overallScore, 'f', 2) + "/10.0", normalFormat);

    // Move cursor after table
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n\n", normalFormat);

    // Recommendations section
    cursor.insertText("Recommendations\n", headerFormat);
    cursor.insertText("===============\n\n", headerFormat);

    if (metrics.overallScore >= 8.0)
    {
        cursor.insertText("• Excellent quality registration achieved\n", normalFormat);
        cursor.insertText("• Consider this as a reference for future registrations\n", normalFormat);
    }
    else if (metrics.overallScore >= 6.0)
    {
        cursor.insertText("• Good quality registration with room for improvement\n", normalFormat);
        cursor.insertText("• Consider increasing point cloud density in sparse areas\n", normalFormat);
    }
    else
    {
        cursor.insertText("• Registration quality needs improvement\n", normalFormat);
        cursor.insertText("• Review scan positions and overlap areas\n", normalFormat);
        cursor.insertText("• Consider additional scans for better coverage\n", normalFormat);
    }

    cursor.insertText("\n\nReport generated by CloudRegistration Quality Assessment Module", normalFormat);
}

QString PDFReportGenerator::getDefaultOutputPath() const
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString reportDir = documentsPath + "/CloudRegistration/Reports";

    QDir dir;
    if (!dir.exists(reportDir))
    {
        dir.mkpath(reportDir);
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return reportDir + "/quality_report_" + timestamp + ".pdf";
}
