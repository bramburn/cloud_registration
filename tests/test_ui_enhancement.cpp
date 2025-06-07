#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QSettings>
#include <QTemporaryDir>
#include <QMainWindow>
#include <QSplitter>
#include <memory>

#include "../src/ui/UIThemeManager.h"
#include "../src/ui/UserPreferences.h"

/**
 * @brief Test suite for Sprint 7 UI enhancement components
 * 
 * Tests cover:
 * - UIThemeManager functionality and theming
 * - UserPreferences management and persistence
 * - UI consistency and validation
 * - Settings migration and compatibility
 */
class UIEnhancementTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application if not already done
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Use temporary directory for test settings
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        
        // Set up test settings
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, tempDir->path());
        
        themeManager = &UIThemeManager::instance();
        userPreferences = &UserPreferences::instance();
    }
    
    void TearDown() override {
        // Clean up
        QSettings settings;
        settings.clear();
        settings.sync();
    }
    
    std::unique_ptr<QApplication> app;
    std::unique_ptr<QTemporaryDir> tempDir;
    UIThemeManager* themeManager;
    UserPreferences* userPreferences;
};

// UIThemeManager Tests
TEST_F(UIEnhancementTest, ThemeManagerBasicFunctionality) {
    // Test initial state
    EXPECT_EQ(themeManager->getCurrentTheme(), UIThemeManager::ThemeType::Light);
    
    // Test theme switching
    themeManager->setTheme(UIThemeManager::ThemeType::Dark);
    EXPECT_EQ(themeManager->getCurrentTheme(), UIThemeManager::ThemeType::Dark);
    
    themeManager->setTheme(UIThemeManager::ThemeType::HighContrast);
    EXPECT_EQ(themeManager->getCurrentTheme(), UIThemeManager::ThemeType::HighContrast);
}

TEST_F(UIEnhancementTest, ThemeManagerColorManagement) {
    // Test color retrieval
    QColor primaryColor = themeManager->getColor(UIThemeManager::ColorRole::Primary);
    EXPECT_TRUE(primaryColor.isValid());
    
    // Test color modification
    QColor newPrimary("#FF5722");
    themeManager->setColor(UIThemeManager::ColorRole::Primary, newPrimary);
    
    QColor retrievedColor = themeManager->getColor(UIThemeManager::ColorRole::Primary);
    EXPECT_EQ(retrievedColor, newPrimary);
    
    // Test hex color retrieval
    QString hexColor = themeManager->getColorHex(UIThemeManager::ColorRole::Primary);
    EXPECT_EQ(hexColor.toUpper(), newPrimary.name().toUpper());
}

TEST_F(UIEnhancementTest, ThemeManagerTypography) {
    // Test font retrieval
    QFont bodyFont = themeManager->getFont(UIThemeManager::TypographyScale::Body1);
    EXPECT_TRUE(!bodyFont.family().isEmpty());
    EXPECT_GT(bodyFont.pointSize(), 0);
    
    // Test font modification
    QFont newFont("Arial", 14, QFont::Bold);
    themeManager->setFont(UIThemeManager::TypographyScale::Headline1, newFont);
    
    QFont retrievedFont = themeManager->getFont(UIThemeManager::TypographyScale::Headline1);
    EXPECT_EQ(retrievedFont.family(), newFont.family());
    EXPECT_EQ(retrievedFont.weight(), newFont.weight());
}

TEST_F(UIEnhancementTest, ThemeManagerDPIScaling) {
    double originalScale = themeManager->getDPIScale();
    
    // Test DPI scaling
    themeManager->updateForDPI(2.0);
    EXPECT_EQ(themeManager->getDPIScale(), 2.0);
    
    // Test scaled size calculation
    int baseSize = 10;
    int scaledSize = themeManager->scaledSize(baseSize);
    EXPECT_EQ(scaledSize, 20); // 10 * 2.0
    
    // Test scaled QSize
    QSize baseQSize(10, 20);
    QSize scaledQSize = themeManager->scaledSize(baseQSize);
    EXPECT_EQ(scaledQSize, QSize(20, 40));
    
    // Restore original scale
    themeManager->updateForDPI(originalScale);
}

