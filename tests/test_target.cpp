#include <gtest/gtest.h>
#include <QVector3D>
#include <QVariantMap>
#include "registration/Target.h"
#include "registration/TargetCorrespondence.h"

class TargetTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        testPosition = QVector3D(1.0f, 2.0f, 3.0f);
        testId = "test_target_001";
    }

    QVector3D testPosition;
    QString testId;
};

// Test SphereTarget creation and basic functionality
TEST_F(TargetTest, SphereTargetCreation) {
    float testRadius = 0.15f;
    SphereTarget sphere(testId, testPosition, testRadius);

    EXPECT_EQ(sphere.targetId(), testId);
    EXPECT_EQ(sphere.position(), testPosition);
    EXPECT_FLOAT_EQ(sphere.radius(), testRadius);
    EXPECT_EQ(sphere.getType(), "Sphere");
    EXPECT_TRUE(sphere.isValid());
    EXPECT_FLOAT_EQ(sphere.confidence(), 1.0f);  // Default confidence
}

// Test SphereTarget serialization
TEST_F(TargetTest, SphereTargetSerialization) {
    float testRadius = 0.25f;
    SphereTarget sphere(testId, testPosition, testRadius);
    sphere.setConfidence(0.85f);
    sphere.setRmsError(0.002f);
    sphere.setInlierCount(150);
    
    // Serialize
    QVariantMap data = sphere.serialize();
    
    // Check serialized data
    EXPECT_EQ(data["targetId"].toString(), testId);
    EXPECT_EQ(data["type"].toString(), "Sphere");
    EXPECT_FLOAT_EQ(data["radius"].toFloat(), testRadius);
    EXPECT_FLOAT_EQ(data["confidence"].toFloat(), 0.85f);
    EXPECT_FLOAT_EQ(data["rmsError"].toFloat(), 0.002f);
    EXPECT_EQ(data["inlierCount"].toInt(), 150);
    
    // Test position serialization
    QVariantList posList = data["position"].toList();
    EXPECT_EQ(posList.size(), 3);
    EXPECT_FLOAT_EQ(posList[0].toFloat(), testPosition.x());
    EXPECT_FLOAT_EQ(posList[1].toFloat(), testPosition.y());
    EXPECT_FLOAT_EQ(posList[2].toFloat(), testPosition.z());
}

// Test SphereTarget deserialization
TEST_F(TargetTest, SphereTargetDeserialization) {
    // Create test data
    QVariantMap data;
    data["targetId"] = testId;
    data["type"] = "Sphere";
    data["position"] = QVariantList{testPosition.x(), testPosition.y(), testPosition.z()};
    data["confidence"] = 0.75f;
    data["isValid"] = true;
    data["radius"] = 0.3f;
    data["rmsError"] = 0.001f;
    data["inlierCount"] = 200;
    
    // Create sphere and deserialize
    SphereTarget sphere("", QVector3D(), 0.0f);
    bool success = sphere.deserialize(data);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(sphere.targetId(), testId);
    EXPECT_EQ(sphere.position(), testPosition);
    EXPECT_FLOAT_EQ(sphere.radius(), 0.3f);
    EXPECT_FLOAT_EQ(sphere.confidence(), 0.75f);
    EXPECT_FLOAT_EQ(sphere.rmsError(), 0.001f);
    EXPECT_EQ(sphere.inlierCount(), 200);
}

// Test NaturalPointTarget creation and functionality
TEST_F(TargetTest, NaturalPointTargetCreation) {
    QString description = "Building corner point";
    NaturalPointTarget naturalPoint(testId, testPosition, description);

    EXPECT_EQ(naturalPoint.targetId(), testId);
    EXPECT_EQ(naturalPoint.position(), testPosition);
    EXPECT_EQ(naturalPoint.description(), description);
    EXPECT_EQ(naturalPoint.getType(), "NaturalPoint");
    EXPECT_TRUE(naturalPoint.isValid());
}

