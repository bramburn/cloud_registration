#include "UserPreferences.h"
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>

UserPreferences& UserPreferences::instance() {
    static UserPreferences instance;
    return instance;
}

UserPreferences::UserPreferences(QObject* parent)
    : QObject(parent)
    , m_cacheEnabled(true)
    , m_organizationName("FaroSceneRegistration")
    , m_applicationName("PointCloudRegistration") {
    
    // Initialize QSettings
    QCoreApplication::setOrganizationName(m_organizationName);
    QCoreApplication::setApplicationName(m_applicationName);
    m_settings = new QSettings(this);
    
    // Initialize default preferences
    initializeDefaultPreferences();
    
    // Check for settings migration
    int currentVersion = getSettingsVersion();
    if (currentVersion < CURRENT_SETTINGS_VERSION) {
        migrateSettings(currentVersion, CURRENT_SETTINGS_VERSION);
        setSettingsVersion(CURRENT_SETTINGS_VERSION);
    }
    
    qDebug() << "UserPreferences initialized with settings file:" << m_settings->fileName();
}

UserPreferences::~UserPreferences() {
    sync();
}

void UserPreferences::initializeDefaultPreferences() {
    initializeGeneralPreferences();
    initializeInterfacePreferences();
    initializePerformancePreferences();
    initializeRenderingPreferences();
    initializeRegistrationPreferences();
    initializeExportPreferences();
    initializeAdvancedPreferences();
}

void UserPreferences::initializeGeneralPreferences() {
    registerPreference({
        "general/autoSave", "Auto Save", "Automatically save projects",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::General, false, false
    });
    
    registerPreference({
        "general/autoSaveInterval", "Auto Save Interval", "Auto save interval in minutes",
        PreferenceType::Integer, 5, 1, 60, QStringList(),
        Category::General, false, false
    });
    
    registerPreference({
        "general/recentFilesCount", "Recent Files Count", "Number of recent files to remember",
        PreferenceType::Integer, 10, 1, 50, QStringList(),
        Category::General, false, false
    });
    
    registerPreference({
        "general/confirmExit", "Confirm Exit", "Show confirmation dialog when exiting",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::General, false, false
    });
}

void UserPreferences::initializeInterfacePreferences() {
    registerPreference({
        "interface/theme", "Theme", "Application theme",
        PreferenceType::String, "Light", QVariant(), QVariant(), 
        QStringList() << "Light" << "Dark" << "High Contrast",
        Category::Interface, false, false
    });
    
    registerPreference({
        "interface/fontSize", "Font Size", "Base font size",
        PreferenceType::Integer, 10, 8, 24, QStringList(),
        Category::Interface, false, false
    });
    
    registerPreference({
        "interface/showToolTips", "Show Tool Tips", "Display helpful tooltips",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::Interface, false, false
    });
    
    registerPreference({
        "interface/animationsEnabled", "Enable Animations", "Enable UI animations",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::Interface, false, false
    });
}

void UserPreferences::initializePerformancePreferences() {
    registerPreference({
        "performance/maxThreads", "Max Threads", "Maximum number of processing threads",
        PreferenceType::Integer, -1, -1, 32, QStringList(),
        Category::Performance, true, false
    });
    
    registerPreference({
        "performance/memoryLimit", "Memory Limit (GB)", "Maximum memory usage in GB",
        PreferenceType::Double, 4.0, 1.0, 64.0, QStringList(),
        Category::Performance, true, false
    });
    
    registerPreference({
        "performance/enableGPU", "Enable GPU Acceleration", "Use GPU for computations",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::Performance, true, false
    });
}

void UserPreferences::initializeRenderingPreferences() {
    registerPreference({
        "rendering/pointSize", "Point Size", "Default point size for rendering",
        PreferenceType::Double, 2.0, 0.1, 10.0, QStringList(),
        Category::Rendering, false, false
    });
    
    registerPreference({
        "rendering/backgroundColor", "Background Color", "3D viewer background color",
        PreferenceType::Color, QColor("#F0F0F0"), QVariant(), QVariant(), QStringList(),
        Category::Rendering, false, false
    });
    
    registerPreference({
        "rendering/lodEnabled", "Enable LOD", "Enable level-of-detail rendering",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::Rendering, false, false
    });
    
    registerPreference({
        "rendering/maxFPS", "Max FPS", "Maximum frame rate",
        PreferenceType::Integer, 60, 15, 120, QStringList(),
        Category::Rendering, false, false
    });
}