TEST_F(UIEnhancementTest, ThemeManagerStyleSheetGeneration) {
    // Test global style sheet generation
    QString globalStyleSheet = themeManager->generateGlobalStyleSheet();
    EXPECT_FALSE(globalStyleSheet.isEmpty());
    EXPECT_TRUE(globalStyleSheet.contains("QWidget"));
    
    // Test component-specific style sheet
    QString buttonStyle = themeManager->generateComponentStyleSheet("QPushButton");
    EXPECT_FALSE(buttonStyle.isEmpty());
    EXPECT_TRUE(buttonStyle.contains("QPushButton"));
    
    // Test all component styles
    auto allStyles = themeManager->generateAllComponentStyles();
    EXPECT_FALSE(allStyles.button.isEmpty());
    EXPECT_FALSE(allStyles.lineEdit.isEmpty());
}

TEST_F(UIEnhancementTest, ThemeManagerCustomTheme) {
    UIThemeManager::ThemeConfig customConfig;
    customConfig.name = "TestTheme";
    customConfig.type = UIThemeManager::ThemeType::Custom;
    customConfig.colors[UIThemeManager::ColorRole::Primary] = QColor("#123456");
    customConfig.colors[UIThemeManager::ColorRole::Background] = QColor("#ABCDEF");
    
    themeManager->setCustomTheme(customConfig);
    
    EXPECT_EQ(themeManager->getCurrentTheme(), UIThemeManager::ThemeType::Custom);
    EXPECT_EQ(themeManager->getColor(UIThemeManager::ColorRole::Primary), QColor("#123456"));
    EXPECT_EQ(themeManager->getColor(UIThemeManager::ColorRole::Background), QColor("#ABCDEF"));
}

TEST_F(UIEnhancementTest, ThemeManagerColorUtilities) {
    QColor testColor("#FF0000"); // Red
    
    // Test color brightness adjustment
    QColor lighterColor = themeManager->adjustColorBrightness(testColor, 1.5);
    QColor darkerColor = themeManager->adjustColorBrightness(testColor, 0.5);
    
    EXPECT_TRUE(lighterColor.isValid());
    EXPECT_TRUE(darkerColor.isValid());
    
    // Test contrasting text color
    QColor whiteBackground("#FFFFFF");
    QColor blackBackground("#000000");
    
    QColor textForWhite = themeManager->getContrastingTextColor(whiteBackground);
    QColor textForBlack = themeManager->getContrastingTextColor(blackBackground);
    
    // Should return dark text for white background and light text for black background
    EXPECT_TRUE(textForWhite.lightness() < 128);
    EXPECT_TRUE(textForBlack.lightness() > 128);
    
    // Test color contrast calculation
    double contrast = themeManager->getColorContrast(whiteBackground, blackBackground);
    EXPECT_GT(contrast, 10.0); // High contrast between white and black
}

// UserPreferences Tests
TEST_F(UIEnhancementTest, UserPreferencesBasicOperations) {
    // Test setting and getting values
    userPreferences->setValue("test/string", "Hello World");
    userPreferences->setValue("test/integer", 42);
    userPreferences->setValue("test/boolean", true);
    userPreferences->setValue("test/double", 3.14159);
    
    EXPECT_EQ(userPreferences->getString("test/string"), "Hello World");
    EXPECT_EQ(userPreferences->getInt("test/integer"), 42);
    EXPECT_EQ(userPreferences->getBool("test/boolean"), true);
    EXPECT_DOUBLE_EQ(userPreferences->getDouble("test/double"), 3.14159);
    
    // Test hasValue
    EXPECT_TRUE(userPreferences->hasValue("test/string"));
    EXPECT_FALSE(userPreferences->hasValue("test/nonexistent"));
    
    // Test removeValue
    userPreferences->removeValue("test/string");
    EXPECT_FALSE(userPreferences->hasValue("test/string"));
}

