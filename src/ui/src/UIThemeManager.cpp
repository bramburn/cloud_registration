#include "UIThemeManager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>

#include <cmath>

UIThemeManager& UIThemeManager::instance()
{
    static UIThemeManager instance;
    return instance;
}

UIThemeManager::UIThemeManager(QObject* parent) : QObject(parent), m_dpiScale(1.0), m_settingsKey("UITheme")
{
    initializePredefinedThemes();

    // Detect DPI scaling
    if (QApplication::primaryScreen())
    {
        m_dpiScale = QApplication::primaryScreen()->devicePixelRatio();
    }

    // Load saved theme or use default
    QSettings settings;
    QString savedTheme = settings.value(m_settingsKey + "/current", "Light").toString();

    if (savedTheme == "Light")
    {
        setTheme(ThemeType::Light);
    }
    else if (savedTheme == "Dark")
    {
        setTheme(ThemeType::Dark);
    }
    else if (savedTheme == "HighContrast")
    {
        setTheme(ThemeType::HighContrast);
    }
    else
    {
        // Try to load custom theme
        if (!loadTheme(savedTheme))
        {
            setTheme(ThemeType::Light);  // Fallback
        }
    }

    qDebug() << "UIThemeManager initialized with DPI scale:" << m_dpiScale;
}

UIThemeManager::~UIThemeManager() = default;

void UIThemeManager::initializePredefinedThemes()
{
    initializeLightTheme();
    initializeDarkTheme();
    initializeHighContrastTheme();
}

void UIThemeManager::initializeLightTheme()
{
    ThemeConfig lightTheme;
    lightTheme.type = ThemeType::Light;
    lightTheme.name = "Light";

    // Light theme color palette
    lightTheme.colors[ColorRole::Primary] = QColor("#2196F3");        // Blue
    lightTheme.colors[ColorRole::Secondary] = QColor("#757575");      // Grey
    lightTheme.colors[ColorRole::Accent] = QColor("#FF5722");         // Deep Orange
    lightTheme.colors[ColorRole::Background] = QColor("#FAFAFA");     // Light Grey
    lightTheme.colors[ColorRole::Surface] = QColor("#FFFFFF");        // White
    lightTheme.colors[ColorRole::Text] = QColor("#212121");           // Dark Grey
    lightTheme.colors[ColorRole::TextSecondary] = QColor("#757575");  // Medium Grey
    lightTheme.colors[ColorRole::Border] = QColor("#E0E0E0");         // Light Border
    lightTheme.colors[ColorRole::Hover] = QColor("#E3F2FD");          // Light Blue
    lightTheme.colors[ColorRole::Pressed] = QColor("#BBDEFB");        // Medium Blue
    lightTheme.colors[ColorRole::Disabled] = QColor("#BDBDBD");       // Disabled Grey
    lightTheme.colors[ColorRole::Success] = QColor("#4CAF50");        // Green
    lightTheme.colors[ColorRole::Warning] = QColor("#FF9800");        // Orange
    lightTheme.colors[ColorRole::Error] = QColor("#F44336");          // Red
    lightTheme.colors[ColorRole::Info] = QColor("#2196F3");           // Blue

    m_predefinedThemes[ThemeType::Light] = lightTheme;
}