void UserPreferences::initializeRegistrationPreferences() {
    registerPreference({
        "registration/icpMaxIterations", "ICP Max Iterations", "Maximum ICP iterations",
        PreferenceType::Integer, 100, 10, 1000, QStringList(),
        Category::Registration, false, false
    });
    
    registerPreference({
        "registration/icpTolerance", "ICP Tolerance", "ICP convergence tolerance",
        PreferenceType::Double, 0.001, 0.0001, 0.1, QStringList(),
        Category::Registration, false, false
    });
    
    registerPreference({
        "registration/sphereRadius", "Default Sphere Radius", "Default sphere target radius (mm)",
        PreferenceType::Double, 75.0, 10.0, 500.0, QStringList(),
        Category::Registration, false, false
    });
}

void UserPreferences::initializeExportPreferences() {
    registerPreference({
        "export/defaultFormat", "Default Export Format", "Default file format for exports",
        PreferenceType::String, "E57", QVariant(), QVariant(),
        QStringList() << "E57" << "LAS" << "PLY" << "XYZ",
        Category::Export, false, false
    });
    
    registerPreference({
        "export/compressionEnabled", "Enable Compression", "Compress exported files",
        PreferenceType::Boolean, true, QVariant(), QVariant(), QStringList(),
        Category::Export, false, false
    });
}

void UserPreferences::initializeAdvancedPreferences() {
    registerPreference({
        "advanced/debugMode", "Debug Mode", "Enable debug logging",
        PreferenceType::Boolean, false, QVariant(), QVariant(), QStringList(),
        Category::Advanced, true, true
    });
    
    registerPreference({
        "advanced/profilingEnabled", "Enable Profiling", "Enable performance profiling",
        PreferenceType::Boolean, false, QVariant(), QVariant(), QStringList(),
        Category::Advanced, false, true
    });
}

void UserPreferences::registerPreference(const PreferenceDefinition& definition) {
    m_definitions[definition.key] = definition;
    
    // Set default value if not already set
    if (!hasValue(definition.key)) {
        setValue(definition.key, definition.defaultValue);
    }
}

QVariant UserPreferences::getValue(const QString& key, const QVariant& defaultValue) const {
    if (m_cacheEnabled && m_cache.contains(key)) {
        return m_cache[key];
    }
    
    QVariant value = m_settings->value(key, defaultValue);
    
    if (m_cacheEnabled) {
        const_cast<UserPreferences*>(this)->updateCache(key, value);
    }
    
    return value;
}

void UserPreferences::setValue(const QString& key, const QVariant& value) {
    if (!isValidValue(key, value)) {
        qWarning() << "Invalid value for preference" << key << ":" << getValidationError(key, value);
        return;
    }
    
    QVariant oldValue = getValue(key);
    m_settings->setValue(key, value);
    updateCache(key, value);
    
    emit valueChanged(key, value, oldValue);
    
    // Emit category change if this preference belongs to a category
    if (m_definitions.contains(key)) {
        emit categoryChanged(m_definitions[key].category);
    }
}

bool UserPreferences::hasValue(const QString& key) const {
    return m_settings->contains(key);
}

void UserPreferences::removeValue(const QString& key) {
    QVariant oldValue = getValue(key);
    m_settings->remove(key);
    m_cache.remove(key);
    
    emit valueChanged(key, QVariant(), oldValue);
}

void UserPreferences::resetToDefault(const QString& key) {
    if (m_definitions.contains(key)) {
        setValue(key, m_definitions[key].defaultValue);
    }
}

void UserPreferences::resetAllToDefaults() {
    for (auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
        setValue(it.key(), it.value().defaultValue);
    }
    emit settingsReset();
}

// Typed getters
bool UserPreferences::getBool(const QString& key, bool defaultValue) const {
    return getValue(key, defaultValue).toBool();
}

int UserPreferences::getInt(const QString& key, int defaultValue) const {
    return getValue(key, defaultValue).toInt();
}