TEST_F(UIEnhancementTest, UserPreferencesTypedSettersGetters) {
    // Test typed setters and getters
    userPreferences->setString("typed/string", "Test String");
    userPreferences->setInt("typed/int", 123);
    userPreferences->setBool("typed/bool", false);
    userPreferences->setDouble("typed/double", 2.718);
    
    QStringList testList = {"item1", "item2", "item3"};
    userPreferences->setStringList("typed/stringlist", testList);
    
    QColor testColor("#FF5722");
    userPreferences->setColor("typed/color", testColor);
    
    QSize testSize(800, 600);
    userPreferences->setSize("typed/size", testSize);
    
    QPoint testPoint(100, 200);
    userPreferences->setPoint("typed/point", testPoint);
    
    // Verify retrieval
    EXPECT_EQ(userPreferences->getString("typed/string"), "Test String");
    EXPECT_EQ(userPreferences->getInt("typed/int"), 123);
    EXPECT_EQ(userPreferences->getBool("typed/bool"), false);
    EXPECT_DOUBLE_EQ(userPreferences->getDouble("typed/double"), 2.718);
    EXPECT_EQ(userPreferences->getStringList("typed/stringlist"), testList);
    EXPECT_EQ(userPreferences->getColor("typed/color"), testColor);
    EXPECT_EQ(userPreferences->getSize("typed/size"), testSize);
    EXPECT_EQ(userPreferences->getPoint("typed/point"), testPoint);
}

TEST_F(UIEnhancementTest, UserPreferencesWindowLayoutManagement) {
    // Create test window layout data
    QByteArray testGeometry("test_geometry_data");
    QByteArray testWindowState("test_window_state_data");
    QMap<QString, QVariant> customData;
    customData["splitter1"] = QByteArray("splitter_state_1");
    customData["splitter2"] = QByteArray("splitter_state_2");
    
    // Save window layout
    userPreferences->saveWindowLayout("TestLayout", testGeometry, testWindowState, customData);
    
    // Verify layout is in available layouts
    QStringList layouts = userPreferences->getAvailableLayouts();
    EXPECT_TRUE(layouts.contains("TestLayout"));
    
    // Load window layout
    auto loadedLayout = userPreferences->loadWindowLayout("TestLayout");
    EXPECT_EQ(loadedLayout.name, "TestLayout");
    EXPECT_EQ(loadedLayout.geometry, testGeometry);
    EXPECT_EQ(loadedLayout.windowState, testWindowState);
    EXPECT_EQ(loadedLayout.customData["splitter1"], customData["splitter1"]);
    EXPECT_EQ(loadedLayout.customData["splitter2"], customData["splitter2"]);
    EXPECT_GT(loadedLayout.timestamp, 0);
    
    // Test default layout
    userPreferences->setDefaultLayout("TestLayout");
    EXPECT_EQ(userPreferences->getDefaultLayout(), "TestLayout");
    
    // Delete layout
    userPreferences->deleteWindowLayout("TestLayout");
    layouts = userPreferences->getAvailableLayouts();
    EXPECT_FALSE(layouts.contains("TestLayout"));
}

TEST_F(UIEnhancementTest, UserPreferencesValidation) {
    // Register a preference with validation
    UserPreferences::PreferenceDefinition testPref;
    testPref.key = "test/validated_int";
    testPref.displayName = "Test Validated Integer";
    testPref.type = UserPreferences::PreferenceType::Integer;
    testPref.defaultValue = 50;
    testPref.minValue = 10;
    testPref.maxValue = 100;
    testPref.category = UserPreferences::Category::General;
    
    userPreferences->registerPreference(testPref);
    
    // Test valid value
    EXPECT_TRUE(userPreferences->isValidValue("test/validated_int", 75));
    userPreferences->setValue("test/validated_int", 75);
    EXPECT_EQ(userPreferences->getInt("test/validated_int"), 75);
    
    // Test invalid values
    EXPECT_FALSE(userPreferences->isValidValue("test/validated_int", 5));   // Below min
    EXPECT_FALSE(userPreferences->isValidValue("test/validated_int", 150)); // Above max
    
    // Test validation error messages
    QString error1 = userPreferences->getValidationError("test/validated_int", 5);
    QString error2 = userPreferences->getValidationError("test/validated_int", 150);
    
    EXPECT_FALSE(error1.isEmpty());
    EXPECT_FALSE(error2.isEmpty());
    EXPECT_TRUE(error1.contains("range"));
    EXPECT_TRUE(error2.contains("range"));
}

