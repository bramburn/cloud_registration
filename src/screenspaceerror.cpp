#include "screenspaceerror.h"
#include <cmath>

float ScreenSpaceError::calculateError(float distance, float pointSize, float screenWidth)
{
    // Stub implementation - basic screen space error calculation
    if (distance <= 0.0f || screenWidth <= 0.0f) {
        return 0.0f;
    }
    
    return pointSize / (distance * screenWidth);
}

bool ScreenSpaceError::shouldCull(float error, float threshold)
{
    return error < threshold;
}