// Test NaturalPointTarget serialization
TEST_F(TargetTest, NaturalPointTargetSerialization) {
    QString description = "Rock formation edge";
    QList<float> featureDescriptor = {0.8f, 0.1f, 0.1f};  // High planarity

    NaturalPointTarget naturalPoint(testId, testPosition, description);
    naturalPoint.setFeatureDescriptor(featureDescriptor);
    naturalPoint.setConfidence(0.9f);
    
    // Serialize
    QVariantMap data = naturalPoint.serialize();
    
    // Check serialized data
    EXPECT_EQ(data["targetId"].toString(), testId);
    EXPECT_EQ(data["type"].toString(), "NaturalPoint");
    EXPECT_EQ(data["description"].toString(), description);
    EXPECT_FLOAT_EQ(data["confidence"].toFloat(), 0.9f);

    // Check feature descriptor
    QVariantList featureList = data["featureDescriptor"].toList();
    EXPECT_EQ(featureList.size(), 3);
    EXPECT_FLOAT_EQ(featureList[0].toFloat(), featureDescriptor[0]);
    EXPECT_FLOAT_EQ(featureList[1].toFloat(), featureDescriptor[1]);
    EXPECT_FLOAT_EQ(featureList[2].toFloat(), featureDescriptor[2]);
}

// Test CheckerboardTarget creation and functionality
TEST_F(TargetTest, CheckerboardTargetCreation) {
    QList<QVector3D> corners = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(0.1f, 0.0f, 0.0f),
        QVector3D(0.0f, 0.1f, 0.0f),
        QVector3D(0.1f, 0.1f, 0.0f)
    };
    
    CheckerboardTarget checkerboard(testId, testPosition, corners);

    EXPECT_EQ(checkerboard.targetId(), testId);
    EXPECT_EQ(checkerboard.position(), testPosition);
    EXPECT_EQ(checkerboard.cornerPoints().size(), 4);
    EXPECT_EQ(checkerboard.getType(), "Checkerboard");
    EXPECT_TRUE(checkerboard.isValid());
}

// Test TargetCorrespondence functionality
TEST_F(TargetTest, TargetCorrespondence) {
    QString targetId1 = "target_001";
    QString targetId2 = "target_002";
    QString scanId1 = "scan_001";
    QString scanId2 = "scan_002";

    TargetCorrespondence correspondence(targetId1, targetId2, scanId1, scanId2);

    EXPECT_EQ(correspondence.targetId1(), targetId1);
    EXPECT_EQ(correspondence.targetId2(), targetId2);
    EXPECT_EQ(correspondence.scanId1(), scanId1);
    EXPECT_EQ(correspondence.scanId2(), scanId2);
    EXPECT_FLOAT_EQ(correspondence.confidence(), 1.0f);  // Default confidence
    EXPECT_FLOAT_EQ(correspondence.distance(), 0.0f);    // Default distance
    EXPECT_TRUE(correspondence.isValid());
}

// Test invalid target scenarios
TEST_F(TargetTest, InvalidTargetScenarios) {
    // Test empty ID
    SphereTarget invalidSphere("", testPosition, 0.1f);
    invalidSphere.setValid(false);
    EXPECT_FALSE(invalidSphere.isValid());
    
    // Test invalid correspondence
    TargetCorrespondence invalidCorr("", "target_002", "scan_001", "scan_002");
    EXPECT_FALSE(invalidCorr.isValid());
    
    // Test low confidence correspondence
    TargetCorrespondence lowConfCorr("target_001", "target_002", "scan_001", "scan_002");
    lowConfCorr.setConfidence(0.3f);  // Below 0.5 threshold
    EXPECT_FALSE(lowConfCorr.isValid());
}

// Test target confidence settings
TEST_F(TargetTest, TargetConfidenceSettings) {
    SphereTarget sphere(testId, testPosition, 0.1f);

    // Test valid confidence range
    sphere.setConfidence(0.85f);
    EXPECT_FLOAT_EQ(sphere.confidence(), 0.85f);

    // Test boundary values
    sphere.setConfidence(0.0f);
    EXPECT_FLOAT_EQ(sphere.confidence(), 0.0f);

    sphere.setConfidence(1.0f);
    EXPECT_FLOAT_EQ(sphere.confidence(), 1.0f);
}

// Test target position updates
TEST_F(TargetTest, TargetPositionUpdates) {
    SphereTarget sphere(testId, testPosition, 0.1f);

    QVector3D newPosition(5.0f, 6.0f, 7.0f);
    sphere.setPosition(newPosition);

    EXPECT_EQ(sphere.position(), newPosition);
    EXPECT_NE(sphere.position(), testPosition);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
