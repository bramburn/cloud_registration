Here's a step-by-step implementation guide for Sprint 1 using libE57Format with Qt6 and vcpkg:

### 1. Project Setup with vcpkg
**vcpkg.json**:
```json
{
  "name": "faroscene-registration",
  "version-string": "0.1.0",
  "dependencies": [
    "libe57format",
    "xerces-c"
  ]
}
```

**CMakeLists.txt** (critical sections):
```cmake
cmake_minimum_required(VERSION 3.15)
project(FaroSceneRegistration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(E57Format CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/e57parserlib.cpp
  src/mainwindow.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Qt6::Core
  E57Format
)

# Google Test setup
enable_testing()
add_subdirectory(tests)
```

### 2. Core Wrapper Class Implementation
**e57parserlib.h**:
```cpp
#include 
#include 

class E57ParserLib {
public:
    E57ParserLib();
    ~E57ParserLib();
    
    bool openFile(const std::string& filePath);
    void closeFile();
    
    std::string getGuid() const;
    std::pair getVersion() const;
    int getScanCount() const;
    std::string getLastError() const;

private:
    e57::ImageFile* m_imageFile = nullptr;
    std::string m_lastError;
};
```

**e57parserlib.cpp**:
```cpp
#include "e57parserlib.h"
#include 

E57ParserLib::E57ParserLib() = default;

E57ParserLib::~E57ParserLib() {
    closeFile();
}

bool E57ParserLib::openFile(const std::string& filePath) {
    try {
        closeFile();
        m_imageFile = new e57::ImageFile(filePath, "r");
        
        if(!m_imageFile->isOpen()) {
            m_lastError = "Failed to open file handle";
            return false;
        }
        return true;
    } catch (e57::E57Exception& ex) {
        std::stringstream ss;
        ex.report(ss);
        m_lastError = ss.str();
        return false;
    }
}

void E57ParserLib::closeFile() {
    if(m_imageFile) {
        if(m_imageFile->isOpen()) m_imageFile->close();
        delete m_imageFile;
        m_imageFile = nullptr;
    }
}

std::string E57ParserLib::getGuid() const {
    if(!m_imageFile) return "";
    
    e57::StructureNode root = m_imageFile->root();
    if(root.isDefined("guid")) {
        return static_cast(root.get("guid")).value();
    }
    return "";
}

std::pair E57ParserLib::getVersion() const {
    if(!m_imageFile) return {0,0};
    return {m_imageFile->fileVersionMajor(), m_imageFile->fileVersionMinor()};
}

int E57ParserLib::getScanCount() const {
    if(!m_imageFile) return 0;
    
    try {
        e57::StructureNode root = m_imageFile->root();
        if(root.isDefined("/data3D")) {
            e57::VectorNode data3D = static_cast(root.get("/data3D"));
            return static_cast(data3D.childCount());
        }
    } catch(...) {}
    return 0;
}

std::string E57ParserLib::getLastError() const {
    return m_lastError;
}
```

### 3. Unit Tests (Google Test)
**tests/test_e57parser.cpp**:
```cpp
#include 
#include "e57parserlib.h"

class E57ParserTest : public ::testing::Test {
protected:
    E57ParserLib parser;
    const std::string validFile = "testdata/valid.e57";
    const std::string invalidFile = "testdata/invalid.e57";
};

TEST_F(E57ParserTest, OpenValidFile) {
    EXPECT_TRUE(parser.openFile(validFile));
    EXPECT_TRUE(parser.getLastError().empty());
}

TEST_F(E57ParserTest, OpenInvalidFile) {
    EXPECT_FALSE(parser.openFile(invalidFile));
    EXPECT_FALSE(parser.getLastError().empty());
}

TEST_F(E57ParserTest, ReadMetadata) {
    ASSERT_TRUE(parser.openFile(validFile));
    
    EXPECT_FALSE(parser.getGuid().empty());
    auto version = parser.getVersion();
    EXPECT_GT(version.first, 0);
    EXPECT_GE(parser.getScanCount(), 0);
}
```

### 4. Critical Implementation Notes

1. **Dependency Management**:
   ```bash
   # Install dependencies
   vcpkg install libe57format:x64-windows
   ```

2. **CMake Configuration**:
   ```bash
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake
   ```

3. **Error Handling Patterns**:
   ```cpp
   try {
       // libE57Format operations
   } catch (e57::E57Exception& ex) {
       std::stringstream ss;
       ex.report(ss);
       // Log or store error message
       m_lastError = ss.str();
   }
   ```

4. **Qt Integration**:
   ```cpp
   // Convert E57 errors to QString for Qt UI
   QString error = QString::fromStdString(parser.getLastError());
   ```

### 5. Recommended Development Workflow

1. Validate library linkage with simple test program
2. Implement core file opening functionality
3. Add metadata extraction methods incrementally
4. Create test E57 files using CloudCompare or E57 reference implementation
5. Implement continuous integration with test file validation

### 6. Key API Reference

**libE57Format Core Classes**:
- `e57::ImageFile`: Main file handle
- `e57::StructureNode`: XML structure container
- `e57::VectorNode`: Ordered collection of nodes
- `e57::E57Exception`: Error reporting class

**Critical Methods**:
- `ImageFile::root()`: Get root structure node
- `Node::isDefined()`: Check node existence
- `Node::type()`: Get node type (E57_STRING, etc.)

For complex E57 structures, refer to the official ASTM E2807 specification document for node hierarchy requirements.

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/6928cdd1-d8a9-4783-9802-2eeb667bacfb/paste.txt
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/2469c776-4e44-454f-8652-5d5edb726e3b/paste-2.txt

---
Answer from Perplexity: pplx.ai/share