double UserPreferences::getDouble(const QString& key, double defaultValue) const {
    return getValue(key, defaultValue).toDouble();
}

QString UserPreferences::getString(const QString& key, const QString& defaultValue) const {
    return getValue(key, defaultValue).toString();
}

QStringList UserPreferences::getStringList(const QString& key, const QStringList& defaultValue) const {
    return getValue(key, defaultValue).toStringList();
}

QColor UserPreferences::getColor(const QString& key, const QColor& defaultValue) const {
    QVariant value = getValue(key, defaultValue);
    if (value.canConvert<QColor>()) {
        return value.value<QColor>();
    }
    return QColor(value.toString());
}

QFont UserPreferences::getFont(const QString& key, const QFont& defaultValue) const {
    QVariant value = getValue(key, defaultValue);
    if (value.canConvert<QFont>()) {
        return value.value<QFont>();
    }
    return defaultValue;
}

QSize UserPreferences::getSize(const QString& key, const QSize& defaultValue) const {
    return getValue(key, defaultValue).toSize();
}

QPoint UserPreferences::getPoint(const QString& key, const QPoint& defaultValue) const {
    return getValue(key, defaultValue).toPoint();
}

QByteArray UserPreferences::getByteArray(const QString& key, const QByteArray& defaultValue) const {
    return getValue(key, defaultValue).toByteArray();
}

// Typed setters
void UserPreferences::setBool(const QString& key, bool value) { setValue(key, value); }
void UserPreferences::setInt(const QString& key, int value) { setValue(key, value); }
void UserPreferences::setDouble(const QString& key, double value) { setValue(key, value); }
void UserPreferences::setString(const QString& key, const QString& value) { setValue(key, value); }
void UserPreferences::setStringList(const QString& key, const QStringList& value) { setValue(key, value); }
void UserPreferences::setColor(const QString& key, const QColor& value) { setValue(key, value); }
void UserPreferences::setFont(const QString& key, const QFont& value) { setValue(key, value); }
void UserPreferences::setSize(const QString& key, const QSize& value) { setValue(key, value); }
void UserPreferences::setPoint(const QString& key, const QPoint& value) { setValue(key, value); }
void UserPreferences::setByteArray(const QString& key, const QByteArray& value) { setValue(key, value); }

// Window layout management
void UserPreferences::saveWindowLayout(const QString& name, const QByteArray& geometry,
                                     const QByteArray& windowState, const QMap<QString, QVariant>& customData) {
    m_settings->beginGroup(WINDOW_LAYOUTS_GROUP);
    m_settings->beginGroup(name);

    m_settings->setValue("geometry", geometry);
    m_settings->setValue("windowState", windowState);
    m_settings->setValue("timestamp", QDateTime::currentMSecsSinceEpoch());

    // Save custom data
    m_settings->beginGroup("customData");
    for (auto it = customData.begin(); it != customData.end(); ++it) {
        m_settings->setValue(it.key(), it.value());
    }
    m_settings->endGroup();

    m_settings->endGroup();
    m_settings->endGroup();

    emit layoutSaved(name);
    qDebug() << "Window layout saved:" << name;
}

UserPreferences::WindowLayout UserPreferences::loadWindowLayout(const QString& name) const {
    WindowLayout layout;

    m_settings->beginGroup(WINDOW_LAYOUTS_GROUP);
    m_settings->beginGroup(name);

    if (m_settings->contains("geometry")) {
        layout.name = name;
        layout.geometry = m_settings->value("geometry").toByteArray();
        layout.windowState = m_settings->value("windowState").toByteArray();
        layout.timestamp = m_settings->value("timestamp", 0).toLongLong();

        // Load custom data
        m_settings->beginGroup("customData");
        for (const QString& key : m_settings->childKeys()) {
            layout.customData[key] = m_settings->value(key);
        }
        m_settings->endGroup();
    }

    m_settings->endGroup();
    m_settings->endGroup();

    return layout;
}

QStringList UserPreferences::getAvailableLayouts() const {
    m_settings->beginGroup(WINDOW_LAYOUTS_GROUP);
    QStringList layouts = m_settings->childGroups();
    m_settings->endGroup();
    return layouts;
}

