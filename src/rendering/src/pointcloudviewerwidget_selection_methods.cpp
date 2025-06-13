// Sprint 1.1: Additional selection mode methods for PointCloudViewerWidget
// This file contains the implementation of selection mode methods that need to be added to pointcloudviewerwidget.cpp

void PointCloudViewerWidget::handleSelectionModeMousePress(QMouseEvent* event)
{
    if (!m_pointSelector || !m_hasData || m_selectionMode == SelectionMode::None || m_selectionMode == SelectionMode::Navigation)
    {
        return;
    }
    
    if (event->button() == Qt::LeftButton)
    {
        // Convert point data to PointFullData format for the selector
        std::vector<PointFullData> pointData;
        pointData.reserve(m_pointCount);
        
        for (int i = 0; i < m_pointCount; ++i)
        {
            int baseIndex = i * 6; // Assuming XYZ RGB format
            if (baseIndex + 5 < static_cast<int>(m_pointData.size()))
            {
                PointFullData point;
                point.x = m_pointData[baseIndex];
                point.y = m_pointData[baseIndex + 1];
                point.z = m_pointData[baseIndex + 2];
                point.r = static_cast<uint8_t>(m_pointData[baseIndex + 3] * 255);
                point.g = static_cast<uint8_t>(m_pointData[baseIndex + 4] * 255);
                point.b = static_cast<uint8_t>(m_pointData[baseIndex + 5] * 255);
                point.intensity = 1.0f; // Default intensity
                pointData.push_back(point);
            }
        }
        
        // Perform point selection
        auto result = m_pointSelector->selectPoint(
            pointData,
            m_viewMatrix,
            m_projectionMatrix,
            event->pos(),
            size(),
            5.0f // Selection radius in pixels
        );
        
        if (result.isValid())
        {
            emit pointSelected(result.selectedPoint, result.pointIndex);
            qDebug() << "Point selected at" << result.selectedPoint;
        }
        else
        {
            emit selectionFailed("No point found near click position");
        }
    }
}

void PointCloudViewerWidget::renderCrosshairs()
{
    if (!m_showCrosshairs)
    {
        return;
    }
    
    // Use QPainter for overlay rendering
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Set crosshair style
    QPen crosshairPen(Qt::red, 2);
    crosshairPen.setStyle(Qt::DashLine);
    painter.setPen(crosshairPen);
    
    // Draw crosshairs at center of widget or at mouse position
    QPoint center = m_crosshairPosition.isNull() ? rect().center() : m_crosshairPosition;
    int crosshairSize = 20;
    
    // Horizontal line
    painter.drawLine(center.x() - crosshairSize, center.y(), 
                     center.x() + crosshairSize, center.y());
    
    // Vertical line
    painter.drawLine(center.x(), center.y() - crosshairSize,
                     center.x(), center.y() + crosshairSize);
    
    // Draw selection mode indicator
    painter.setPen(QPen(Qt::white, 1));
    painter.setFont(m_detailFont);
    
    QString modeText;
    switch (m_selectionMode)
    {
        case SelectionMode::ManualAlignment:
            modeText = "Manual Alignment Mode - Click to select points";
            break;
        case SelectionMode::Measurement:
            modeText = "Measurement Mode";
            break;
        case SelectionMode::Annotation:
            modeText = "Annotation Mode";
            break;
        default:
            break;
    }
    
    if (!modeText.isEmpty())
    {
        QRect textRect = painter.fontMetrics().boundingRect(modeText);
        textRect.moveTopLeft(QPoint(10, 10));
        
        // Draw background
        painter.fillRect(textRect.adjusted(-5, -2, 5, 2), QColor(0, 0, 0, 128));
        
        // Draw text
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignTop, modeText);
    }
}
