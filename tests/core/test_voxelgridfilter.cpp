#include <gtest/gtest.h>
#include "core/voxelgridfilter.h"
#include "core/loadingsettings.h"
#include <chrono>

class VoxelGridFilterTest : public ::testing::Test {
protected:
    void SetUp() override {
        filter = std::make_unique<VoxelGridFilter>();
    }

    void TearDown() override {
        filter.reset();
    }

    std::unique_ptr<VoxelGridFilter> filter;
};

// Test Case 1: Empty input vector
TEST_F(VoxelGridFilterTest, EmptyInput) {
    std::vector<float> input;
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    EXPECT_TRUE(result.empty());
}

// Test Case 2: Single point input
TEST_F(VoxelGridFilterTest, SinglePoint) {
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    ASSERT_EQ(result.size(), 3);
    EXPECT_FLOAT_EQ(result[0], 1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_FLOAT_EQ(result[2], 3.0f);
}

// Test Case 3: Points in same voxel should be merged
TEST_F(VoxelGridFilterTest, PointsInSameVoxel) {
    // Three points very close together (within same voxel)
    std::vector<float> input = {
        0.0f, 0.0f, 0.0f,    // Point 1
        0.01f, 0.01f, 0.01f, // Point 2 (very close)
        0.02f, 0.02f, 0.02f  // Point 3 (very close)
    };

    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;  // Large enough to contain all points
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    // Should result in one point (centroid)
    ASSERT_EQ(result.size(), 3);

    // Centroid should be approximately (0.01, 0.01, 0.01)
    EXPECT_NEAR(result[0], 0.01f, 0.001f);
    EXPECT_NEAR(result[1], 0.01f, 0.001f);
    EXPECT_NEAR(result[2], 0.01f, 0.001f);
}

// Test Case 4: Points in different voxels
TEST_F(VoxelGridFilterTest, PointsInDifferentVoxels) {
    std::vector<float> input = {
        0.0f, 0.0f, 0.0f,    // Point 1 in voxel (0,0,0)
        1.0f, 1.0f, 1.0f     // Point 2 in different voxel
    };

    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.5;  // Small enough to separate points
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    // Should result in two points
    ASSERT_EQ(result.size(), 6);
}

// Test Case 5: Min points per voxel filtering
TEST_F(VoxelGridFilterTest, MinPointsPerVoxelFiltering) {
    std::vector<float> input = {
        // Voxel 1: 3 points
        0.0f, 0.0f, 0.0f,
        0.01f, 0.01f, 0.01f,
        0.02f, 0.02f, 0.02f,

        // Voxel 2: 1 point (should be filtered out)
        2.0f, 2.0f, 2.0f
    };

    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 2;  // Require at least 2 points

    std::vector<float> result = filter->filter(input, settings);

    // Should result in one point (only voxel with 3 points survives)
    ASSERT_EQ(result.size(), 3);
}

// Test Case 6: Invalid leaf size handling
TEST_F(VoxelGridFilterTest, InvalidLeafSize) {
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.0;  // Invalid
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    // Should still work with default leaf size
    ASSERT_EQ(result.size(), 3);
}

// Test Case 7: Invalid input size (not divisible by 3)
TEST_F(VoxelGridFilterTest, InvalidInputSize) {
    std::vector<float> input = {1.0f, 2.0f};  // Only 2 elements, not divisible by 3
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 1;

    std::vector<float> result = filter->filter(input, settings);

    EXPECT_TRUE(result.empty());
}

// Test Case 8: Large point cloud performance test
TEST_F(VoxelGridFilterTest, LargePointCloudPerformance) {
    // Create a larger point cloud for performance testing
    std::vector<float> input;
    const int numPoints = 10000;
    input.reserve(numPoints * 3);

    for (int i = 0; i < numPoints; ++i) {
        input.push_back(static_cast<float>(i % 100));      // X
        input.push_back(static_cast<float>((i / 100) % 100)); // Y
        input.push_back(static_cast<float>(i / 10000));    // Z
    }

    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 10.0;  // Larger leaf size to ensure reduction
    settings.parameters["minPointsPerVoxel"] = 1;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<float> result = filter->filter(input, settings);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete in reasonable time (less than 1 second for 10k points)
    EXPECT_LT(duration.count(), 1000);

    // Should significantly reduce point count
    EXPECT_LT(result.size(), input.size());
    EXPECT_GT(result.size(), 0);
}
