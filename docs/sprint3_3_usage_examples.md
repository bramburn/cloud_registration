# Sprint 3.3 Usage Examples

## IconManager Usage

### Basic Icon Retrieval
```cpp
#include "iconmanager.h"

// Get a basic scan icon
QIcon scanIcon = IconManager::instance().getIcon(ItemType::Scan, ItemState::Unloaded);

// Get a loaded cluster icon
QIcon loadedClusterIcon = IconManager::instance().getIcon(ItemType::Cluster, ItemState::Loaded);

// Get a composite icon with import type badge
QIcon copiedScanIcon = IconManager::instance().getCompositeIcon(
    ItemType::Scan, 
    ItemState::Loaded, 
    ImportType::Copy
);
```

### Using Icons in UI Components
```cpp
// In a tree view or list widget
QStandardItem* item = new QStandardItem("My Scan");
item->setIcon(IconManager::instance().getIcon(ItemType::Scan, ItemState::Missing));

// In a button
QPushButton* button = new QPushButton("Load Scan");
button->setIcon(IconManager::instance().getIcon(ItemType::Scan, ItemState::Loading));
```

### Theme Management
```cpp
// Switch to dark theme
IconManager::instance().setTheme("dark");

// Preload icons for better performance
IconManager::instance().preloadIcons();

// Clear cache when needed
IconManager::instance().clearCache();
```

## ProgressManager Usage

### Starting and Managing Operations
```cpp
#include "progressmanager.h"

// Start a scan import operation
QString operationId = ProgressManager::instance().startOperation(
    OperationType::ScanImport,
    "Importing LAS files",
    100,  // max steps
    true  // cancellable
);

// Update progress
ProgressManager::instance().updateProgress(
    operationId,
    25,  // current value
    "Processing file 1 of 4",  // step description
    "Reading point data from scan001.las"  // detailed status
);

// Finish operation
ProgressManager::instance().finishOperation(operationId, "Import completed successfully");
```

### Connecting to Progress Signals
```cpp
// In your UI class constructor
auto& progressManager = ProgressManager::instance();

connect(&progressManager, &ProgressManager::operationStarted,
        this, &MyWidget::onOperationStarted);
connect(&progressManager, &ProgressManager::progressUpdated,
        this, &MyWidget::onProgressUpdated);
connect(&progressManager, &ProgressManager::operationFinished,
        this, &MyWidget::onOperationFinished);

// Slot implementations
void MyWidget::onOperationStarted(const QString& operationId, const QString& name, OperationType type) {
    statusLabel->setText(QString("Started: %1").arg(name));
    progressBar->setVisible(true);
}

void MyWidget::onProgressUpdated(const QString& operationId, int value, int max, 
                                const QString& step, const QString& details) {
    progressBar->setMaximum(max);
    progressBar->setValue(value);
    statusLabel->setText(step);
    progressBar->setToolTip(details);
}
```

### Cancellation Support
```cpp
// Check if operation can be cancelled
ProgressInfo info = ProgressManager::instance().getProgressInfo(operationId);
if (info.isCancellable) {
    cancelButton->setEnabled(true);
}

// Cancel operation
connect(cancelButton, &QPushButton::clicked, [operationId]() {
    ProgressManager::instance().cancelOperation(operationId);
});

// Check if operation was cancelled (in worker thread)
if (ProgressManager::instance().isOperationCancelled(operationId)) {
    // Stop processing and clean up
    return;
}
```

## Enhanced ProjectTreeModel Usage

### Setting Up the Model with Enhanced Features
```cpp
#include "projecttreemodel.h"

ProjectTreeModel* model = new ProjectTreeModel(this);
model->setSqliteManager(sqliteManager);

// Set up tree view
QTreeView* treeView = new QTreeView();
treeView->setModel(model);

// Enable tooltips
treeView->setMouseTracking(true);
```

