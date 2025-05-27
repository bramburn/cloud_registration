#ifndef LOADINGSETTINGS_H
#define LOADINGSETTINGS_H

#include <QVariantMap>

// Enum to define the available point cloud loading methods
enum class LoadingMethod {
    FullLoad,    // Loads all points and attributes (current baseline)
    HeaderOnly   // Reads only the file header for metadata
    // Future methods will be added here (e.g., VoxelGrid, PoissonDisk)
};

// Struct to hold the selected loading method and its parameters
struct LoadingSettings {
    LoadingMethod method = LoadingMethod::FullLoad; // Default to full load
    QVariantMap parameters; // A map to store key-value pairs for method-specific settings
                           // (e.g., "leafSize" for VoxelGrid, "keepPercentage" for Random)
};

#endif // LOADINGSETTINGS_H