void UserPreferences::deleteWindowLayout(const QString& name) {
    m_settings->beginGroup(WINDOW_LAYOUTS_GROUP);
    m_settings->remove(name);
    m_settings->endGroup();
    qDebug() << "Window layout deleted:" << name;
}

void UserPreferences::setDefaultLayout(const QString& name) {
    setValue(DEFAULT_LAYOUT_KEY, name);
}

QString UserPreferences::getDefaultLayout() const {
    return getString(DEFAULT_LAYOUT_KEY, "Default");
}

// Settings management
void UserPreferences::sync() {
    m_settings->sync();
}

void UserPreferences::clear() {
    m_settings->clear();
    clearCache();
    emit settingsReset();
}

QString UserPreferences::getSettingsFilePath() const {
    return m_settings->fileName();
}

bool UserPreferences::exportSettings(const QString& filePath) const {
    QJsonObject root;

    // Export all settings
    for (const QString& key : m_settings->allKeys()) {
        QVariant value = m_settings->value(key);
        if (value.canConvert<QString>()) {
            root[key] = value.toString();
        } else if (value.canConvert<int>()) {
            root[key] = value.toInt();
        } else if (value.canConvert<double>()) {
            root[key] = value.toDouble();
        } else if (value.canConvert<bool>()) {
            root[key] = value.toBool();
        }
    }

    QJsonDocument doc(root);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        emit settingsExported(filePath);
        return true;
    }

    return false;
}

bool UserPreferences::importSettings(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        QString key = it.key();
        QJsonValue jsonValue = it.value();

        QVariant value;
        if (jsonValue.isBool()) {
            value = jsonValue.toBool();
        } else if (jsonValue.isDouble()) {
            value = jsonValue.toDouble();
        } else if (jsonValue.isString()) {
            value = jsonValue.toString();
        }

        if (value.isValid()) {
            setValue(key, value);
        }
    }

    emit settingsImported(filePath);
    return true;
}

// Validation
bool UserPreferences::isValidValue(const QString& key, const QVariant& value) const {
    if (!m_definitions.contains(key)) {
        return true; // Allow unknown keys
    }

    const PreferenceDefinition& def = m_definitions[key];

    // Check type compatibility
    QVariant convertedValue = convertValue(value, def.type);
    if (!convertedValue.isValid()) {
        return false;
    }

    // Check range
    if (!validateValueRange(convertedValue, def.minValue, def.maxValue)) {
        return false;
    }

    // Check valid values list
    if (!validateValueList(convertedValue, def.validValues)) {
        return false;
    }

    return true;
}

QString UserPreferences::getValidationError(const QString& key, const QVariant& value) const {
    if (!m_definitions.contains(key)) {
        return QString();
    }

    const PreferenceDefinition& def = m_definitions[key];

    // Check type compatibility
    QVariant convertedValue = convertValue(value, def.type);
    if (!convertedValue.isValid()) {
        return QString("Invalid type for %1. Expected %2.").arg(key, typeToString(def.type));
    }

    // Check range
    if (!validateValueRange(convertedValue, def.minValue, def.maxValue)) {
        return QString("Value for %1 is out of range [%2, %3]").arg(key, def.minValue.toString(), def.maxValue.toString());
    }

    // Check valid values list
    if (!validateValueList(convertedValue, def.validValues)) {
        return QString("Invalid value for %1. Valid values are: %2").arg(key, def.validValues.join(", "));
    }

    return QString();
}

QStringList UserPreferences::validateAllSettings() const {
    QStringList errors;

    for (const QString& key : m_settings->allKeys()) {
        QVariant value = getValue(key);
        QString error = getValidationError(key, value);
        if (!error.isEmpty()) {
            errors.append(error);
        }
    }

    return errors;
}

// Categories and definitions
QList<UserPreferences::PreferenceDefinition> UserPreferences::getPreferencesByCategory(Category category) const {
    QList<PreferenceDefinition> result;

    for (auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
        if (it.value().category == category) {
            result.append(it.value());
        }
    }

    return result;
}

QList<UserPreferences::PreferenceDefinition> UserPreferences::getAllPreferences() const {
    return m_definitions.values();
}