### Accessing Enhanced Data
```cpp
// Get item type
QModelIndex index = treeView->currentIndex();
int itemType = model->data(index, ProjectTreeModel::ItemTypeRole).toInt();

// Get point count
qint64 pointCount = model->data(index, ProjectTreeModel::PointCountRole).toLongLong();

// Get file size
qint64 fileSize = model->data(index, ProjectTreeModel::FileSizeRole).toLongLong();

// Check if item is loaded
bool isLoaded = model->data(index, ProjectTreeModel::IsLoadedRole).toBool();

// Get full file path
QString fullPath = model->data(index, ProjectTreeModel::FullPathRole).toString();
```

## MainWindow Status Bar Integration

### Setting Up Progress Display
```cpp
// The MainWindow automatically connects to ProgressManager
// Progress will be displayed in the status bar when operations start

// You can also manually update status
statusBar()->showMessage("Ready", 2000);

// Access progress widgets if needed
if (m_progressBar->isVisible()) {
    // An operation is currently running
}
```

## Integration Example: Scan Import with Progress

```cpp
class ScanImportManager : public QObject {
    Q_OBJECT

public slots:
    void importScans(const QStringList& filePaths) {
        // Start progress tracking
        QString operationId = ProgressManager::instance().startOperation(
            OperationType::ScanImport,
            QString("Importing %1 scans").arg(filePaths.size()),
            filePaths.size(),
            true
        );

        // Process files
        for (int i = 0; i < filePaths.size(); ++i) {
            // Check for cancellation
            if (ProgressManager::instance().isOperationCancelled(operationId)) {
                return;
            }

            const QString& filePath = filePaths[i];
            
            // Update progress
            ProgressManager::instance().updateProgress(
                operationId,
                i,
                QString("Processing %1").arg(QFileInfo(filePath).fileName()),
                QString("Importing file %1 of %2").arg(i + 1).arg(filePaths.size())
            );

            // Do actual import work
            importSingleScan(filePath);
        }

        // Finish operation
        ProgressManager::instance().finishOperation(
            operationId,
            QString("Successfully imported %1 scans").arg(filePaths.size())
        );

        // Update tree model to show new scans with appropriate icons
        emit scansImported();
    }

private:
    void importSingleScan(const QString& filePath) {
        // Implementation details...
    }

signals:
    void scansImported();
};
```

## Testing Examples

### Unit Testing IconManager
```cpp
TEST(IconManagerTest, BasicFunctionality) {
    IconManager& manager = IconManager::instance();
    
    QIcon icon = manager.getIcon(ItemType::Scan, ItemState::Loaded);
    EXPECT_FALSE(icon.isNull());
    
    QIcon compositeIcon = manager.getCompositeIcon(
        ItemType::Scan, ItemState::Missing, ImportType::Copy
    );
    EXPECT_FALSE(compositeIcon.isNull());
}
```

### Unit Testing ProgressManager
```cpp
TEST(ProgressManagerTest, OperationLifecycle) {
    ProgressManager& manager = ProgressManager::instance();
    
    QString opId = manager.startOperation(OperationType::ScanImport, "Test", 100);
    EXPECT_FALSE(opId.isEmpty());
    
    manager.updateProgress(opId, 50);
    ProgressInfo info = manager.getProgressInfo(opId);
    EXPECT_EQ(info.currentValue, 50);
    
    manager.finishOperation(opId);
}
```

## Best Practices

### Performance Tips
1. **Preload Icons:** Call `IconManager::instance().preloadIcons()` at startup
2. **Cache Progress Info:** Store ProgressInfo locally if accessing frequently
3. **Batch Updates:** Update progress in reasonable intervals (not every iteration)

### UI/UX Guidelines
1. **Consistent Icons:** Use the same icon types throughout the application
2. **Meaningful Progress:** Provide descriptive step names and details
3. **Cancellation:** Always provide cancellation for long operations
4. **Feedback:** Use tooltips to provide additional context

### Error Handling
1. **Check Operation Validity:** Always verify operation IDs before updates
2. **Handle Missing Icons:** IconManager provides fallbacks automatically
3. **Progress Validation:** Ensure progress values are within valid ranges