void UIThemeManager::initializeDarkTheme()
{
    ThemeConfig darkTheme;
    darkTheme.type = ThemeType::Dark;
    darkTheme.name = "Dark";

    // Dark theme color palette
    darkTheme.colors[ColorRole::Primary] = QColor("#64B5F6");        // Light Blue
    darkTheme.colors[ColorRole::Secondary] = QColor("#BDBDBD");      // Light Grey
    darkTheme.colors[ColorRole::Accent] = QColor("#FF7043");         // Light Deep Orange
    darkTheme.colors[ColorRole::Background] = QColor("#121212");     // Very Dark Grey
    darkTheme.colors[ColorRole::Surface] = QColor("#1E1E1E");        // Dark Grey
    darkTheme.colors[ColorRole::Text] = QColor("#FFFFFF");           // White
    darkTheme.colors[ColorRole::TextSecondary] = QColor("#BDBDBD");  // Light Grey
    darkTheme.colors[ColorRole::Border] = QColor("#424242");         // Medium Dark Grey
    darkTheme.colors[ColorRole::Hover] = QColor("#263238");          // Dark Blue Grey
    darkTheme.colors[ColorRole::Pressed] = QColor("#37474F");        // Medium Blue Grey
    darkTheme.colors[ColorRole::Disabled] = QColor("#616161");       // Dark Disabled Grey
    darkTheme.colors[ColorRole::Success] = QColor("#66BB6A");        // Light Green
    darkTheme.colors[ColorRole::Warning] = QColor("#FFB74D");        // Light Orange
    darkTheme.colors[ColorRole::Error] = QColor("#EF5350");          // Light Red
    darkTheme.colors[ColorRole::Info] = QColor("#64B5F6");           // Light Blue

    m_predefinedThemes[ThemeType::Dark] = darkTheme;
}

void UIThemeManager::initializeHighContrastTheme()
{
    ThemeConfig hcTheme;
    hcTheme.type = ThemeType::HighContrast;
    hcTheme.name = "High Contrast";

    // High contrast color palette
    hcTheme.colors[ColorRole::Primary] = QColor("#0000FF");        // Pure Blue
    hcTheme.colors[ColorRole::Secondary] = QColor("#808080");      // Medium Grey
    hcTheme.colors[ColorRole::Accent] = QColor("#FF0000");         // Pure Red
    hcTheme.colors[ColorRole::Background] = QColor("#FFFFFF");     // Pure White
    hcTheme.colors[ColorRole::Surface] = QColor("#F0F0F0");        // Light Grey
    hcTheme.colors[ColorRole::Text] = QColor("#000000");           // Pure Black
    hcTheme.colors[ColorRole::TextSecondary] = QColor("#404040");  // Dark Grey
    hcTheme.colors[ColorRole::Border] = QColor("#000000");         // Black Border
    hcTheme.colors[ColorRole::Hover] = QColor("#E0E0FF");          // Light Blue
    hcTheme.colors[ColorRole::Pressed] = QColor("#C0C0FF");        // Medium Blue
    hcTheme.colors[ColorRole::Disabled] = QColor("#C0C0C0");       // Light Grey
    hcTheme.colors[ColorRole::Success] = QColor("#008000");        // Pure Green
    hcTheme.colors[ColorRole::Warning] = QColor("#FFA500");        // Pure Orange
    hcTheme.colors[ColorRole::Error] = QColor("#FF0000");          // Pure Red
    hcTheme.colors[ColorRole::Info] = QColor("#0000FF");           // Pure Blue

    m_predefinedThemes[ThemeType::HighContrast] = hcTheme;
}

void UIThemeManager::initializeTypography()
{
    // Base font family - try to use system default
    QString fontFamily = "Segoe UI";
    if (!QFontDatabase::families().contains(fontFamily))
    {
        fontFamily = QApplication::font().family();
    }

    // Initialize typography scale
    m_currentConfig.fonts[TypographyScale::Headline1] = QFont(fontFamily, scaledSize(32), QFont::Bold);
    m_currentConfig.fonts[TypographyScale::Headline2] = QFont(fontFamily, scaledSize(24), QFont::Bold);
    m_currentConfig.fonts[TypographyScale::Headline3] = QFont(fontFamily, scaledSize(20), QFont::DemiBold);
    m_currentConfig.fonts[TypographyScale::Subtitle1] = QFont(fontFamily, scaledSize(16), QFont::DemiBold);
    m_currentConfig.fonts[TypographyScale::Subtitle2] = QFont(fontFamily, scaledSize(14), QFont::DemiBold);
    m_currentConfig.fonts[TypographyScale::Body1] = QFont(fontFamily, scaledSize(12), QFont::Normal);
    m_currentConfig.fonts[TypographyScale::Body2] = QFont(fontFamily, scaledSize(11), QFont::Normal);
    m_currentConfig.fonts[TypographyScale::Caption] = QFont(fontFamily, scaledSize(10), QFont::Normal);
    m_currentConfig.fonts[TypographyScale::Button] = QFont(fontFamily, scaledSize(12), QFont::DemiBold);
    m_currentConfig.fonts[TypographyScale::Overline] = QFont(fontFamily, scaledSize(10), QFont::Normal);

    // Set letter spacing for overline
    m_currentConfig.fonts[TypographyScale::Overline].setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
}

