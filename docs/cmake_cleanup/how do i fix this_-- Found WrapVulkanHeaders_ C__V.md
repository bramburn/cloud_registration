<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" class="logo" width="120"/>

# how do i fix this

-- Found WrapVulkanHeaders: C:/VulkanSDK/1.4.313.1/Include
CMake Error at CMakeLists.txt:165 (find_package):
Could not find a package configuration file provided by "Eigen3" with any
of the following names:

    Eigen3Config.cmake
    eigen3-config.cmake
    Add the installation prefix of "Eigen3" to CMAKE_PREFIX_PATH or set
"Eigen3_DIR" to a directory containing one of the above files.  If "Eigen3"
provides a separate development package or SDK, be sure it has been
installed.

-- Configuring incomplete, errors occurred

To resolve the CMake error when Eigen3 cannot be found, follow these steps:

### 1. **Install Eigen3 Correctly**

- **Using vcpkg** (recommended for Windows):

```bash
vcpkg install eigen3
```

Configure CMake with the vcpkg toolchain:

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

- **Manual Installation** (if not using a package manager):

1. Download Eigen from [the official website](https://eigen.tuxfamily.org/).
2. Build and install with CMake:

```bash
cmake -B build -S eigen_source_dir
cmake --install build --prefix="C:/Eigen3"
```


### 2. **Set CMake Variables**

- If Eigen is installed manually, specify its location during CMake configuration:

```bash
cmake -B build -S . -DEigen3_DIR="C:/Eigen3/share/eigen3/cmake"
```

or:

```bash
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Eigen3"
```


### 3. **Update `CMakeLists.txt`**

Ensure `find_package` and linking are correctly implemented:

```cmake
cmake_minimum_required(VERSION 3.10)
project(YourProject)

find_package(Eigen3 REQUIRED)  # Must be before any target definitions

add_executable(YourTarget main.cpp)
target_link_libraries(YourTarget PRIVATE Eigen3::Eigen)
```


### 4. **Verify Installation Paths**

- Confirm `Eigen3Config.cmake` exists in the specified directory (e.g., `C:/Eigen3/share/eigen3/cmake`).
- For vcpkg users, ensure the toolchain file path is correct.


### Key Notes:

- **Order Matters**: Set `CMAKE_TOOLCHAIN_FILE`, `Eigen3_DIR`, or `CMAKE_PREFIX_PATH` **before** the `project()` command in `CMakeLists.txt`.
- **vcpkg Integration**: Automatically handles dependencies if the toolchain is specified.
- **Header-Only**: Eigen requires no compilation, but CMake needs the config files for proper detection.

By following these steps, CMake should locate Eigen3 and resolve the configuration error[^1_7][^1_4][^1_5].

<div style="text-align: center">⁂</div>

[^1_1]: how-does-3dsurvey-software-app-wUW6gHbzTGSnOP8HJzfduA.md

[^1_2]: paste.txt

[^1_3]: https://github.com/conan-io/conan-center-index/issues/13353

[^1_4]: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

[^1_5]: https://github.com/evaleev/libint/issues/172

[^1_6]: https://www.reddit.com/r/cmake/comments/l4p81q/unable_to_find_eigen_using_cmake/

[^1_7]: https://eigen.tuxfamily.org/dox/TopicCMakeGuide.html

[^1_8]: https://github.com/The-OpenROAD-Project/OpenROAD/issues/413

[^1_9]: https://askubuntu.com/questions/1265526/cmake-cant-find-the-eigen3-package

[^1_10]: https://gitlab.com/libeigen/eigen/-/issues/1375

[^1_11]: https://forum.qt.io/topic/125307/qmake-alternative-of-cmake-s-find_package

[^1_12]: https://www.sofa-framework.org/community/forum/topic/cmake-error-in-building-sofa/


---

# how do i fix?

PS C:\dev\cloud_registration> vcpkg install eigen3
warning: The vcpkg C:\dev\vcpkg\vcpkg.exe is using detected vcpkg root C:\dev\vcpkg and ignoring mismatched VCPKG_ROOT environment value c:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg. To suppress this message, unset the environment variable or use the --vcpkg-root command line switch.
error: In manifest mode, `vcpkg install` does not support individual package arguments.
To install additional packages, edit vcpkg.json and then run `vcpkg install` without any package arguments.
See https://learn.microsoft.com/vcpkg/users/manifests?WT.mc_id=vcpkg_inproduct_cli for more information.
Using manifest file at C:\dev\cloud_registration\vcpkg.json.
Synopsis: Installs a package
Examples:
vcpkg install <port name> <port name>...
vcpkg install zlib zlib:x64-windows curl boost
vcpkg install --triplet x64-windows
See https://learn.microsoft.com/vcpkg/commands/install for more information.
Options:
--allow-unsupported    Continues with a warning on unsupported ports, rather than failing
--x-asset-sources=...  Asset caching sources. See 'vcpkg help assetcaching'
--binarysource=...     Binary caching sources. See 'vcpkg help binarycaching'
--x-buildtrees-root=...
Buildtrees directory (experimental)
--classic              Force classic mode, even if a manifest could be found.
--clean-after-build    Cleans buildtrees, packages and downloads after building each package
--clean-buildtrees-after-build
Cleans buildtrees after building each package
--clean-downloads-after-build
Cleans downloads after building each package
--clean-packages-after-build
Cleans packages after building each package
--downloads-root=...   Downloads directory (default: %VCPKG_DOWNLOADS%)
--dry-run              Does not actually build or install
--editable             Disables source re-extraction and binary caching for libraries on the
command line (classic mode)
--enforce-port-checks  Fails install if a port has detected problems or attempts to use a
deprecated feature
--x-feature=...        Additional features from the top-level manifest to install (manifest mode)
--head                 Installs the libraries on the command line using the latest upstream
sources (classic mode)
--host-triplet=...     Host triplet. See 'vcpkg help triplet' (default:
%VCPKG_DEFAULT_HOST_TRIPLET%)
--x-install-root=...   Installed directory (experimental)
--keep-going           Continues installing packages on failure
--x-no-default-features
Does not install the default features from the top-level manifest (manifest
mode)
--no-downloads         Does not download new sources
--no-print-usage       Does not print CMake usage information after install
--only-binarycaching   Fails if cached binaries are not available
--only-downloads       Makes best-effort attempt to download sources without building
--overlay-ports=...    Overlay-port directories, or directories containing overlay-port
directories (also: %VCPKG_OVERLAY_PORTS%)
--overlay-triplets=... Directories of overlay triplets (also: %VCPKG_OVERLAY_TRIPLETS%)
--x-packages-root=...  Packages directory (experimental)
--recurse              Allows removal of packages as part of installation
--triplet=...          Target triplet. See 'vcpkg help triplet' (default: %VCPKG_DEFAULT_TRIPLET%)
--vcpkg-root=...       The vcpkg root directory (default: %VCPKG_ROOT%)
--x-write-nuget-packages-config=...
Writes a NuGet packages.config-formatted file for use with external binary
caching. See `vcpkg help binarycaching` for more information

To resolve the Eigen3 installation issue in your vcpkg-managed C++/Qt project:

### 1. **Update vcpkg Manifest**

Add Eigen3 to your `vcpkg.json`:

```json
{
  "name": "cloud-registration",
  "version": "1.0.0",
  "dependencies": [
    "eigen3",
    "qt6"
  ]
}
```


### 2. **Install Dependencies**

Run from your project root:

```powershell
vcpkg install --triplet=x64-windows
```


### 3. **Configure CMake with vcpkg**

Add this to your CMake command:

```powershell
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows
```


### 4. **Verify CMake Integration**

In your `CMakeLists.txt`, ensure proper Eigen3 usage:

```cmake
find_package(Eigen3 REQUIRED)
target_link_libraries(CloudRegistration PRIVATE Eigen3::Eigen)
```


### Key Notes:

- **Manifest Mode**: vcpkg requires dependencies in `vcpkg.json`
- **Triplet Specification**: Match your build target (x64-windows shown)
- **Toolchain File**: Essential for CMake to find vcpkg-installed packages

This approach maintains vcpkg manifest mode while resolving the Eigen3 dependency[^2_1][^2_2].

<div style="text-align: center">⁂</div>

[^2_1]: CMakeLists.txt

[^2_2]: repomix-output2.md

[^2_3]: how-does-3dsurvey-software-app-wUW6gHbzTGSnOP8HJzfduA.md

[^2_4]: paste.txt

[^2_5]: https://learn.microsoft.com/en-us/vcpkg/consume/manifest-mode

[^2_6]: https://www.reddit.com/r/cpp_questions/comments/1j7tjdj/vcpkg_install_does_not_support_individual_package/

[^2_7]: https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode

[^2_8]: https://perception-ml.com/oh-so-there-are-c-package-managers/

[^2_9]: https://vcpkg.io/en/package/eigen3.html

[^2_10]: https://vcpkg.link/ports/eigen3

[^2_11]: https://stackoverflow.com/questions/77831134/how-to-install-package-with-module-options-in-vcpkg-manifest-use

[^2_12]: https://www.jetbrains.com/help/clion/package-management.html

[^2_13]: https://jenniferchukwu.com/posts/vcpkg

[^2_14]: https://decovar.dev/blog/2022/10/30/cpp-dependencies-with-vcpkg/


---

# this is my cmake how do i fix it?

Im still getting errors
S C:\dev\cloud_registration\build> cmake ..
-- Found Qt6 at: C:/Qt/6.9.0/msvc2022_64/lib/cmake/Qt6
CMake Error at CMakeLists.txt:165 (find_package):
By not providing "FindEigen3.cmake" in CMAKE_MODULE_PATH this project has
asked CMake to find a package configuration file provided by "Eigen3", but
CMake did not find one.

Could not find a package configuration file provided by "Eigen3" with any
of the following names:

    Eigen3Config.cmake
    eigen3-config.cmake
    Add the installation prefix of "Eigen3" to CMAKE_PREFIX_PATH or set
"Eigen3_DIR" to a directory containing one of the above files.  If "Eigen3"
provides a separate development package or SDK, be sure it has been
installed.

To resolve the Eigen3 configuration error in your Qt6/C++ project using vcpkg:

### 1. **Update vcpkg Manifest**

Add Eigen3 to your `vcpkg.json`:

```json
{
  "dependencies": [
    "eigen3",
    "qt6"
  ]
}
```


### 2. **Install Dependencies**

Run from your project root:

```powershell
vcpkg install --triplet=x64-windows
```


### 3. **Configure CMake with vcpkg Toolchain**

Modify your CMake command to include:

```powershell
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows
```


### 4. **Update CMakeLists.txt**

Ensure proper Eigen3 integration:

```cmake
# Place BEFORE project() command
set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(Eigen3 REQUIRED)
target_link_libraries(CloudRegistration PRIVATE Eigen3::Eigen)
```


### Key Fixes:

- **Manifest Mode Requirement**: vcpkg requires dependencies in `vcpkg.json`
- **Toolchain File**: Essential for CMake to locate vcpkg-installed packages
- **Triplet Specification**: Ensures architecture compatibility (x64-windows shown)

After these changes, CMake should successfully locate Eigen3 through vcpkg's integration. The error occurs because CMake couldn't find Eigen3's config files - vcpkg handles this automatically when properly configured[^3_6][^3_4].

<div style="text-align: center">⁂</div>

[^3_1]: CMakeLists.txt

[^3_2]: how-does-3dsurvey-software-app-wUW6gHbzTGSnOP8HJzfduA.md

[^3_3]: paste.txt

[^3_4]: https://github.com/conan-io/conan-center-index/issues/13353

[^3_5]: https://stackoverflow.com/questions/48144415/not-found-eigen3-dir-when-configuring-a-cmake-project-in-windows

[^3_6]: https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

[^3_7]: https://perception-ml.com/oh-so-there-are-c-package-managers/

[^3_8]: https://www.reddit.com/r/cmake/comments/l4p81q/unable_to_find_eigen_using_cmake/

[^3_9]: https://stackoverflow.com/questions/61778069/cmake-find-package-unable-to-find-eigen3config-cmake-spectra-windows

[^3_10]: https://askubuntu.com/questions/1265526/cmake-cant-find-the-eigen3-package

[^3_11]: https://github.com/stevenlovegrove/Pangolin/issues/692

[^3_12]: https://stackoverflow.com/questions/12249140/find-package-eigen3-for-cmake

[^3_13]: https://www.sofa-framework.org/community/forum/topic/cmake-error-in-building-sofa/


---

# how can i install it manually its not working

To manually install Eigen3 for your Qt6/C++ project, follow these steps:

### 1. **Download Eigen Headers**

```powershell
# Clone Eigen repository (official mirror)
git clone https://gitlab.com/libeigen/eigen.git
cd eigen
git checkout 3.4.0  # Use stable release branch
```


### 2. **CMake Installation**

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="C:/Libs/eigen3"
cmake --install .
```


### 3. **Update CMakeLists.txt**

Add these lines **BEFORE** `find_package(Eigen3`:

```cmake
# Manual Eigen3 configuration
set(Eigen3_DIR "C:/Libs/eigen3/share/eigen3/cmake")
find_package(Eigen3 REQUIRED NO_MODULE)
```


### 4. **Alternative: Embedded Submodule**

```powershell
# In your project root
git submodule add https://gitlab.com/libeigen/eigen.git extern/eigen
```

Add to CMakeLists.txt:

```cmake
# Add before find_package(Eigen3)
add_subdirectory(extern/eigen)
include_directories(${EIGEN3_INCLUDE_DIRS})
```


### Key Fixes for Your Setup:

1. **Remove `NO_MODULE`** from `find_package` if using manual headers
2. **Set CMAKE_PREFIX_PATH** in CMake command:
```powershell
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Libs/eigen3"
```


### Verify Installation:

Check for these files exist:

```
C:/Libs/eigen3/share/eigen3/cmake/Eigen3Config.cmake
C:/Libs/eigen3/include/eigen3/Eigen/
```

Eigen being header-only requires no compilation, but CMake needs the config files for proper detection[^4_1][^4_4]. The error occurs because CMake can't find these config files in standard locations.

<div style="text-align: center">⁂</div>

[^4_1]: CMakeLists.txt

[^4_2]: how-does-3dsurvey-software-app-wUW6gHbzTGSnOP8HJzfduA.md

[^4_3]: paste.txt

[^4_4]: https://github.com/OPM/eigen3/blob/master/INSTALL

[^4_5]: https://sdurobotics.gitlab.io/robwork/installation/windows_dependencies/eigen3/

[^4_6]: https://eigen.tuxfamily.org/dox/GettingStarted.html

[^4_7]: https://stackoverflow.com/questions/68884411/how-do-you-install-eigen

[^4_8]: https://askubuntu.com/questions/860207/how-to-install-eigen-3-3-in-ubuntu-14-04

[^4_9]: https://www.reddit.com/r/learnprogramming/comments/1d7ouhi/how_do_i_download_the_source_code_of_eigen3_340/

[^4_10]: https://stackoverflow.com/questions/54678613/how-to-install-eigen3-using-pip-on-windows

[^4_11]: https://github.com/Microsoft/vcpkg/issues/4185

[^4_12]: https://stackoverflow.com/questions/63092579/adding-eigen-library-to-c-project-using-cmake

[^4_13]: https://laurentkneip.github.io/opengv/page_installation.html


---

# PS C:\dev\cloud_registration> cmake .

-- Building for: Visual Studio 17 2022
-- Running vcpkg install
Detecting compiler hash for triplet x64-windows...
Compiler found: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe
The following packages are already installed:
eigen3:x64-windows@3.4.0\#5
gtest:x64-windows@1.17.0\#1
libe57format:x64-windows@3.2.0

* vcpkg-cmake:x64-windows@2024-04-23
* vcpkg-cmake-config:x64-windows@2024-05-23
vulkan:x64-windows@2023-12-17
* vulkan-headers:x64-windows@1.4.304.1\#1
* vulkan-loader:x64-windows@1.4.304.1
xerces-c[core,network]:x64-windows@3.3.0
eigen3 provides CMake targets:


# this is heuristically generated, and may not be correct

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE Eigen3::Eigen)

eigen3 provides pkg-config modules:

# A C++ template library for linear algebra: vectors, matrices, and related algorithms

eigen3

The package gtest is compatible with built-in CMake targets:

    enable_testing()
    
    find_package(GTest CONFIG REQUIRED)
    target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
    
    add_test(AllTestsInMain main)
    The package xerces-c is compatible with built-in CMake targets:

    find_package(XercesC REQUIRED)
    target_link_libraries(main PRIVATE XercesC::XercesC)
    libe57format provides CMake targets:

    find_package(E57Format CONFIG REQUIRED)
    target_link_libraries(main PRIVATE E57Format)
    vulkan is compatible with built-in CMake targets:

    # https://cmake.org/cmake/help/latest/module/FindVulkan.html
    find_package(Vulkan REQUIRED)
    target_link_libraries(main PRIVATE Vulkan::Vulkan)
    All requested installations completed successfully in: 1.17 ms
-- Running vcpkg install - done
-- The CXX compiler identification is MSVC 19.44.35207.1
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found Qt6 at: C:/Qt/6.9.0/msvc2022_64/lib/cmake/Qt6
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - not found
-- Found Threads: TRUE
-- Performing Test HAVE_STDATOMIC
-- Performing Test HAVE_STDATOMIC - Success
-- Found WrapAtomic: TRUE
-- Found WrapVulkanHeaders: C:/dev/cloud_registration/vcpkg_installed/x64-windows/include
-- Google Test found - building unit tests
-- Google Mock found - enabling mock-based tests
-- Configuring done (12.6s)
CMake Error at C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake:598 (_add_executable):
Cannot find source file:

    src/registration/WorkflowStateMachine.cpp
    Tried extensions .c .C .c++ .cc .cpp .cxx .cu .mpp .m .M .mm .ixx .cppm
.ccm .cxxm .c++m .h .hh .h++ .hm .hpp .hxx .in .txx .f .F .for .f77 .f90
.f95 .f03 .hip .ispc
Call Stack (most recent call first):
CMakeLists.txt:360 (add_executable)

CMake Error at C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake:598 (_add_executable):
Cannot find source file:

    src/performance/ParallelProcessing.cpp
    Tried extensions .c .C .c++ .cc .cpp .cxx .cu .mpp .m .M .mm .ixx .cppm
.ccm .cxxm .c++m .h .hh .h++ .hm .hpp .hxx .in .txx .f .F .for .f77 .f90
.f95 .f03 .hip .ispc
Call Stack (most recent call first):
CMakeLists.txt:758 (add_executable)

-- Generating done (1.6s)
CMake Generate step failed.  Build files cannot be regenerated correctly.

To resolve the CMake errors about missing source files in your Qt6/C++ project, follow these steps:

### 1. **Verify File Existence**

Check if these files exist in your project directory:

```powershell
ls C:\dev\cloud_registration\src\registration\WorkflowStateMachine.cpp
ls C:\dev\cloud_registration\src\performance\ParallelProcessing.cpp
```


### 2. **Fix Paths in CMakeLists.txt**

Update the source file paths to use absolute paths:

```cmake
# Before
set(SOURCES
    src/registration/WorkflowStateMachine.cpp
    src/performance/ParallelProcessing.cpp
)

# After
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/registration/WorkflowStateMachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/performance/ParallelProcessing.cpp
)
```


### 3. **Check File Case Sensitivity**

Ensure filenames match exactly (including case):

```
WorkflowStateMachine.cpp ≠ workflowstatemachine.cpp
```


### 4. **Update Test Executables**

Modify test target definitions to use absolute paths:

```cmake
# Before (line 360)
add_executable(RegistrationWorkflowTests
    tests/test_registration_workflow.cpp
    src/registration/WorkflowStateMachine.cpp
)

# After
add_executable(RegistrationWorkflowTests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_registration_workflow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/registration/WorkflowStateMachine.cpp
)
```


### 5. **Regenerate Build Files**

After making changes:

```powershell
rm -rf build/
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
```


### Key Notes:

- Use **absolute paths** with `${CMAKE_CURRENT_SOURCE_DIR}` for all source files
- Verify **file existence** in Explorer/Finder
- Check for **typos** in filenames/paths
- Ensure **consistent casing** in filenames and paths

[^5_1][^5_2][^5_4][^5_5][^5_6][^5_7][^5_8]

<div style="text-align: center">⁂</div>

[^5_1]: CMakeLists.txt

[^5_2]: how-does-3dsurvey-software-app-wUW6gHbzTGSnOP8HJzfduA.md

[^5_3]: paste.txt

[^5_4]: https://stackoverflow.com/questions/53708207/why-am-i-getting-error-cannot-find-source-file-in-cmake

[^5_5]: https://stackoverflow.com/q/45122037

[^5_6]: https://www.reddit.com/r/cpp_questions/comments/1305pt7/cmake_error_cannot_find_source_file/

[^5_7]: https://forum.qt.io/topic/155659/cmake-file-not-found-project-sources

[^5_8]: https://robotics.stackexchange.com/questions/88501/cmake-cannot-find-source-file

[^5_9]: https://discourse.cmake.org/t/cmake-error-cannot-find-source-file-no-sources-given-to-target/9650

[^5_10]: https://www.reddit.com/r/QtFramework/comments/1ao5imr/help_with_linking_qt6_to_my_cmake_project/

[^5_11]: https://github.com/Luca96/dlib-for-android/issues/2

[^5_12]: https://stackoverflow.com/questions/74610090/cmake-error-with-qt6-file-does-not-exist

[^5_13]: https://forum.opencv.org/t/cmake-error-trying-to-add-opencv-to-embarcadero-c-builder-10-3/13160


---

# How can I fix missing source files in my CMake project

To resolve the missing source file errors in your CMake/Qt6 project:

### 1. **Verify File Existence**

Check if the files physically exist at these paths:

```powershell
ls C:\dev\cloud_registration\src\registration\WorkflowStateMachine.cpp
ls C:\dev\cloud_registration\src\performance\ParallelProcessing.cpp
```


### 2. **Use Absolute Paths in CMakeLists.txt**

Modify your source declarations to use `${CMAKE_CURRENT_SOURCE_DIR}`:

```cmake
# Before
set(SOURCES
    src/registration/WorkflowStateMachine.cpp
    src/performance/ParallelProcessing.cpp
)

# After
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/registration/WorkflowStateMachine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/performance/ParallelProcessing.cpp
)
```


### 3. **Check File Case Sensitivity**

Ensure filenames match exactly (Windows is case-insensitive but CMake preserves case):

```
WorkflowStateMachine.cpp ≠ workflowstatemachine.cpp
```


### 4. **Update Executable Targets**

Fix test target definitions in `CMakeLists.txt`:

```cmake
# Before (line 360)
add_executable(RegistrationWorkflowTests
    tests/test_registration_workflow.cpp
    src/registration/WorkflowStateMachine.cpp
)

