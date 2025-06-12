#ifndef USERPREFERENCES_H
#define USERPREFERENCES_H

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QSize>
#include <QPoint>
#include <QByteArray>
#include <QStringList>
#include <QMap>

/**
 * @brief User preferences and settings management system
 * 
 * This class manages user-specific settings and preferences including:
 * - Window layout and geometry persistence
 * - Application settings and defaults
 * - User customizations and preferences
 * - Settings validation and migration
 * 
 * Sprint 7 Requirements:
 * - Save/restore window layouts using QMainWindow state
 * - Persist user preferences between sessions
 * - Provide smart defaults for new users
 * - Settings validation and error handling
 */
class UserPreferences : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Preference categories for organization
     */
    enum class Category {
        General,
        Interface,
        Performance,
        Rendering,
        Registration,
        Export,
        Advanced
    };

    /**
     * @brief Preference data types
     */
    enum class PreferenceType {
        Boolean,
        Integer,
        Double,
        String,
        StringList,
        Color,
        Font,
        Size,
        Point,
        ByteArray
    };

    /**
     * @brief Preference definition structure
     */
    struct PreferenceDefinition {
        QString key;
        QString displayName;
        QString description;
        PreferenceType type;
        QVariant defaultValue;
        QVariant minValue;
        QVariant maxValue;
        QStringList validValues;
        Category category;
        bool requiresRestart = false;
        bool isAdvanced = false;
    };

    /**
     * @brief Window layout information
     */
    struct WindowLayout {
        QString name;
        QByteArray geometry;
        QByteArray windowState;
        QMap<QString, QVariant> customData;
        qint64 timestamp;
    };

public:
    static UserPreferences& instance();
    explicit UserPreferences(QObject* parent = nullptr);
    ~UserPreferences();

    // Preference management
    void registerPreference(const PreferenceDefinition& definition);
    QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    bool hasValue(const QString& key) const;
    void removeValue(const QString& key);
    void resetToDefault(const QString& key);
    void resetAllToDefaults();

    // Typed getters for convenience
    bool getBool(const QString& key, bool defaultValue = false) const;
    int getInt(const QString& key, int defaultValue = 0) const;
    double getDouble(const QString& key, double defaultValue = 0.0) const;
    QString getString(const QString& key, const QString& defaultValue = QString()) const;
    QStringList getStringList(const QString& key, const QStringList& defaultValue = QStringList()) const;
    QColor getColor(const QString& key, const QColor& defaultValue = QColor()) const;
    QFont getFont(const QString& key, const QFont& defaultValue = QFont()) const;
    QSize getSize(const QString& key, const QSize& defaultValue = QSize()) const;
    QPoint getPoint(const QString& key, const QPoint& defaultValue = QPoint()) const;
    QByteArray getByteArray(const QString& key, const QByteArray& defaultValue = QByteArray()) const;

    // Typed setters for convenience
    void setBool(const QString& key, bool value);
    void setInt(const QString& key, int value);
    void setDouble(const QString& key, double value);
    void setString(const QString& key, const QString& value);
    void setStringList(const QString& key, const QStringList& value);
    void setColor(const QString& key, const QColor& value);
    void setFont(const QString& key, const QFont& value);
    void setSize(const QString& key, const QSize& value);
    void setPoint(const QString& key, const QPoint& value);
    void setByteArray(const QString& key, const QByteArray& value);

    // Window layout management
    void saveWindowLayout(const QString& name, const QByteArray& geometry, 
                         const QByteArray& windowState, const QMap<QString, QVariant>& customData = QMap<QString, QVariant>());
    WindowLayout loadWindowLayout(const QString& name) const;
    QStringList getAvailableLayouts() const;
    void deleteWindowLayout(const QString& name);
    void setDefaultLayout(const QString& name);
    QString getDefaultLayout() const;

    // Settings management
    void sync();
    void clear();
    QString getSettingsFilePath() const;
    bool exportSettings(const QString& filePath) const;
    bool importSettings(const QString& filePath);

    // Validation
    bool isValidValue(const QString& key, const QVariant& value) const;
    QString getValidationError(const QString& key, const QVariant& value) const;
    QStringList validateAllSettings() const;

    // Categories and definitions
    QList<PreferenceDefinition> getPreferencesByCategory(Category category) const;
    QList<PreferenceDefinition> getAllPreferences() const;
    PreferenceDefinition getPreferenceDefinition(const QString& key) const;
    QStringList getModifiedSettings() const;

    // Migration and versioning
    void migrateSettings(int fromVersion, int toVersion);
    int getSettingsVersion() const;
    void setSettingsVersion(int version);

signals:
    void valueChanged(const QString& key, const QVariant& newValue, const QVariant& oldValue);
    void categoryChanged(Category category);
    void settingsReset();
    void layoutSaved(const QString& name);
    void layoutLoaded(const QString& name);
    void settingsImported(const QString& filePath);
    void settingsExported(const QString& filePath);

private slots:
    void onSettingsChanged();

private:
    QSettings* m_settings;
    QMap<QString, PreferenceDefinition> m_definitions;
    QMap<QString, QVariant> m_cache;
    bool m_cacheEnabled;
    QString m_organizationName;
    QString m_applicationName;
    
    void initializeDefaultPreferences();
    void initializeGeneralPreferences();
    void initializeInterfacePreferences();
    void initializePerformancePreferences();
    void initializeRenderingPreferences();
    void initializeRegistrationPreferences();
    void initializeExportPreferences();
    void initializeAdvancedPreferences();
    
    QVariant convertValue(const QVariant& value, PreferenceType targetType) const;
    bool validateValueRange(const QVariant& value, const QVariant& minValue, const QVariant& maxValue) const;
    bool validateValueList(const QVariant& value, const QStringList& validValues) const;
    
    QString categoryToString(Category category) const;
    Category stringToCategory(const QString& categoryStr) const;
    QString typeToString(PreferenceType type) const;
    PreferenceType stringToType(const QString& typeStr) const;
    
    void updateCache(const QString& key, const QVariant& value);
    void clearCache();
    
    // Migration helpers
    void migrateFromVersion1To2();
    void migrateFromVersion2To3();
    
    // Default values
    static constexpr int CURRENT_SETTINGS_VERSION = 3;
    static constexpr const char* SETTINGS_VERSION_KEY = "SettingsVersion";
    static constexpr const char* WINDOW_LAYOUTS_GROUP = "WindowLayouts";
    static constexpr const char* DEFAULT_LAYOUT_KEY = "DefaultLayout";
};

#endif // USERPREFERENCES_H