void UIThemeManager::setTheme(ThemeType type)
{
    if (m_predefinedThemes.contains(type))
    {
        m_currentConfig = m_predefinedThemes[type];
        m_currentConfig.scaleFactor = m_dpiScale;

        initializeTypography();
        applyThemeToApplication();

        // Save current theme
        QSettings settings;
        settings.setValue(m_settingsKey + "/current", m_currentConfig.name);

        emit themeChanged(type);
        qDebug() << "Theme changed to:" << m_currentConfig.name;
    }
}

void UIThemeManager::setCustomTheme(const ThemeConfig& config)
{
    m_currentConfig = config;
    m_currentConfig.type = ThemeType::Custom;
    m_currentConfig.scaleFactor = m_dpiScale;

    updateFontsForScale();
    applyThemeToApplication();

    emit themeChanged(ThemeType::Custom);
    qDebug() << "Custom theme applied:" << config.name;
}

UIThemeManager::ThemeType UIThemeManager::getCurrentTheme() const
{
    return m_currentConfig.type;
}

UIThemeManager::ThemeConfig UIThemeManager::getThemeConfig() const
{
    return m_currentConfig;
}

QColor UIThemeManager::getColor(ColorRole role) const
{
    return m_currentConfig.colors.value(role, QColor("#FF00FF"));  // Magenta fallback
}

void UIThemeManager::setColor(ColorRole role, const QColor& color)
{
    if (isValidColor(color))
    {
        m_currentConfig.colors[role] = color;
        emit colorChanged(role, color);
        applyThemeToApplication();
    }
}

QString UIThemeManager::getColorHex(ColorRole role) const
{
    return getColor(role).name();
}

QFont UIThemeManager::getFont(TypographyScale scale) const
{
    return m_currentConfig.fonts.value(scale, QApplication::font());
}

void UIThemeManager::setFont(TypographyScale scale, const QFont& font)
{
    m_currentConfig.fonts[scale] = scaledFont(font);
    emit fontChanged(scale, font);
    applyThemeToApplication();
}

void UIThemeManager::setBaseFontFamily(const QString& family)
{
    for (auto& font : m_currentConfig.fonts)
    {
        font.setFamily(family);
    }
    applyThemeToApplication();
}

void UIThemeManager::setScaleFactor(double factor)
{
    if (factor > 0.5 && factor <= 3.0)
    {
        m_currentConfig.scaleFactor = factor;
        updateFontsForScale();
        applyThemeToApplication();
    }
}

QString UIThemeManager::generateGlobalStyleSheet() const
{
    ComponentStyles styles = generateAllComponentStyles();

    QString globalStyle = QString("QWidget { "
                                  "    font-family: %1; "
                                  "    font-size: %2pt; "
                                  "    color: %3; "
                                  "    background-color: %4; "
                                  "} ")
                              .arg(getFont(TypographyScale::Body1).family())
                              .arg(getFont(TypographyScale::Body1).pointSize())
                              .arg(getColorHex(ColorRole::Text))
                              .arg(getColorHex(ColorRole::Background));

    return globalStyle + styles.button + styles.lineEdit + styles.comboBox + styles.label + styles.groupBox +
           styles.tabWidget + styles.treeView + styles.tableView + styles.scrollBar + styles.toolBar +
           styles.statusBar + styles.menuBar + styles.menu + styles.dialog + styles.progressBar + styles.slider +
           styles.spinBox + styles.checkBox + styles.radioButton;
}

