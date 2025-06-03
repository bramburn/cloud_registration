#include "iconmanager.h"
#include <QApplication>
#include <QPalette>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

const QSize IconManager::DEFAULT_ICON_SIZE(16, 16);
const QSize IconManager::OVERLAY_SIZE(8, 8);
const QSize IconManager::BADGE_SIZE(6, 6);

IconManager& IconManager::instance()
{
    static IconManager instance;
    return instance;
}

IconManager::IconManager()
    : m_currentTheme("default")
    , m_isDarkTheme(false)
    , m_iconBasePath(":/icons")
{
    // Detect theme
    QPalette palette = QApplication::palette();
    m_isDarkTheme = palette.color(QPalette::Window).lightness() < 128;
    
    loadIcons();
}

void IconManager::loadIcons()
{
    loadBaseIcons();
    loadOverlayIcons();
    loadBadgeIcons();
    loadThemeSpecificIcons();
}

void IconManager::loadBaseIcons()
{
    // Base item type icons
    m_iconCache["scan_base"] = QIcon(getIconPath("scan.svg"));
    m_iconCache["cluster_base"] = QIcon(getIconPath("cluster.svg"));
    m_iconCache["project_base"] = QIcon(getIconPath("project.svg"));
    
    // Fallback to built-in Qt icons if custom icons not found
    if (m_iconCache["scan_base"].isNull()) {
        m_iconCache["scan_base"] = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }
    if (m_iconCache["cluster_base"].isNull()) {
        m_iconCache["cluster_base"] = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    }
    if (m_iconCache["project_base"].isNull()) {
        m_iconCache["project_base"] = QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon);
    }
}

void IconManager::loadOverlayIcons()
{
    // State overlay icons
    m_iconCache["loaded_overlay"] = QIcon(getIconPath("overlays/loaded.svg"));
    m_iconCache["unloaded_overlay"] = QIcon(getIconPath("overlays/unloaded.svg"));
    m_iconCache["locked_overlay"] = QIcon(getIconPath("overlays/locked.svg"));
    m_iconCache["missing_overlay"] = QIcon(getIconPath("overlays/missing.svg"));
    m_iconCache["loading_overlay"] = QIcon(getIconPath("overlays/loading.svg"));
    m_iconCache["error_overlay"] = QIcon(getIconPath("overlays/error.svg"));
    
    // Fallback overlays using built-in icons
    if (m_iconCache["loaded_overlay"].isNull()) {
        m_iconCache["loaded_overlay"] = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
    }
    if (m_iconCache["locked_overlay"].isNull()) {
        m_iconCache["locked_overlay"] = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
    }
    if (m_iconCache["missing_overlay"].isNull()) {
        m_iconCache["missing_overlay"] = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    }
    if (m_iconCache["error_overlay"].isNull()) {
        m_iconCache["error_overlay"] = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
    }
}

void IconManager::loadBadgeIcons()
{
    // Import type badges
    m_iconCache["copy_badge"] = QIcon(getIconPath("badges/copy.svg"));
    m_iconCache["move_badge"] = QIcon(getIconPath("badges/move.svg"));
    m_iconCache["link_badge"] = QIcon(getIconPath("badges/link.svg"));
    
    // Fallback badges using built-in icons
    if (m_iconCache["copy_badge"].isNull()) {
        m_iconCache["copy_badge"] = QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    }
    if (m_iconCache["move_badge"].isNull()) {
        m_iconCache["move_badge"] = QApplication::style()->standardIcon(QStyle::SP_ArrowRight);
    }
    if (m_iconCache["link_badge"].isNull()) {
        m_iconCache["link_badge"] = QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon);
    }
}

void IconManager::loadThemeSpecificIcons()
{
    QString themePrefix = m_isDarkTheme ? "dark/" : "light/";
    
    // Load theme-specific variants if available
    QString themedScanPath = getIconPath(themePrefix + "scan.svg");
    if (QFile::exists(themedScanPath)) {
        m_iconCache["scan_base"] = QIcon(themedScanPath);
    }
    
    QString themedClusterPath = getIconPath(themePrefix + "cluster.svg");
    if (QFile::exists(themedClusterPath)) {
        m_iconCache["cluster_base"] = QIcon(themedClusterPath);
    }
}

QIcon IconManager::getIcon(ItemType type, ItemState state) const
{
    return getCompositeIcon(type, state, ImportType::None);
}

