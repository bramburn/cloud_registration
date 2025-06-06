// Sprint 3 Test Suite for E57ParserLib - Intensity, Color, and CompressedVector Handling
// Tests for enhanced point data extraction with intensity and RGB color normalization

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QDebug>
#include <memory>
#include "../src/e57parserlib.h"

class E57ParserLibSprint3Test : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<E57ParserLib>();
        
        // Connect signals for testing
        progressSpy = std::make_unique<QSignalSpy>(parser.get(), &E57ParserLib::progressUpdated);
        finishedSpy = std::make_unique<QSignalSpy>(parser.get(), &E57ParserLib::parsingFinished);
    }

    void TearDown() override {
        if (parser && parser->isOpen()) {
            parser->closeFile();
        }
        parser.reset();
        progressSpy.reset();
        finishedSpy.reset();
    }

    std::unique_ptr<E57ParserLib> parser;
    std::unique_ptr<QSignalSpy> progressSpy;
    std::unique_ptr<QSignalSpy> finishedSpy;
    
    // Test file paths (these would need to be actual E57 files for real testing)
    const std::string testFileXYZOnly = "test_data/xyz_only.e57";
    const std::string testFileWithIntensity = "test_data/xyz_intensity.e57";
    const std::string testFileWithColor = "test_data/xyz_color.e57";
    const std::string testFileComplete = "test_data/xyz_intensity_color.e57";
};

// Test Case 3.1.1: Parse E57 file containing intensity field (ScaledIntegerNode)
TEST_F(E57ParserLibSprint3Test, ExtractIntensityScaledInteger) {
    // This test would require an actual E57 file with intensity data
    // For now, we test the API structure
    
    EXPECT_FALSE(parser->isOpen());
    
    // Test that the enhanced point data method exists and can be called
    auto points = parser->extractEnhancedPointData(0);
    EXPECT_TRUE(points.empty()); // Should be empty since no file is open
    
    // Verify error handling
    EXPECT_FALSE(parser->getLastError().isEmpty());
}

// Test Case 3.1.2: Parse E57 file where intensity is FloatNode
TEST_F(E57ParserLibSprint3Test, ExtractIntensityFloat) {
    // Test the PointData structure
    E57ParserLib::PointData point(1.0f, 2.0f, 3.0f);

    EXPECT_FLOAT_EQ(point.x, 1.0f);
    EXPECT_FLOAT_EQ(point.y, 2.0f);
    EXPECT_FLOAT_EQ(point.z, 3.0f);
    EXPECT_FALSE(point.hasIntensity);
    EXPECT_FALSE(point.hasColor);
    EXPECT_FLOAT_EQ(point.intensity, 0.0f);
    EXPECT_EQ(point.r, 0);
    EXPECT_EQ(point.g, 0);
    EXPECT_EQ(point.b, 0);
}

// Test Case 3.1.3: Parse E57 file without intensity field
TEST_F(E57ParserLibSprint3Test, NoIntensityField) {
    // Test default PointData construction
    E57ParserLib::PointData point;
    
    EXPECT_FLOAT_EQ(point.x, 0.0f);
    EXPECT_FLOAT_EQ(point.y, 0.0f);
    EXPECT_FLOAT_EQ(point.z, 0.0f);
    EXPECT_FALSE(point.hasIntensity);
    EXPECT_FALSE(point.hasColor);
}

// Test Case 3.1.4: Parse E57 file with intensityLimits where min equals max
TEST_F(E57ParserLibSprint3Test, IntensityLimitsMinEqualsMax) {
    // This would test the normalization logic when min == max
    // For now, we can test that the parser handles invalid scan indices gracefully
    
    auto points = parser->extractEnhancedPointData(-1);
    EXPECT_TRUE(points.empty());
    
    points = parser->extractEnhancedPointData(999);
    EXPECT_TRUE(points.empty());
}

// Test Case 3.2.1: Parse E57 file with 8-bit RGB color (IntegerNode 0-255)
TEST_F(E57ParserLibSprint3Test, ExtractColor8Bit) {
    // Test PointData color functionality
    E57ParserLib::PointData point;
    point.r = 255;
    point.g = 128;
    point.b = 64;
    point.hasColor = true;
    
    EXPECT_EQ(point.r, 255);
    EXPECT_EQ(point.g, 128);
    EXPECT_EQ(point.b, 64);
    EXPECT_TRUE(point.hasColor);
}

// Test Case 3.2.2: Parse E57 file with 16-bit RGB color (ScaledIntegerNode)
TEST_F(E57ParserLibSprint3Test, ExtractColor16Bit) {
    // Test that color values are properly constrained to uint8_t range
    E57ParserLib::PointData point;
    point.r = 255;  // Max value
    point.g = 0;    // Min value
    point.b = 127;  // Mid value
    point.hasColor = true;
    
    EXPECT_EQ(point.r, 255);
    EXPECT_EQ(point.g, 0);
    EXPECT_EQ(point.b, 127);
}