TEST_F(UIEnhancementTest, UserPreferencesSignals) {
    QSignalSpy valueChangedSpy(userPreferences, &UserPreferences::valueChanged);
    QSignalSpy categoryChangedSpy(userPreferences, &UserPreferences::categoryChanged);
    QSignalSpy layoutSavedSpy(userPreferences, &UserPreferences::layoutSaved);
    
    // Test value changed signal
    userPreferences->setValue("signal/test", "test_value");
    EXPECT_EQ(valueChangedSpy.count(), 1);
    
    QList<QVariant> arguments = valueChangedSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), "signal/test");
    EXPECT_EQ(arguments.at(1).toString(), "test_value");
    
    // Test layout saved signal
    userPreferences->saveWindowLayout("SignalTestLayout", QByteArray(), QByteArray());
    EXPECT_EQ(layoutSavedSpy.count(), 1);
    EXPECT_EQ(layoutSavedSpy.first().at(0).toString(), "SignalTestLayout");
}

TEST_F(UIEnhancementTest, UserPreferencesSettingsExportImport) {
    // Set up some test preferences
    userPreferences->setString("export/test1", "value1");
    userPreferences->setInt("export/test2", 42);
    userPreferences->setBool("export/test3", true);
    userPreferences->setDouble("export/test4", 3.14);
    
    // Export settings
    QString exportPath = tempDir->path() + "/exported_settings.json";
    bool exportSuccess = userPreferences->exportSettings(exportPath);
    EXPECT_TRUE(exportSuccess);
    EXPECT_TRUE(QFile::exists(exportPath));
    
    // Clear current settings
    userPreferences->removeValue("export/test1");
    userPreferences->removeValue("export/test2");
    userPreferences->removeValue("export/test3");
    userPreferences->removeValue("export/test4");
    
    // Verify settings are cleared
    EXPECT_FALSE(userPreferences->hasValue("export/test1"));
    EXPECT_FALSE(userPreferences->hasValue("export/test2"));
    
    // Import settings
    bool importSuccess = userPreferences->importSettings(exportPath);
    EXPECT_TRUE(importSuccess);
    
    // Verify imported values
    EXPECT_EQ(userPreferences->getString("export/test1"), "value1");
    EXPECT_EQ(userPreferences->getInt("export/test2"), 42);
    EXPECT_EQ(userPreferences->getBool("export/test3"), true);
    EXPECT_DOUBLE_EQ(userPreferences->getDouble("export/test4"), 3.14);
}

TEST_F(UIEnhancementTest, UserPreferencesDefaultValues) {
    // Test that default preferences are properly initialized
    QList<UserPreferences::PreferenceDefinition> allPrefs = userPreferences->getAllPreferences();
    EXPECT_GT(allPrefs.size(), 0);
    
    // Check some expected default preferences
    EXPECT_TRUE(userPreferences->hasValue("general/autoSave"));
    EXPECT_TRUE(userPreferences->hasValue("interface/theme"));
    EXPECT_TRUE(userPreferences->hasValue("rendering/pointSize"));
    
    // Test category filtering
    auto generalPrefs = userPreferences->getPreferencesByCategory(UserPreferences::Category::General);
    auto interfacePrefs = userPreferences->getPreferencesByCategory(UserPreferences::Category::Interface);
    
    EXPECT_GT(generalPrefs.size(), 0);
    EXPECT_GT(interfacePrefs.size(), 0);
    
    // Verify all general preferences are actually in the General category
    for (const auto& pref : generalPrefs) {
        EXPECT_EQ(pref.category, UserPreferences::Category::General);
    }
}

TEST_F(UIEnhancementTest, UserPreferencesResetFunctionality) {
    // Modify some default values
    userPreferences->setBool("general/autoSave", false);
    userPreferences->setString("interface/theme", "Dark");
    
    // Verify modifications
    EXPECT_FALSE(userPreferences->getBool("general/autoSave"));
    EXPECT_EQ(userPreferences->getString("interface/theme"), "Dark");
    
    // Reset individual preference
    userPreferences->resetToDefault("general/autoSave");
    EXPECT_TRUE(userPreferences->getBool("general/autoSave")); // Should be back to default (true)
    
    // Reset all preferences
    QSignalSpy resetSpy(userPreferences, &UserPreferences::settingsReset);
    userPreferences->resetAllToDefaults();
    
    EXPECT_EQ(resetSpy.count(), 1);
    EXPECT_EQ(userPreferences->getString("interface/theme"), "Light"); // Should be back to default
}