UserPreferences::PreferenceDefinition UserPreferences::getPreferenceDefinition(const QString& key) const {
    return m_definitions.value(key, PreferenceDefinition());
}

QStringList UserPreferences::getModifiedSettings() const {
    QStringList modified;

    for (auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
        QVariant currentValue = getValue(it.key());
        if (currentValue != it.value().defaultValue) {
            modified.append(it.key());
        }
    }

    return modified;
}

// Migration and versioning
void UserPreferences::migrateSettings(int fromVersion, int toVersion) {
    qDebug() << "Migrating settings from version" << fromVersion << "to" << toVersion;

    for (int version = fromVersion; version < toVersion; ++version) {
        switch (version) {
        case 1:
            migrateFromVersion1To2();
            break;
        case 2:
            migrateFromVersion2To3();
            break;
        default:
            qWarning() << "Unknown migration version:" << version;
            break;
        }
    }
}

int UserPreferences::getSettingsVersion() const {
    return getInt(SETTINGS_VERSION_KEY, 1);
}

void UserPreferences::setSettingsVersion(int version) {
    setInt(SETTINGS_VERSION_KEY, version);
}

// Helper methods
void UserPreferences::updateCache(const QString& key, const QVariant& value) {
    if (m_cacheEnabled) {
        m_cache[key] = value;
    }
}

void UserPreferences::clearCache() {
    m_cache.clear();
}

QVariant UserPreferences::convertValue(const QVariant& value, PreferenceType targetType) const {
    switch (targetType) {
    case PreferenceType::Boolean:
        return value.toBool();
    case PreferenceType::Integer:
        return value.toInt();
    case PreferenceType::Double:
        return value.toDouble();
    case PreferenceType::String:
        return value.toString();
    case PreferenceType::StringList:
        return value.toStringList();
    case PreferenceType::Color:
        if (value.canConvert<QColor>()) {
            return value;
        }
        return QColor(value.toString());
    case PreferenceType::Font:
        if (value.canConvert<QFont>()) {
            return value;
        }
        return QVariant();
    case PreferenceType::Size:
        return value.toSize();
    case PreferenceType::Point:
        return value.toPoint();
    case PreferenceType::ByteArray:
        return value.toByteArray();
    }
    return QVariant();
}

bool UserPreferences::validateValueRange(const QVariant& value, const QVariant& minValue, const QVariant& maxValue) const {
    if (!minValue.isValid() && !maxValue.isValid()) {
        return true;
    }

    if (value.canConvert<double>()) {
        double val = value.toDouble();
        if (minValue.isValid() && val < minValue.toDouble()) {
            return false;
        }
        if (maxValue.isValid() && val > maxValue.toDouble()) {
            return false;
        }
    }

    return true;
}

bool UserPreferences::validateValueList(const QVariant& value, const QStringList& validValues) const {
    if (validValues.isEmpty()) {
        return true;
    }

    return validValues.contains(value.toString());
}

QString UserPreferences::typeToString(PreferenceType type) const {
    switch (type) {
    case PreferenceType::Boolean: return "Boolean";
    case PreferenceType::Integer: return "Integer";
    case PreferenceType::Double: return "Double";
    case PreferenceType::String: return "String";
    case PreferenceType::StringList: return "StringList";
    case PreferenceType::Color: return "Color";
    case PreferenceType::Font: return "Font";
    case PreferenceType::Size: return "Size";
    case PreferenceType::Point: return "Point";
    case PreferenceType::ByteArray: return "ByteArray";
    }
    return "Unknown";
}

// Migration implementations
void UserPreferences::migrateFromVersion1To2() {
    // Example migration: rename old keys
    if (hasValue("oldKey")) {
        setValue("newKey", getValue("oldKey"));
        removeValue("oldKey");
    }
}

void UserPreferences::migrateFromVersion2To3() {
    // Example migration: convert values
    if (hasValue("performance/threads")) {
        int oldThreads = getInt("performance/threads");
        setInt("performance/maxThreads", oldThreads);
        removeValue("performance/threads");
    }
}

void UserPreferences::onSettingsChanged() {
    // Handle external settings changes
    clearCache();
}
