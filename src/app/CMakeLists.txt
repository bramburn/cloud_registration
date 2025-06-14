# Application module CMakeLists.txt

# Define the executable
set(APP_SOURCES
    main.cpp
    MainWindow.cpp
)

set(APP_HEADERS
    MainWindow.h
)

# Create the executable
add_executable(PointCloudApplication ${APP_SOURCES} ${APP_HEADERS})

# Link Qt libraries
target_link_libraries(PointCloudApplication 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Svg
)

# Set target properties
set_target_properties(PointCloudApplication PROPERTIES
    WIN32_EXECUTABLE TRUE
)
