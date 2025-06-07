#ifndef SCREENSPACEERROR_H
#define SCREENSPACEERROR_H

/**
 * @brief ScreenSpaceError - Utility class for screen space error calculations
 * 
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class ScreenSpaceError {
public:
    static float calculateError(float distance, float pointSize, float screenWidth);
    static bool shouldCull(float error, float threshold);

private:
    ScreenSpaceError() = default;
};

#endif // SCREENSPACEERROR_H
