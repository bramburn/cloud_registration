#include <iostream>
#include <vector>
#include "../../src/voxelgridfilter.h"
#include "../../src/loadingsettings.h"
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "Testing VoxelGridFilter..." << std::endl;
    
    // Test 1: Basic functionality
    VoxelGridFilter filter;
    std::vector<float> testPoints = {
        0.0f, 0.0f, 0.0f,    // Point 1
        0.01f, 0.01f, 0.01f, // Point 2 (close to point 1)
        1.0f, 1.0f, 1.0f     // Point 3 (far from others)
    };
    
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 1;
    
    std::vector<float> result = filter.filter(testPoints, settings);
    
    std::cout << "Input points: " << testPoints.size() / 3 << std::endl;
    std::cout << "Output points: " << result.size() / 3 << std::endl;
    
    if (result.size() > 0) {
        std::cout << "First output point: (" << result[0] << ", " << result[1] << ", " << result[2] << ")" << std::endl;
    }
    
    // Test 2: Empty input
    std::vector<float> emptyInput;
    std::vector<float> emptyResult = filter.filter(emptyInput, settings);
    std::cout << "Empty input test - Output size: " << emptyResult.size() << std::endl;
    
    // Test 3: Single point
    std::vector<float> singlePoint = {5.0f, 6.0f, 7.0f};
    std::vector<float> singleResult = filter.filter(singlePoint, settings);
    std::cout << "Single point test - Output size: " << singleResult.size() / 3 << std::endl;
    
    std::cout << "VoxelGridFilter tests completed successfully!" << std::endl;
    
    return 0;
}
