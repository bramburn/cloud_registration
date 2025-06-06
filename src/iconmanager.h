#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QIcon>
#include <QHash>
#include <QPixmap>
#include <QPainter>
#include <QSize>

enum class ItemType {
    Scan,
    Cluster,
    Project
};

enum class ItemState {
    Loaded,
    Unloaded,
    Locked,
    Unlocked,
    Missing,
    Loading,
    Error,
    Processing,     // Sprint 2.1: Being processed
    Cached,         // Sprint 2.1: In LRU cache
    MemoryWarning,  // Sprint 2.1: Approaching memory limits
    Optimized       // Sprint 2.1: Processed and ready
};

enum class ImportType {
    Copy,
    Move,
    Link,
    None  // For items that don't have import type (like clusters)
};

/**
 * @brief Centralized icon management system for Sprint 3.3
 * 
 * Provides consistent icons across the application with support for:
 * - Base item type icons (scan, cluster, project)
 * - State overlays (loaded, locked, missing, etc.)
 * - Import type badges (copy, move, link)
 * - High DPI scaling
 * - Theme support
 */
class IconManager
{
public:
    static IconManager& instance();
    
    // Main icon retrieval methods
    QIcon getIcon(ItemType type, ItemState state = ItemState::Unloaded) const;
    QIcon getCompositeIcon(ItemType type, ItemState state, ImportType importType) const;
    QIcon getStateOverlayIcon(ItemState state) const;
    QIcon getImportBadgeIcon(ImportType importType) const;
    
    // High DPI support
    QIcon getScaledIcon(ItemType type, ItemState state, const QSize& size) const;
    
    // Theme support
    void setTheme(const QString& themeName);
    QString currentTheme() const { return m_currentTheme; }
    
    // Cache management
    void clearCache();
    void preloadIcons();
    
private:
    IconManager();
    ~IconManager() = default;
    IconManager(const IconManager&) = delete;
    IconManager& operator=(const IconManager&) = delete;
    
    void loadIcons();
    void loadThemeSpecificIcons();
    void loadBaseIcons();
    void loadOverlayIcons();
    void loadBadgeIcons();
    
    QPixmap createCompositePixmap(const QIcon& baseIcon, const QIcon& overlayIcon, 
                                  const QIcon& badgeIcon = QIcon(), const QSize& size = QSize(16, 16)) const;
    
    QString getIconPath(const QString& iconName) const;
    QString getCacheKey(ItemType type, ItemState state, ImportType importType, const QSize& size = QSize(16, 16)) const;
    
    // Icon storage
    QHash<QString, QIcon> m_iconCache;
    mutable QHash<QString, QPixmap> m_compositeCache;
    
    // Theme management
    QString m_currentTheme;
    bool m_isDarkTheme;
    
    // Icon paths
    QString m_iconBasePath;
    
    // Default sizes
    static const QSize DEFAULT_ICON_SIZE;
    static const QSize OVERLAY_SIZE;
    static const QSize BADGE_SIZE;
};

#endif // ICONMANAGER_H
