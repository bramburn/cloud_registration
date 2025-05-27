## How to Implement the Sprint 1 Action List

The action list you provided outlines a clear, modular, and testable plan for Sprint 1 of your FARO Scene-like registration software MVP using Qt6 and C++. Below is a step-by-step guide to implementing these actions, structured by feature and file, with key code and architectural advice.

---

## 1. **Settings Data Model**

**File:** `src/loadingsettings.h`

- **Define the loading method enum and settings struct:**

```cpp
#ifndef LOADINGSETTINGS_H
#define LOADINGSETTINGS_H

#include 

enum class LoadingMethod {
    FullLoad,
    HeaderOnly
};

struct LoadingSettings {
    LoadingMethod method = LoadingMethod::FullLoad;
    QVariantMap parameters;
};

#endif // LOADINGSETTINGS_H
```
This struct will be used throughout the dialog, parser, and main window to represent the user’s choice[1].

---

## 2. **Settings Dialog UI and Logic**

**Files:**  
- `src/loadingsettingsdialog.h`  
- `src/loadingsettingsdialog.cpp`

**Key Steps:**
- Subclass `QDialog` as `LoadingSettingsDialog`.
- Add a `QComboBox` for method selection ("Full Load", "Header-Only").
- Add `QPushButton`s for "Apply", "Cancel", and "OK".
- Use `QSettings` to persist user choices.
- Provide a public `getSettings()` method.

**Example Header:**
```cpp
#ifndef LOADINGSETTINGSDIALOG_H
#define LOADINGSETTINGSDIALOG_H

#include 
#include 
#include 
#include 
#include 
#include "loadingsettings.h"

class LoadingSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingSettingsDialog(QWidget *parent = nullptr);
    ~LoadingSettingsDialog();
    LoadingSettings getSettings() const;

private slots:
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    void onMethodChanged(int index);

private:
    void loadSettings();
    void saveSettings();
    void updateUIForMethod(LoadingMethod method);

    QComboBox *m_methodComboBox;
    QPushButton *m_applyButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QVBoxLayout *m_mainLayout;
    LoadingSettings m_currentSettings;
    QSettings m_qSettings;
};

#endif // LOADINGSETTINGSDIALOG_H
```
**Implementation Notes:**
- In the constructor, set up the UI and connect signals/slots.
- On dialog open, call `loadSettings()` to initialize the combo box.
- On Apply/OK, call `saveSettings()` to persist via `QSettings`[1].

---

## 3. **Main Window Integration**

**Files:**  
- `src/mainwindow.h`  
- `src/mainwindow.cpp`

**Key Steps:**
- Add a `QAction` for "Loading Settings..." to the "File" menu.
- Connect its `triggered()` signal to a slot (e.g., `onLoadingSettingsTriggered()`).
- In the slot, instantiate and show `LoadingSettingsDialog` modally.
- In your file open logic (e.g., `onOpenFileClicked`), always retrieve the latest `LoadingSettings` from `QSettings` and pass to the parser[1].

**Example Integration:**
```cpp
// In mainwindow.h
private slots:
    void onLoadingSettingsTriggered();
    void onLasHeaderParsed(const LasHeaderMetadata& metadata);

// In mainwindow.cpp
void MainWindow::onLoadingSettingsTriggered() {
    LoadingSettingsDialog dlg(this);
    dlg.exec();
}
```
---

## 4. **LAS Parser Refactoring**

**Files:**  
- `src/lasparser.h`  
- `src/lasparser.cpp`  
- `src/lasheadermetadata.h` (new)

**Key Steps:**
- Update `LasParser::parse()` to take `const LoadingSettings&`.
- Implement logic: If `HeaderOnly`, only read the header, populate metadata, emit a signal, and return an empty vector. If `FullLoad`, proceed as before.
- Add private members for header metadata.
- Define and emit a `headerParsed(const LasHeaderMetadata&)` signal after header read.

**Header Metadata Struct Example:**
```cpp
#ifndef LASHEADERMETADATA_H
#define LASHEADERMETADATA_H

#include 
#include 
#include 

struct LasHeaderMetadata {
    uint32_t numberOfPointRecords = 0;
    QVector3D minBounds;
    QVector3D maxBounds;
    QString filePath;
};

#endif // LASHEADERMETADATA_H
```
- Use this struct in your signal and to update the UI[1].

---

## 5. **Status Bar Metadata Display**

- In `MainWindow`, connect `LasParser`'s new `headerParsed` signal to a slot (e.g., `onLasHeaderParsed`).
- In this slot, update the `QStatusBar` with the file name, point count, and bounding box as soon as a header is parsed in "Header-Only" mode[1].

---

## 6. **Persistence with QSettings**

- In `LoadingSettingsDialog`, use `QSettings` to save the user’s choice on Apply/OK and load it in the constructor.
- In `MainWindow`, always read the current settings from `QSettings` before file operations, so user preferences persist across sessions[1].

---

## 7. **Testing**

- Manual UI testing: Verify dialog opens, buttons work, and settings persist.
- Unit tests (with Google Test):  
  - Check that `LasParser::parse()` returns an empty vector in "Header-Only".
  - Check that `headerParsed` emits correct metadata.
  - Test `LoadingSettingsDialog` QSettings read/write[1].

---

## 8. **File Relationships and Modular Design**

- `src/loadingsettings.h` is included by dialog, main window, and parser.
- `src/loadingsettingsdialog.h/cpp` implement the dialog and settings logic.
- `src/lasparser.h/cpp` uses the settings and emits metadata.
- `src/lasheadermetadata.h` is used for passing header info between parser and UI.
- All new code should be well-commented and modular for future extension[1].

---

## Summary Table: Key Components

| File                           | Purpose                                      |
|--------------------------------|----------------------------------------------|
| src/loadingsettings.h          | Defines LoadingMethod enum and struct        |
| src/loadingsettingsdialog.h/cpp| Implements settings dialog UI and logic      |
| src/lasheadermetadata.h        | Metadata struct for LAS header info          |
| src/lasparser.h/cpp            | LAS parsing logic (header only/full)         |
| src/mainwindow.h/cpp           | Main UI, menu, status bar, dialog integration|

---

## Next Steps

- Scaffold each new file/class as described.
- Implement and test each feature incrementally.
- Use Google Test to verify all backend logic.
- Manually verify the UI and persistence.
- Ensure all acceptance criteria are met for Sprint 1[1].

This approach will give you a robust, extensible MVP foundation for your FARO Scene registration software using Qt6 and C++[1].

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/0e8591dd-4529-4172-8a70-dad9da9039b9/paste.txt

---
Answer from Perplexity: pplx.ai/share