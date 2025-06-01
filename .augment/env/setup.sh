#!/bin/bash

# Check and create setup.sh in the correct location
set -e

echo "=== Checking and Creating setup.sh ==="

cd /mnt/persist/workspace

# Check if setup.sh exists
if [ ! -f "setup.sh" ]; then
    echo "Creating setup.sh in the workspace directory..."
    
    # Create the setup.sh script
    cat > setup.sh << 'EOF'
#!/bin/bash

# Cloud Registration Project Setup Script
# This script sets up the development environment for the Cloud Registration application
# Supports Ubuntu/Debian Linux systems

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command_exists apt-get; then
            echo "ubuntu"
        elif command_exists yum; then
            echo "centos"
        else
            echo "unknown"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# Function to install packages on Ubuntu/Debian
install_ubuntu_packages() {
    print_status "Updating package lists..."
    sudo apt-get update

    print_status "Installing essential build tools..."
    sudo apt-get install -y \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        git \
        curl \
        wget

    print_status "Installing Qt6 development packages..."
    sudo apt-get install -y \
        qt6-base-dev \
        qt6-base-dev-tools \
        libqt6opengl6-dev \
        libqt6openglwidgets6 \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        libgl1-mesa-dev \
        libglu1-mesa-dev

    print_status "Installing testing frameworks..."
    sudo apt-get install -y \
        libgtest-dev \
        libgmock-dev

    print_status "Installing additional dependencies..."
    sudo apt-get install -y \
        libxml2-dev \
        libboost-dev \
        libboost-system-dev \
        libboost-filesystem-dev \
        lcov

    print_success "All packages installed successfully!"
}

# Function to install packages on CentOS/RHEL
install_centos_packages() {
    print_status "Installing EPEL repository..."
    sudo yum install -y epel-release

    print_status "Installing development tools..."
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y cmake ninja-build pkg-config git

    print_status "Installing Qt6 packages..."
    sudo yum install -y qt6-qtbase-devel qt6-qttools-devel

    print_warning "Some packages may need manual installation on CentOS"
}

# Function to install packages on macOS
install_macos_packages() {
    if ! command_exists brew; then
        print_error "Homebrew not found. Please install Homebrew first:"
        print_error "https://brew.sh/"
        exit 1
    fi

    print_status "Installing packages via Homebrew..."
    brew install cmake ninja qt6 boost

    print_success "macOS packages installed successfully!"
}