QString UIThemeManager::generateComponentStyleSheet(const QString& component) const
{
    if (component == "QPushButton")
        return generateButtonStyle();
    if (component == "QLineEdit")
        return generateLineEditStyle();
    if (component == "QComboBox")
        return generateComboBoxStyle();
    if (component == "QLabel")
        return generateLabelStyle();
    if (component == "QGroupBox")
        return generateGroupBoxStyle();
    if (component == "QTabWidget")
        return generateTabWidgetStyle();
    if (component == "QTreeView")
        return generateTreeViewStyle();
    if (component == "QTableView")
        return generateTableViewStyle();
    if (component == "QScrollBar")
        return generateScrollBarStyle();
    if (component == "QToolBar")
        return generateToolBarStyle();
    if (component == "QStatusBar")
        return generateStatusBarStyle();
    if (component == "QMenuBar")
        return generateMenuBarStyle();
    if (component == "QMenu")
        return generateMenuStyle();
    if (component == "QDialog")
        return generateDialogStyle();
    if (component == "QProgressBar")
        return generateProgressBarStyle();
    if (component == "QSlider")
        return generateSliderStyle();
    if (component == "QSpinBox")
        return generateSpinBoxStyle();
    if (component == "QCheckBox")
        return generateCheckBoxStyle();
    if (component == "QRadioButton")
        return generateRadioButtonStyle();

    return QString();
}

UIThemeManager::ComponentStyles UIThemeManager::generateAllComponentStyles() const
{
    ComponentStyles styles;
    styles.button = generateButtonStyle();
    styles.lineEdit = generateLineEditStyle();
    styles.comboBox = generateComboBoxStyle();
    styles.label = generateLabelStyle();
    styles.groupBox = generateGroupBoxStyle();
    styles.tabWidget = generateTabWidgetStyle();
    styles.treeView = generateTreeViewStyle();
    styles.tableView = generateTableViewStyle();
    styles.scrollBar = generateScrollBarStyle();
    styles.toolBar = generateToolBarStyle();
    styles.statusBar = generateStatusBarStyle();
    styles.menuBar = generateMenuBarStyle();
    styles.menu = generateMenuStyle();
    styles.dialog = generateDialogStyle();
    styles.progressBar = generateProgressBarStyle();
    styles.slider = generateSliderStyle();
    styles.spinBox = generateSpinBoxStyle();
    styles.checkBox = generateCheckBoxStyle();
    styles.radioButton = generateRadioButtonStyle();
    return styles;
}

void UIThemeManager::updateForDPI(double dpiScale)
{
    if (dpiScale != m_dpiScale)
    {
        m_dpiScale = dpiScale;
        m_currentConfig.scaleFactor = dpiScale;
        updateFontsForScale();
        applyThemeToApplication();
        emit dpiScaleChanged(dpiScale);
    }
}

double UIThemeManager::getDPIScale() const
{
    return m_dpiScale;
}

int UIThemeManager::scaledSize(int baseSize) const
{
    return static_cast<int>(baseSize * m_currentConfig.scaleFactor);
}

QSize UIThemeManager::scaledSize(const QSize& baseSize) const
{
    return QSize(scaledSize(baseSize.width()), scaledSize(baseSize.height()));
}

void UIThemeManager::saveTheme(const QString& name) const
{
    QString themeName = name.isEmpty() ? m_currentConfig.name : name;
    saveThemeToSettings(m_currentConfig, themeName);
    qDebug() << "Theme saved:" << themeName;
}

bool UIThemeManager::loadTheme(const QString& name)
{
    ThemeConfig config = loadThemeFromSettings(name);
    if (!config.name.isEmpty())
    {
        setCustomTheme(config);
        return true;
    }
    return false;
}

QStringList UIThemeManager::getAvailableThemes() const
{
    QStringList themes;
    themes << "Light"
           << "Dark"
           << "High Contrast";

    // Add custom themes from settings
    QSettings settings;
    settings.beginGroup(m_settingsKey + "/custom");
    themes.append(settings.childGroups());
    settings.endGroup();

    return themes;
}

