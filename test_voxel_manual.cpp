#include "src/voxelgridfilter.h"
#include "src/loadingsettings.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Testing VoxelGridFilter..." << std::endl;
    
    // Create test data
    std::vector<float> input = {
        0.0f, 0.0f, 0.0f,    // Point 1
        0.01f, 0.01f, 0.01f, // Point 2 (close to point 1)
        1.0f, 1.0f, 1.0f     // Point 3 (far from others)
    };
    
    // Create settings
    LoadingSettings settings;
    settings.method = LoadingMethod::VoxelGrid;
    settings.parameters["leafSize"] = 0.1;
    settings.parameters["minPointsPerVoxel"] = 1;
    
    // Create filter and apply
    VoxelGridFilter filter;
    std::vector<float> result = filter.filter(input, settings);
    
    std::cout << "Input points: " << input.size() / 3 << std::endl;
    std::cout << "Output points: " << result.size() / 3 << std::endl;
    
    std::cout << "Output coordinates:" << std::endl;
    for (size_t i = 0; i < result.size(); i += 3) {
        std::cout << "  (" << result[i] << ", " << result[i+1] << ", " << result[i+2] << ")" << std::endl;
    }
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}