# Function to build Google Test (Ubuntu specific)
build_gtest_ubuntu() {
    if [ -d "/usr/src/gtest" ]; then
        print_status "Building Google Test..."
        cd /usr/src/gtest
        sudo cmake CMakeLists.txt
        sudo make
        sudo cp lib/*.a /usr/lib/ 2>/dev/null || sudo cp *.a /usr/lib/
        cd - > /dev/null
        print_success "Google Test built successfully!"
    fi
}

# Function to setup environment variables
setup_environment() {
    print_status "Setting up environment variables..."
    
    # Create or update .profile
    if ! grep -q "# Cloud Registration Environment" ~/.profile 2>/dev/null; then
        echo "" >> ~/.profile
        echo "# Cloud Registration Environment" >> ~/.profile
        echo "export QT_SELECT=qt6" >> ~/.profile
        echo "export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6:\$CMAKE_PREFIX_PATH" >> ~/.profile
        print_success "Environment variables added to ~/.profile"
    else
        print_status "Environment variables already configured"
    fi
}

# Function to create build directory and configure project
configure_project() {
    print_status "Configuring project..."
    
    # Clean and create build directory
    rm -rf build
    mkdir -p build
    cd build
    
    # Configure with CMake
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6 \
        -G Ninja
    
    cd ..
    print_success "Project configured successfully!"
}

# Function to build the project
build_project() {
    print_status "Building the project..."
    
    cd build
    ninja
    cd ..
    
    if [ -f "build/CloudRegistration" ]; then
        print_success "Project built successfully!"
        print_success "Executable: build/CloudRegistration"
    else
        print_error "Build failed - executable not found"
        return 1
    fi
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    cd build
    if command_exists ctest; then
        ctest --output-on-failure
        print_success "Tests completed!"
    else
        print_warning "CTest not available, skipping tests"
    fi
    cd ..
}

# Function to display usage information
show_usage() {
    echo "Cloud Registration Project Setup Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --install-deps    Install system dependencies only"
    echo "  --configure       Configure project only"
    echo "  --build          Build project only"
    echo "  --test           Run tests only"
    echo "  --clean          Clean build directory"
    echo "  --help           Show this help message"
    echo ""
    echo "Default: Install dependencies, configure, and build the project"
}

# Function to clean build directory
clean_build() {
    print_status "Cleaning build directory..."
    rm -rf build
    print_success "Build directory cleaned!"
}

# Main setup function
main_setup() {
    print_status "Starting Cloud Registration project setup..."
    print_status "Detected OS: $(detect_os)"
    
    # Install dependencies based on OS
    case $(detect_os) in
        "ubuntu")
            install_ubuntu_packages
            build_gtest_ubuntu
            ;;
        "centos")
            install_centos_packages
            ;;
        "macos")
            install_macos_packages
            ;;
        *)
            print_error "Unsupported operating system"
            exit 1
            ;;
    esac
    
    # Setup environment
    setup_environment
    
    # Configure and build project
    configure_project
    build_project
    
    # Run tests
    run_tests
    
    print_success "Setup completed successfully!"
    echo ""
    echo "Next steps:"
    echo "1. Source your profile: source ~/.profile"
    echo "2. Run the application: ./build/CloudRegistration"
    echo "3. Test with sample LAS files in the sample/ directory"
    echo ""
    echo "For development:"
    echo "- Build: cd build && ninja"
    echo "- Test: cd build && ctest --output-on-failure"
    echo "- Clean: ./setup.sh --clean"
}

# Parse command line arguments
case "${1:-}" in
    --install-deps)
        case $(detect_os) in
            "ubuntu") install_ubuntu_packages; build_gtest_ubuntu ;;
            "centos") install_centos_packages ;;
            "macos") install_macos_packages ;;
            *) print_error "Unsupported OS"; exit 1 ;;
        esac
        ;;
    --configure)
        configure_project
        ;;
    --build)
        build_project
        ;;
    --test)
        run_tests
        ;;
    --clean)
        clean_build
        ;;
    --help)
        show_usage
        ;;
    "")
        main_setup
        ;;
    *)
        print_error "Unknown option: $1"
        show_usage
        exit 1
        ;;
esac
EOF

    # Make the setup script executable
    chmod +x setup.sh
    
    echo "✅ setup.sh created and made executable"
else
    echo "✅ setup.sh already exists"
fi

echo ""
echo "📋 Final Repository Setup Summary:"
echo "=================================="
echo ""
echo "✅ Core Files:"
ls -la setup.sh README.md CMakeLists.txt 2>/dev/null || echo "Some files may be missing"

echo ""
echo "✅ Sprint 1.1 Implementation Files:"
echo "   • src/lasparser.h/cpp - LAS file parser"
echo "   • src/lasheadermetadata.h - LAS metadata structure"
echo "   • src/mainwindow.h/cpp - Main application window"
echo "   • src/pointcloudviewerwidget.h/cpp - OpenGL viewer"
echo "   • src/e57parser.h/cpp - E57 parser (mock)"
echo ""

echo "🚀 Ready to Use:"
echo "================"
echo ""
echo "1. Run setup script:"
echo "   ./setup.sh"
echo ""
echo "2. Quick development workflow:"
echo "   ./setup.sh --build    # Build after changes"
echo "   ./setup.sh --test     # Run tests"
echo "   ./setup.sh --clean    # Clean build"
echo ""
echo "3. Run application:"
echo "   ./build/CloudRegistration"
echo ""

echo "🎉 Repository setup is complete and ready for development! 🎉"