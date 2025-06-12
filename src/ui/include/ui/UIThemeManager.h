#ifndef UITHEMEMANAGER_H
#define UITHEMEMANAGER_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QString>
#include <QMap>
#include <QVariant>

/**
 * @brief Professional UI theme management system
 * 
 * This class manages the application's visual theme and ensures consistent
 * look and feel across all UI components. It provides:
 * - Professional color palette management
 * - Typography standards and font management
 * - Qt Style Sheet (QSS) generation
 * - High-DPI display support
 * - Theme customization and persistence
 * 
 * Sprint 7 Requirements:
 * - Consistent professional design language
 * - Global styling through QSS
 * - High-DPI scaling support
 * - User customization capabilities
 */
class UIThemeManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Predefined theme types
     */
    enum class ThemeType {
        Light,
        Dark,
        HighContrast,
        Custom
    };

    /**
     * @brief Color roles for consistent theming
     */
    enum class ColorRole {
        Primary,
        Secondary,
        Accent,
        Background,
        Surface,
        Text,
        TextSecondary,
        Border,
        Hover,
        Pressed,
        Disabled,
        Success,
        Warning,
        Error,
        Info
    };

    /**
     * @brief Typography scale definitions
     */
    enum class TypographyScale {
        Headline1,      // 32pt - Main headings
        Headline2,      // 24pt - Section headings
        Headline3,      // 20pt - Subsection headings
        Subtitle1,      // 16pt - Large subtitles
        Subtitle2,      // 14pt - Medium subtitles
        Body1,          // 12pt - Primary body text
        Body2,          // 11pt - Secondary body text
        Caption,        // 10pt - Captions and labels
        Button,         // 12pt - Button text
        Overline        // 10pt - Overline text
    };

    /**
     * @brief Theme configuration structure
     */
    struct ThemeConfig {
        ThemeType type = ThemeType::Light;
        QMap<ColorRole, QColor> colors;
        QMap<TypographyScale, QFont> fonts;
        QString name = "Default";
        double scaleFactor = 1.0;
        bool enableAnimations = true;
        int borderRadius = 4;
        int shadowBlur = 8;
        QColor shadowColor = QColor(0, 0, 0, 30);
    };

    /**
     * @brief UI component style definitions
     */
    struct ComponentStyles {
        QString button;
        QString lineEdit;
        QString comboBox;
        QString label;
        QString groupBox;
        QString tabWidget;
        QString treeView;
        QString tableView;
        QString scrollBar;
        QString toolBar;
        QString statusBar;
        QString menuBar;
        QString menu;
        QString dialog;
        QString progressBar;
        QString slider;
        QString spinBox;
        QString checkBox;
        QString radioButton;
    };

public:
    static UIThemeManager& instance();
    explicit UIThemeManager(QObject* parent = nullptr);
    ~UIThemeManager();

    // Theme management
    void setTheme(ThemeType type);
    void setCustomTheme(const ThemeConfig& config);
    ThemeType getCurrentTheme() const;
    ThemeConfig getThemeConfig() const;
    
    // Color management
    QColor getColor(ColorRole role) const;
    void setColor(ColorRole role, const QColor& color);
    QString getColorHex(ColorRole role) const;
    
    // Typography management
    QFont getFont(TypographyScale scale) const;
    void setFont(TypographyScale scale, const QFont& font);
    void setBaseFontFamily(const QString& family);
    void setScaleFactor(double factor);
    
    // Style sheet generation
    QString generateGlobalStyleSheet() const;
    QString generateComponentStyleSheet(const QString& component) const;
    ComponentStyles generateAllComponentStyles() const;
    
    // High-DPI support
    void updateForDPI(double dpiScale);
    double getDPIScale() const;
    int scaledSize(int baseSize) const;
    QSize scaledSize(const QSize& baseSize) const;
    
    // Theme persistence
    void saveTheme(const QString& name = QString()) const;
    bool loadTheme(const QString& name);
    QStringList getAvailableThemes() const;
    void deleteTheme(const QString& name);
    
    // Validation and utilities
    bool isValidColor(const QColor& color) const;
    QColor adjustColorBrightness(const QColor& color, double factor) const;
    QColor getContrastingTextColor(const QColor& background) const;
    double getColorContrast(const QColor& color1, const QColor& color2) const;

signals:
    void themeChanged(ThemeType newTheme);
    void colorChanged(ColorRole role, const QColor& newColor);
    void fontChanged(TypographyScale scale, const QFont& newFont);
    void dpiScaleChanged(double newScale);

private slots:
    void onSystemThemeChanged();

private:
    ThemeConfig m_currentConfig;
    double m_dpiScale;
    QString m_settingsKey;
    
    // Predefined themes
    QMap<ThemeType, ThemeConfig> m_predefinedThemes;
    
    void initializePredefinedThemes();
    void initializeLightTheme();
    void initializeDarkTheme();
    void initializeHighContrastTheme();
    
    void initializeTypography();
    void updateFontsForScale();
    
    QString generateButtonStyle() const;
    QString generateLineEditStyle() const;
    QString generateComboBoxStyle() const;
    QString generateLabelStyle() const;
    QString generateGroupBoxStyle() const;
    QString generateTabWidgetStyle() const;
    QString generateTreeViewStyle() const;
    QString generateTableViewStyle() const;
    QString generateScrollBarStyle() const;
    QString generateToolBarStyle() const;
    QString generateStatusBarStyle() const;
    QString generateMenuBarStyle() const;
    QString generateMenuStyle() const;
    QString generateDialogStyle() const;
    QString generateProgressBarStyle() const;
    QString generateSliderStyle() const;
    QString generateSpinBoxStyle() const;
    QString generateCheckBoxStyle() const;
    QString generateRadioButtonStyle() const;
    
    QString colorToString(const QColor& color) const;
    QString fontToString(const QFont& font) const;
    QFont scaledFont(const QFont& baseFont) const;
    
    void applyThemeToApplication();
    void saveThemeToSettings(const ThemeConfig& config, const QString& name) const;
    ThemeConfig loadThemeFromSettings(const QString& name) const;
    
    // Color palette helpers
    QColor lighten(const QColor& color, double factor = 0.2) const;
    QColor darken(const QColor& color, double factor = 0.2) const;
    QColor withAlpha(const QColor& color, int alpha) const;
    
    // Validation helpers
    bool isAccessibilityCompliant(const QColor& foreground, const QColor& background) const;
    double calculateLuminance(const QColor& color) const;
};

#endif // UITHEMEMANAGER_H
