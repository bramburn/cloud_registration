#ifndef PDFREPORTGENERATOR_H
#define PDFREPORTGENERATOR_H

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPagedPaintDevice>
#include <QPainter>
#include <QPixmap>
#include <QString>

#include "QualityAssessment.h"

/**
 * @brief PDF Report Generator for Quality Assessment
 *
 * Sprint 6 User Story 2: Registration Quality Assessment and Reporting
 * Generates professional PDF reports with quality metrics, charts, and recommendations.
 */
class PDFReportGenerator : public QObject
{
    Q_OBJECT

public:
    struct ReportOptions
    {
        bool includeCharts = true;
        bool includeScreenshots = true;
        bool includeRecommendations = true;
        bool includeDetailedMetrics = true;
        QString logoPath;
        QString companyName = "CloudRegistration";
        QString reportTitle = "Point Cloud Registration Quality Report";
    };

    explicit PDFReportGenerator(QObject* parent = nullptr);
    ~PDFReportGenerator() override;

    // Main report generation
    bool generatePdfReport(const QualityReport& report,
                           const QString& outputPath,
                           const ReportOptions& options = ReportOptions());

    // Individual section generators
    void drawHeader(QPainter& painter, const QualityReport& report, const ReportOptions& options);
    void drawSummarySection(QPainter& painter, const QualityReport& report);
    void drawMetricsTable(QPainter& painter, const QualityReport& report);
    void drawChartsSection(QPainter& painter, const QualityReport& report);
    void drawRecommendationsSection(QPainter& painter, const QualityReport& report);
    void drawFooter(QPainter& painter, const ReportOptions& options);

    // Chart generation
    QPixmap generateErrorDistributionChart(const QualityMetrics& metrics);
    QPixmap generateQualityGaugeChart(const QualityMetrics& metrics);
    QPixmap generateOverlapChart(const QualityMetrics& metrics);

    // Configuration
    void setPageMargins(int left, int top, int right, int bottom);
    void setFontSizes(int title, int heading, int body, int caption);
    void setColors(const QColor& primary, const QColor& secondary, const QColor& accent);

signals:
    void reportProgress(int percentage, const QString& stage);
    void reportGenerated(const QString& filePath);
    void reportError(const QString& error);

private:
    // Layout helpers
    int
    drawText(QPainter& painter, const QString& text, const QRect& rect, int flags = Qt::AlignLeft | Qt::TextWordWrap);
    void drawLine(QPainter& painter, int x1, int y1, int x2, int y2);
    void drawBox(QPainter& painter, const QRect& rect, const QColor& fillColor = QColor());

    // Chart helpers
    void drawBarChart(QPainter& painter,
                      const QRect& rect,
                      const QStringList& labels,
                      const QList<float>& values,
                      const QString& title);
    void drawGaugeChart(QPainter& painter, const QRect& rect, float value, float maxValue, const QString& title);
    void drawPieChart(QPainter& painter,
                      const QRect& rect,
                      const QStringList& labels,
                      const QList<float>& values,
                      const QString& title);

    // Page management
    void newPage(QPainter& painter, QPagedPaintDevice* device);
    int getCurrentY() const
    {
        return m_currentY;
    }
    void setCurrentY(int y)
    {
        m_currentY = y;
    }
    void addVerticalSpace(int space)
    {
        m_currentY += space;
    }

    // Configuration
    struct PageLayout
    {
        int leftMargin = 50;
        int topMargin = 50;
        int rightMargin = 50;
        int bottomMargin = 50;
        int pageWidth = 595;   // A4 width in points
        int pageHeight = 842;  // A4 height in points
        int contentWidth() const
        {
            return pageWidth - leftMargin - rightMargin;
        }
        int contentHeight() const
        {
            return pageHeight - topMargin - bottomMargin;
        }
    };

    struct FontSizes
    {
        int title = 18;
        int heading = 14;
        int subheading = 12;
        int body = 10;
        int caption = 8;
    };

    struct ColorScheme
    {
        QColor primary = QColor(33, 150, 243);     // Blue
        QColor secondary = QColor(96, 125, 139);   // Blue Grey
        QColor accent = QColor(255, 193, 7);       // Amber
        QColor success = QColor(76, 175, 80);      // Green
        QColor warning = QColor(255, 152, 0);      // Orange
        QColor error = QColor(244, 67, 54);        // Red
        QColor text = QColor(33, 33, 33);          // Dark Grey
        QColor lightGrey = QColor(245, 245, 245);  // Light Grey
    };

    PageLayout m_layout;
    FontSizes m_fonts;
    ColorScheme m_colors;
    int m_currentY = 0;
    int m_pageNumber = 1;
};

#endif  // PDFREPORTGENERATOR_H
