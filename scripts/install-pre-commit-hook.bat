@echo off
REM CloudRegistration Pre-commit Hook Installation Script
REM This script installs a Git pre-commit hook that runs clang-format on staged files

echo Installing CloudRegistration pre-commit hook...

REM Check if we're in a Git repository
if not exist ".git" (
    echo Error: This script must be run from the root of a Git repository.
    exit /b 1
)

REM Create hooks directory if it doesn't exist
if not exist ".git\hooks" (
    mkdir ".git\hooks"
)

REM Create the pre-commit hook
echo Creating pre-commit hook...
(
echo #!/bin/sh
echo # CloudRegistration pre-commit hook
echo # Automatically format C++ files with clang-format before commit
echo.
echo # Check if clang-format is available
echo if ! command -v clang-format ^&^> /dev/null; then
echo     echo "Warning: clang-format not found. Skipping formatting check."
echo     exit 0
echo fi
echo.
echo # Get list of staged C++ files
echo STAGED_FILES=^$^(git diff --cached --name-only --diff-filter=ACM ^| grep -E '\.\(cpp^|cxx^|cc^|c^|hpp^|hxx^|h^)$'^)
echo.
echo if [ -z "$STAGED_FILES" ]; then
echo     echo "No C++ files staged for commit."
echo     exit 0
echo fi
echo.
echo echo "Checking code formatting for staged C++ files..."
echo.
echo # Check formatting for each staged file
echo NEEDS_FORMATTING=""
echo for FILE in $STAGED_FILES; do
echo     if [ -f "$FILE" ]; then
echo         # Check if file needs formatting
echo         if ! clang-format --dry-run --Werror "$FILE" ^> /dev/null 2^>^&1; then
echo             NEEDS_FORMATTING="$NEEDS_FORMATTING $FILE"
echo         fi
echo     fi
echo done
echo.
echo if [ ! -z "$NEEDS_FORMATTING" ]; then
echo     echo "The following files need formatting:"
echo     for FILE in $NEEDS_FORMATTING; do
echo         echo "  - $FILE"
echo     done
echo     echo ""
echo     echo "Please run 'cmake --build build --target format_all' to format the code."
echo     echo "Then stage the changes and commit again."
echo     exit 1
echo fi
echo.
echo echo "All staged C++ files are properly formatted."
echo exit 0
) > ".git\hooks\pre-commit"

REM Make the hook executable (on Windows, this is handled by Git)
echo Pre-commit hook installed successfully!
echo.
echo The hook will now check code formatting before each commit.
echo To bypass the hook for a specific commit, use: git commit --no-verify
echo.
echo To uninstall the hook, delete: .git\hooks\pre-commit
pause