void UIThemeManager::deleteTheme(const QString& name)
{
    QSettings settings;
    settings.remove(m_settingsKey + "/custom/" + name);
    qDebug() << "Theme deleted:" << name;
}

bool UIThemeManager::isValidColor(const QColor& color) const
{
    return color.isValid() && color.alpha() > 0;
}

QColor UIThemeManager::adjustColorBrightness(const QColor& color, double factor) const
{
    int h, s, l, a;
    color.getHsl(&h, &s, &l, &a);
    l = qBound(0, static_cast<int>(l * factor), 255);
    return QColor::fromHsl(h, s, l, a);
}

QColor UIThemeManager::getContrastingTextColor(const QColor& background) const
{
    double luminance = calculateLuminance(background);
    return luminance > 0.5 ? QColor("#000000") : QColor("#FFFFFF");
}

double UIThemeManager::getColorContrast(const QColor& color1, const QColor& color2) const
{
    double lum1 = calculateLuminance(color1);
    double lum2 = calculateLuminance(color2);

    double lighter = qMax(lum1, lum2);
    double darker = qMin(lum1, lum2);

    return (lighter + 0.05) / (darker + 0.05);
}

void UIThemeManager::updateFontsForScale()
{
    for (auto it = m_currentConfig.fonts.begin(); it != m_currentConfig.fonts.end(); ++it)
    {
        it.value() = scaledFont(it.value());
    }
}

QString UIThemeManager::generateButtonStyle() const
{
    return QString("QPushButton { "
                   "    background-color: %1; "
                   "    color: %2; "
                   "    border: 1px solid %3; "
                   "    border-radius: %4px; "
                   "    padding: %5px %6px; "
                   "    font-weight: bold; "
                   "    min-height: %7px; "
                   "} "
                   "QPushButton:hover { "
                   "    background-color: %8; "
                   "} "
                   "QPushButton:pressed { "
                   "    background-color: %9; "
                   "} "
                   "QPushButton:disabled { "
                   "    background-color: %10; "
                   "    color: %11; "
                   "} ")
        .arg(getColorHex(ColorRole::Primary))
        .arg(getColorHex(ColorRole::Surface))
        .arg(getColorHex(ColorRole::Border))
        .arg(m_currentConfig.borderRadius)
        .arg(scaledSize(8))
        .arg(scaledSize(16))
        .arg(scaledSize(24))
        .arg(getColorHex(ColorRole::Hover))
        .arg(getColorHex(ColorRole::Pressed))
        .arg(getColorHex(ColorRole::Disabled))
        .arg(getColorHex(ColorRole::TextSecondary));
}

QString UIThemeManager::generateLineEditStyle() const
{
    return QString("QLineEdit { "
                   "    background-color: %1; "
                   "    color: %2; "
                   "    border: 1px solid %3; "
                   "    border-radius: %4px; "
                   "    padding: %5px; "
                   "    selection-background-color: %6; "
                   "} "
                   "QLineEdit:focus { "
                   "    border: 2px solid %7; "
                   "} "
                   "QLineEdit:disabled { "
                   "    background-color: %8; "
                   "    color: %9; "
                   "} ")
        .arg(getColorHex(ColorRole::Surface))
        .arg(getColorHex(ColorRole::Text))
        .arg(getColorHex(ColorRole::Border))
        .arg(m_currentConfig.borderRadius)
        .arg(scaledSize(6))
        .arg(getColorHex(ColorRole::Primary))
        .arg(getColorHex(ColorRole::Primary))
        .arg(getColorHex(ColorRole::Disabled))
        .arg(getColorHex(ColorRole::TextSecondary));
}