# After
add_executable(RegistrationWorkflowTests
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_registration_workflow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/registration/WorkflowStateMachine.cpp
)
```


### 5. **Rebuild Cleanly**

Delete existing build directory and reconfigure:

```powershell
rm -rf build/
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
```


### Key Fixes:

- **Absolute Paths**: CMake requires explicit paths when files are in subdirectories[^6_1][^6_6]
- **Case Sensitivity**: File paths must match exactly in CMakeLists.txt and filesystem[^6_2][^6_4]
- **Build Clean**: Old build artifacts can cause false errors[^6_6]

This resolves the "Cannot find source file" errors by ensuring CMake can locate all source files through explicit path declarations[^6_1][^6_3][^6_5].

<div style="text-align: center">⁂</div>

[^6_1]: https://stackoverflow.com/questions/43400453/cmake-cannot-find-source-file-add-executable

[^6_2]: https://answers.opencv.org/question/69227/opencv-with-gcc-and-cmake-cannot-find-source-file/

[^6_3]: https://stackoverflow.com/questions/42953465/cmake-cant-find-source-files-where-they-exists

[^6_4]: https://www.reddit.com/r/cpp_questions/comments/1305pt7/cmake_error_cannot_find_source_file/

[^6_5]: https://discourse.cmake.org/t/cmake-error-cannot-find-source-file-no-sources-given-to-target/9650

[^6_6]: https://forum.qt.io/topic/155659/cmake-file-not-found-project-sources

[^6_7]: https://developercommunity.visualstudio.com/content/problem/178495/source-files-not-showing-up-in-cmake-project.html

[^6_8]: https://github.com/nigels-com/glew/issues/314

[^6_9]: https://discourse.cmake.org/t/source-directory-does-not-appear-to-contain-cmakelists-txt/3654

[^6_10]: https://johnfarrier.com/standardizing-the-handling-of-non-source-files-in-cmake-projects-the-config-target/