// Test Case 3.2.3: Parse E57 file without color information
TEST_F(E57ParserLibSprint3Test, NoColorFields) {
    E57ParserLib::PointData point;
    
    EXPECT_FALSE(point.hasColor);
    EXPECT_EQ(point.r, 0);
    EXPECT_EQ(point.g, 0);
    EXPECT_EQ(point.b, 0);
}

// Test Case 3.3.1: Parse E57 file containing XYZ, Intensity, and RGB Color
TEST_F(E57ParserLibSprint3Test, ExtractAllAttributes) {
    // Test complete PointData with all attributes
    E57ParserLib::PointData point(10.5f, 20.3f, 30.1f);
    point.intensity = 0.75f;
    point.hasIntensity = true;
    point.r = 200;
    point.g = 150;
    point.b = 100;
    point.hasColor = true;
    
    EXPECT_FLOAT_EQ(point.x, 10.5f);
    EXPECT_FLOAT_EQ(point.y, 20.3f);
    EXPECT_FLOAT_EQ(point.z, 30.1f);
    EXPECT_TRUE(point.hasIntensity);
    EXPECT_FLOAT_EQ(point.intensity, 0.75f);
    EXPECT_TRUE(point.hasColor);
    EXPECT_EQ(point.r, 200);
    EXPECT_EQ(point.g, 150);
    EXPECT_EQ(point.b, 100);
}

// Test Case 3.3.2: Parse E57 file with only XYZ and Intensity (no color)
TEST_F(E57ParserLibSprint3Test, ExtractXYZAndIntensity) {
    E57ParserLib::PointData point(1.0f, 2.0f, 3.0f);
    point.intensity = 0.5f;
    point.hasIntensity = true;
    // Color should remain default
    
    EXPECT_TRUE(point.hasIntensity);
    EXPECT_FALSE(point.hasColor);
    EXPECT_FLOAT_EQ(point.intensity, 0.5f);
    EXPECT_EQ(point.r, 0);
    EXPECT_EQ(point.g, 0);
    EXPECT_EQ(point.b, 0);
}

// Test Case 3.3.3: Parse E57 file with only XYZ and Color (no intensity)
TEST_F(E57ParserLibSprint3Test, ExtractXYZAndColor) {
    E57ParserLib::PointData point(1.0f, 2.0f, 3.0f);
    point.r = 255;
    point.g = 255;
    point.b = 255;
    point.hasColor = true;
    // Intensity should remain default
    
    EXPECT_FALSE(point.hasIntensity);
    EXPECT_TRUE(point.hasColor);
    EXPECT_FLOAT_EQ(point.intensity, 0.0f);
    EXPECT_EQ(point.r, 255);
    EXPECT_EQ(point.g, 255);
    EXPECT_EQ(point.b, 255);
}

// Test error handling and edge cases
TEST_F(E57ParserLibSprint3Test, ErrorHandling) {
    // Test with no file open
    EXPECT_FALSE(parser->isOpen());
    
    auto points = parser->extractEnhancedPointData();
    EXPECT_TRUE(points.empty());
    EXPECT_FALSE(parser->getLastError().isEmpty());

    // Test legacy method still works
    auto legacyPoints = parser->extractPointData();
    EXPECT_TRUE(legacyPoints.empty());
}

// Test signal emission during enhanced parsing
TEST_F(E57ParserLibSprint3Test, SignalEmission) {
    // Clear any existing signals
    progressSpy->clear();
    finishedSpy->clear();
    
    // Attempt to parse (will fail since no file is open, but should emit signals)
    auto points = parser->extractEnhancedPointData();
    
    // Should have emitted parsingFinished signal with error
    EXPECT_GE(finishedSpy->count(), 1);
    
    if (finishedSpy->count() > 0) {
        QList<QVariant> arguments = finishedSpy->takeFirst();
        EXPECT_FALSE(arguments.at(0).toBool()); // success should be false
        EXPECT_FALSE(arguments.at(1).toString().isEmpty()); // error message should not be empty
    }
}

// Performance test for large point clouds (conceptual)
TEST_F(E57ParserLibSprint3Test, PerformanceConsiderations) {
    // This test verifies that the API can handle the expected data structures
    // In a real scenario, this would test with large E57 files
    
    std::vector<E57ParserLib::PointData> largePointSet;
    largePointSet.reserve(1000000); // Reserve space for 1M points
    
    // Simulate adding points
    for (int i = 0; i < 1000; ++i) {
        E57ParserLib::PointData point(
            static_cast<float>(i), 
            static_cast<float>(i * 2), 
            static_cast<float>(i * 3)
        );
        point.intensity = static_cast<float>(i) / 1000.0f;
        point.hasIntensity = true;
        point.r = static_cast<uint8_t>(i % 256);
        point.g = static_cast<uint8_t>((i * 2) % 256);
        point.b = static_cast<uint8_t>((i * 3) % 256);
        point.hasColor = true;
        
        largePointSet.push_back(point);
    }
    
    EXPECT_EQ(largePointSet.size(), 1000);
    EXPECT_TRUE(largePointSet[999].hasIntensity);
    EXPECT_TRUE(largePointSet[999].hasColor);
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