QIcon IconManager::getCompositeIcon(ItemType type, ItemState state, ImportType importType) const
{
    QString cacheKey = getCacheKey(type, state, importType);
    
    if (m_compositeCache.contains(cacheKey)) {
        return QIcon(m_compositeCache[cacheKey]);
    }
    
    // Get base icon
    QString baseKey;
    switch (type) {
        case ItemType::Scan: baseKey = "scan_base"; break;
        case ItemType::Cluster: baseKey = "cluster_base"; break;
        case ItemType::Project: baseKey = "project_base"; break;
    }
    
    QIcon baseIcon = m_iconCache.value(baseKey);
    QIcon overlayIcon;
    QIcon badgeIcon;
    
    // Get state overlay
    switch (state) {
        case ItemState::Loaded: overlayIcon = m_iconCache.value("loaded_overlay"); break;
        case ItemState::Locked: overlayIcon = m_iconCache.value("locked_overlay"); break;
        case ItemState::Missing: overlayIcon = m_iconCache.value("missing_overlay"); break;
        case ItemState::Loading: overlayIcon = m_iconCache.value("loading_overlay"); break;
        case ItemState::Error: overlayIcon = m_iconCache.value("error_overlay"); break;
        case ItemState::Unloaded:
        case ItemState::Unlocked:
        default: 
            // No overlay for these states
            break;
    }
    
    // Get import type badge
    if (importType != ImportType::None) {
        switch (importType) {
            case ImportType::Copy: badgeIcon = m_iconCache.value("copy_badge"); break;
            case ImportType::Move: badgeIcon = m_iconCache.value("move_badge"); break;
            case ImportType::Link: badgeIcon = m_iconCache.value("link_badge"); break;
            default: break;
        }
    }
    
    QPixmap composite = createCompositePixmap(baseIcon, overlayIcon, badgeIcon);
    m_compositeCache[cacheKey] = composite;
    
    return QIcon(composite);
}

QPixmap IconManager::createCompositePixmap(const QIcon& baseIcon, const QIcon& overlayIcon, 
                                          const QIcon& badgeIcon, const QSize& size) const
{
    QPixmap pixmap = baseIcon.pixmap(size);
    
    if (pixmap.isNull()) {
        // Create a default pixmap if base icon is null
        pixmap = QPixmap(size);
        pixmap.fill(Qt::transparent);
    }
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw state overlay (top-right corner)
    if (!overlayIcon.isNull()) {
        QPixmap overlay = overlayIcon.pixmap(OVERLAY_SIZE);
        int x = size.width() - OVERLAY_SIZE.width();
        int y = 0;
        painter.drawPixmap(x, y, overlay);
    }
    
    // Draw import type badge (bottom-right corner)
    if (!badgeIcon.isNull()) {
        QPixmap badge = badgeIcon.pixmap(BADGE_SIZE);
        int x = size.width() - BADGE_SIZE.width();
        int y = size.height() - BADGE_SIZE.height();
        painter.drawPixmap(x, y, badge);
    }
    
    return pixmap;
}

QString IconManager::getIconPath(const QString& iconName) const
{
    return QString("%1/%2").arg(m_iconBasePath, iconName);
}

QString IconManager::getCacheKey(ItemType type, ItemState state, ImportType importType, const QSize& size) const
{
    return QString("%1_%2_%3_%4x%5")
        .arg(static_cast<int>(type))
        .arg(static_cast<int>(state))
        .arg(static_cast<int>(importType))
        .arg(size.width())
        .arg(size.height());
}

QIcon IconManager::getStateOverlayIcon(ItemState state) const
{
    switch (state) {
        case ItemState::Loaded: return m_iconCache.value("loaded_overlay");
        case ItemState::Locked: return m_iconCache.value("locked_overlay");
        case ItemState::Missing: return m_iconCache.value("missing_overlay");
        case ItemState::Loading: return m_iconCache.value("loading_overlay");
        case ItemState::Error: return m_iconCache.value("error_overlay");
        default: return QIcon();
    }
}

QIcon IconManager::getImportBadgeIcon(ImportType importType) const
{
    switch (importType) {
        case ImportType::Copy: return m_iconCache.value("copy_badge");
        case ImportType::Move: return m_iconCache.value("move_badge");
        case ImportType::Link: return m_iconCache.value("link_badge");
        default: return QIcon();
    }
}

void IconManager::setTheme(const QString& themeName)
{
    if (m_currentTheme != themeName) {
        m_currentTheme = themeName;
        clearCache();
        loadIcons();
    }
}

void IconManager::clearCache()
{
    m_iconCache.clear();
    m_compositeCache.clear();
}

void IconManager::preloadIcons()
{
    // Pre-generate common icon combinations
    QList<ItemType> types = {ItemType::Scan, ItemType::Cluster, ItemType::Project};
    QList<ItemState> states = {ItemState::Loaded, ItemState::Unloaded, ItemState::Locked, ItemState::Missing};
    QList<ImportType> importTypes = {ImportType::None, ImportType::Copy, ImportType::Move, ImportType::Link};
    
    for (ItemType type : types) {
        for (ItemState state : states) {
            for (ImportType importType : importTypes) {
                // Skip invalid combinations
                if (type == ItemType::Cluster && importType != ImportType::None) {
                    continue;
                }
                getCompositeIcon(type, state, importType);
            }
        }
    }
}

QIcon IconManager::getScaledIcon(ItemType type, ItemState state, const QSize& size) const
{
    QString cacheKey = getCacheKey(type, state, ImportType::None, size);
    
    if (m_compositeCache.contains(cacheKey)) {
        return QIcon(m_compositeCache[cacheKey]);
    }
    
    // Create scaled version
    QIcon baseIcon = getIcon(type, state);
    QPixmap scaledPixmap = baseIcon.pixmap(size);
    m_compositeCache[cacheKey] = scaledPixmap;
    
    return QIcon(scaledPixmap);
}
