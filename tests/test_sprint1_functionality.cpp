#include <gtest/gtest.h>
#include <QApplication>
#include <QSettings>
#include <QSignalSpy>
#include "../src/loadingsettings.h"
#include "../src/loadingsettingsdialog.h"
#include "../src/lasparser.h"
#include "../src/lasheadermetadata.h"

class Sprint1Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Clear any existing settings for clean test environment
        QSettings settings("CloudRegistration", "PointCloudViewer");
        settings.clear();
    }

    void TearDown() override {
        // Clean up settings after test
        QSettings settings("CloudRegistration", "PointCloudViewer");
        settings.clear();
    }
};

// Test LoadingSettings structure
TEST_F(Sprint1Test, LoadingSettingsStructure) {
    LoadingSettings settings;
    
    // Test default values
    EXPECT_EQ(settings.method, LoadingMethod::FullLoad);
    EXPECT_TRUE(settings.parameters.isEmpty());
    
    // Test setting values
    settings.method = LoadingMethod::HeaderOnly;
    EXPECT_EQ(settings.method, LoadingMethod::HeaderOnly);
}

// Test LoadingSettingsDialog basic functionality
TEST_F(Sprint1Test, LoadingSettingsDialogCreation) {
    // Create a QApplication if one doesn't exist
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    
    LoadingSettingsDialog dialog;
    
    // Test dialog properties
    EXPECT_EQ(dialog.windowTitle(), "Point Cloud Loading Settings");
    EXPECT_TRUE(dialog.isModal());
    
    // Test default settings
    LoadingSettings settings = dialog.getSettings();
    EXPECT_EQ(settings.method, LoadingMethod::FullLoad);
}

// Test settings persistence
TEST_F(Sprint1Test, SettingsPersistence) {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    
    // Save a setting
    {
        QSettings settings("CloudRegistration", "PointCloudViewer");
        settings.setValue("PointCloudLoading/DefaultMethod", 
                         static_cast<int>(LoadingMethod::HeaderOnly));
        settings.sync();
    }
    
    // Create dialog and verify it loads the saved setting
    LoadingSettingsDialog dialog;
    LoadingSettings loadedSettings = dialog.getSettings();
    EXPECT_EQ(loadedSettings.method, LoadingMethod::HeaderOnly);
}

// Test LasHeaderMetadata structure
TEST_F(Sprint1Test, LasHeaderMetadataStructure) {
    LasHeaderMetadata metadata;
    
    // Test default values
    EXPECT_EQ(metadata.numberOfPointRecords, 0);
    EXPECT_EQ(metadata.filePath, QString());
    
    // Test setting values
    metadata.numberOfPointRecords = 12345;
    metadata.filePath = "test.las";
    metadata.minBounds = QVector3D(-10.0f, -5.0f, 0.0f);
    metadata.maxBounds = QVector3D(10.0f, 5.0f, 20.0f);
    
    EXPECT_EQ(metadata.numberOfPointRecords, 12345);
    EXPECT_EQ(metadata.filePath, "test.las");
    EXPECT_EQ(metadata.minBounds, QVector3D(-10.0f, -5.0f, 0.0f));
    EXPECT_EQ(metadata.maxBounds, QVector3D(10.0f, 5.0f, 20.0f));
}

// Test LasParser signal emission (mock test)
TEST_F(Sprint1Test, LasParserSignalEmission) {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    
    LasParser parser;
    
    // Test that signals are properly declared
    QSignalSpy progressSpy(&parser, &LasParser::progressUpdated);
    QSignalSpy finishedSpy(&parser, &LasParser::parsingFinished);
    QSignalSpy headerSpy(&parser, &LasParser::headerParsed);
    
    // Verify signal spies are valid (signals exist)
    EXPECT_TRUE(progressSpy.isValid());
    EXPECT_TRUE(finishedSpy.isValid());
    EXPECT_TRUE(headerSpy.isValid());
}

// Test LoadingMethod enum values
TEST_F(Sprint1Test, LoadingMethodEnum) {
    // Test enum values are correctly defined
    EXPECT_EQ(static_cast<int>(LoadingMethod::FullLoad), 0);
    EXPECT_EQ(static_cast<int>(LoadingMethod::HeaderOnly), 1);
    
    // Test enum can be cast to/from int for QSettings
    LoadingMethod method = LoadingMethod::HeaderOnly;
    int methodInt = static_cast<int>(method);
    LoadingMethod methodBack = static_cast<LoadingMethod>(methodInt);
    EXPECT_EQ(method, methodBack);
}

// Integration test for settings workflow
TEST_F(Sprint1Test, SettingsWorkflow) {
    int argc = 0;
    char* argv[] = {nullptr};
    QApplication app(argc, argv);
    
    // Step 1: Create dialog with default settings
    LoadingSettingsDialog dialog1;
    EXPECT_EQ(dialog1.getSettings().method, LoadingMethod::FullLoad);
    
    // Step 2: Simulate changing setting and saving
    QSettings settings("CloudRegistration", "PointCloudViewer");
    settings.setValue("PointCloudLoading/DefaultMethod", 
                     static_cast<int>(LoadingMethod::HeaderOnly));
    settings.sync();
    
    // Step 3: Create new dialog and verify it loads the saved setting
    LoadingSettingsDialog dialog2;
    EXPECT_EQ(dialog2.getSettings().method, LoadingMethod::HeaderOnly);
    
    // Step 4: Verify MainWindow would read the same setting
    int methodValue = settings.value("PointCloudLoading/DefaultMethod",
                                    static_cast<int>(LoadingMethod::FullLoad)).toInt();
    LoadingMethod loadedMethod = static_cast<LoadingMethod>(methodValue);
    EXPECT_EQ(loadedMethod, LoadingMethod::HeaderOnly);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
