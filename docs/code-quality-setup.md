# CloudRegistration Code Quality Setup

This document describes the code quality tools and processes implemented for the CloudRegistration project.

## Overview

The CloudRegistration project uses automated code formatting and static analysis tools to maintain consistent code quality:

- **clang-format**: Automatic code formatting
- **clang-tidy**: Static code analysis
- **CI/CD Integration**: Automated quality checks on every commit
- **Pre-commit Hooks**: Local quality checks before commits

## Tools and Configuration

### clang-format

**Purpose**: Ensures consistent code formatting across the entire codebase.

**Configuration File**: `.clang-format`
- Based on Google style with Allman braces
- 120 character line limit
- 4-space indentation
- Qt-specific include ordering

**Usage**:
```bash
# Format all C++ files
cmake --build build --target format_all

# Check formatting without modifying files
cmake --build build --target format_check
```

### clang-tidy

**Purpose**: Static analysis to catch potential bugs and enforce modern C++ practices.

**Configuration File**: `.clang-tidy`
- Comprehensive rule set for C++ best practices
- Qt6-specific configurations
- Disabled rules that conflict with project style

**Usage**:
```bash
# Run static analysis on all C++ files
cmake --build build --target run_clang_tidy
```

## Local Development Setup

### Prerequisites

1. **clang-format and clang-tidy**: Usually installed with Visual Studio or LLVM
2. **CMake**: Version 3.20 or higher
3. **vcpkg**: For dependency management

### Initial Setup

1. **Configure the project**:
   ```bash
   cmake -B build -S .
   ```

2. **Verify tools are available**:
   ```bash
   clang-format --version
   clang-tidy --version
   ```

3. **Install pre-commit hook** (optional but recommended):
   ```bash
   scripts\install-pre-commit-hook.bat
   ```

### Daily Workflow

1. **Before committing code**:
   ```bash
   # Format your code
   cmake --build build --target format_all
   
   # Check for static analysis issues
   cmake --build build --target run_clang_tidy
   ```

2. **Commit your changes**:
   ```bash
   git add .
   git commit -m "Your commit message"
   ```

   If you have the pre-commit hook installed, it will automatically check formatting.

## CI/CD Integration

### GitHub Actions Workflow

The project includes a comprehensive CI/CD workflow (`.github/workflows/code-quality.yml`) that:

1. **Code Formatting Check**: Verifies all code follows the formatting standards
2. **Static Analysis**: Runs clang-tidy analysis on the entire codebase
3. **Build and Test**: Ensures the project builds and tests pass

### Workflow Triggers

- **Push**: To `main`, `develop`, or `feature/*` branches
- **Pull Request**: To `main` or `develop` branches

### Handling CI Failures

If the CI fails due to formatting issues:
1. Run `cmake --build build --target format_all` locally
2. Commit the formatting changes
3. Push the updated code

If the CI fails due to static analysis warnings:
1. Review the clang-tidy output in the CI logs
2. Fix the identified issues
3. Commit and push the fixes

## Configuration Details

### .clang-format Configuration

Key settings:
- **BasedOnStyle**: Google
- **BreakBeforeBraces**: Allman
- **ColumnLimit**: 120
- **IndentWidth**: 4
- **PointerAlignment**: Left

### .clang-tidy Configuration

Enabled check categories:
- **bugprone-***: Potential bug detection
- **cert-***: CERT secure coding standards
- **cppcoreguidelines-***: C++ Core Guidelines
- **google-***: Google style guidelines
- **misc-***: Miscellaneous checks
- **modernize-***: Modern C++ practices
- **performance-***: Performance improvements
- **portability-***: Cross-platform compatibility
- **readability-***: Code readability

## Troubleshooting

### Common Issues

1. **clang-format not found**:
   - Install Visual Studio with C++ tools
   - Or install LLVM separately
   - Ensure the tools are in your PATH

2. **CMake targets not available**:
   - Reconfigure CMake: `cmake -B build -S .`
   - Check that clang-format/clang-tidy were found during configuration

3. **Too many clang-tidy warnings**:
   - Focus on fixing errors first
   - Consider disabling specific checks in `.clang-tidy` if they're not relevant

### Getting Help

- Check the CMake configuration output for tool detection
- Review the CI logs for specific error messages
- Consult the clang-format and clang-tidy documentation for configuration options

## Best Practices

1. **Run formatting before every commit**
2. **Address static analysis warnings promptly**
3. **Use the pre-commit hook to catch issues early**
4. **Review CI feedback and fix issues quickly**
5. **Keep configuration files updated as the project evolves**

## Future Enhancements

Potential improvements to consider:
- Integration with IDE formatting on save
- Additional static analysis tools (e.g., cppcheck)
- Code coverage reporting
- Performance benchmarking in CI
- Automated dependency updates