// Placeholder implementations for remaining style methods
QString UIThemeManager::generateComboBoxStyle() const
{
    return QString();
}
QString UIThemeManager::generateLabelStyle() const
{
    return QString();
}
QString UIThemeManager::generateGroupBoxStyle() const
{
    return QString();
}
QString UIThemeManager::generateTabWidgetStyle() const
{
    return QString();
}
QString UIThemeManager::generateTreeViewStyle() const
{
    return QString();
}
QString UIThemeManager::generateTableViewStyle() const
{
    return QString();
}
QString UIThemeManager::generateScrollBarStyle() const
{
    return QString();
}
QString UIThemeManager::generateToolBarStyle() const
{
    return QString();
}
QString UIThemeManager::generateStatusBarStyle() const
{
    return QString();
}
QString UIThemeManager::generateMenuBarStyle() const
{
    return QString();
}
QString UIThemeManager::generateMenuStyle() const
{
    return QString();
}
QString UIThemeManager::generateDialogStyle() const
{
    return QString();
}
QString UIThemeManager::generateProgressBarStyle() const
{
    return QString();
}
QString UIThemeManager::generateSliderStyle() const
{
    return QString();
}
QString UIThemeManager::generateSpinBoxStyle() const
{
    return QString();
}
QString UIThemeManager::generateCheckBoxStyle() const
{
    return QString();
}
QString UIThemeManager::generateRadioButtonStyle() const
{
    return QString();
}

// Helper methods
QString UIThemeManager::colorToString(const QColor& color) const
{
    return color.name();
}

QString UIThemeManager::fontToString(const QFont& font) const
{
    return QString("%1 %2pt").arg(font.family()).arg(font.pointSize());
}

QFont UIThemeManager::scaledFont(const QFont& baseFont) const
{
    QFont scaled = baseFont;
    scaled.setPointSize(static_cast<int>(baseFont.pointSize() * m_currentConfig.scaleFactor));
    return scaled;
}

void UIThemeManager::applyThemeToApplication()
{
    QString styleSheet = generateGlobalStyleSheet();
    QApplication::setStyleSheet(styleSheet);
}

void UIThemeManager::saveThemeToSettings(const ThemeConfig& config, const QString& name) const
{
    QSettings settings;
    settings.beginGroup(m_settingsKey + "/custom/" + name);

    // Save colors
    settings.beginGroup("colors");
    for (auto it = config.colors.begin(); it != config.colors.end(); ++it)
    {
        settings.setValue(QString::number(static_cast<int>(it.key())), it.value().name());
    }
    settings.endGroup();

    // Save fonts
    settings.beginGroup("fonts");
    for (auto it = config.fonts.begin(); it != config.fonts.end(); ++it)
    {
        settings.setValue(QString::number(static_cast<int>(it.key())), fontToString(it.value()));
    }
    settings.endGroup();

    settings.setValue("name", config.name);
    settings.setValue("borderRadius", config.borderRadius);
    settings.setValue("enableAnimations", config.enableAnimations);
    settings.endGroup();
}

UIThemeManager::ThemeConfig UIThemeManager::loadThemeFromSettings(const QString& name) const
{
    QSettings settings;
    ThemeConfig config;

    settings.beginGroup(m_settingsKey + "/custom/" + name);
    if (!settings.contains("name"))
    {
        settings.endGroup();
        return config;  // Return empty config if theme doesn't exist
    }

    config.name = settings.value("name").toString();
    config.borderRadius = settings.value("borderRadius", 4).toInt();
    config.enableAnimations = settings.value("enableAnimations", true).toBool();

    // Load colors
    settings.beginGroup("colors");
    for (const QString& key : settings.childKeys())
    {
        ColorRole role = static_cast<ColorRole>(key.toInt());
        config.colors[role] = QColor(settings.value(key).toString());
    }
    settings.endGroup();

    settings.endGroup();
    return config;
}

double UIThemeManager::calculateLuminance(const QColor& color) const
{
    double r = color.redF();
    double g = color.greenF();
    double b = color.blueF();

    // Apply gamma correction
    r = (r <= 0.03928) ? r / 12.92 : std::pow((r + 0.055) / 1.055, 2.4);
    g = (g <= 0.03928) ? g / 12.92 : std::pow((g + 0.055) / 1.055, 2.4);
    b = (b <= 0.03928) ? b / 12.92 : std::pow((b + 0.055) / 1.055, 2.4);

    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

void UIThemeManager::onSystemThemeChanged()
{
    // Handle system theme changes if needed
    qDebug() << "System theme changed";
}
