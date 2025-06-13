# Sprint 3 Git Workflow Guide

## Creating Sub-branch and PR for Sprint 3 Implementation

### Step 1: Create Sprint 3 Sub-branch
```powershell
# Ensure you're on the feature/modular-restructure branch
git checkout feature/modular-restructure

# Pull latest changes from main and merge into feature branch
git pull origin main ; git merge main

# Create and checkout Sprint 3 sub-branch
git checkout -b feature/modular-restructure/sprint3-algorithms-library

# Verify you're on the correct branch
git branch --show-current
```

### Step 2: Stage and Commit Sprint 3 Changes
```powershell
# Add all modified and new files
git add src/algorithms/CMakeLists.txt
git add tests/algorithms/CMakeLists.txt
git add tests/algorithms/test_icp_registration.cpp
git add tests/algorithms/test_least_squares_alignment.cpp
git add tests/algorithms/test_point_to_plane_icp.cpp
git add SPRINT3_IMPLEMENTATION_SUMMARY.md
git add validate_sprint3.py

# Remove old test files (if they still exist)
git rm tests/test_icp_registration.cpp 2>$null
git rm tests/sprint4/test_least_squares_alignment.cpp 2>$null
git rm tests/test_point_to_plane_icp.cpp 2>$null

# Check status
git status

# Commit with descriptive message
git commit -m "Sprint 3: Implement Algorithms Library

- Updated src/algorithms/CMakeLists.txt with proper dependencies
- Migrated algorithm tests to tests/algorithms/ directory
- Created comprehensive test CMakeLists.txt for algorithms
- Updated include paths in all test files
- Removed old test files from original locations
- Added validation script and implementation summary

Algorithms library now properly:
- Links against Core, Qt6::Core, Qt6::Gui, Eigen3::Eigen
- Provides ICPRegistration, LeastSquaresAlignment, PointToPlaneICP
- Has comprehensive test suite with proper modular structure
- Integrates with existing app without direct file compilation

All Sprint 3 acceptance criteria met and validated."
```

### Step 3: Push Sub-branch to Remote
```powershell
# Push the new sub-branch to remote
git push -u origin feature/modular-restructure/sprint3-algorithms-library

# Verify push was successful
git log --oneline -5
```

### Step 4: Create Pull Request
```powershell
# Option 1: Using GitHub CLI (if available)
gh pr create --base feature/modular-restructure --head feature/modular-restructure/sprint3-algorithms-library --title "Sprint 3: Algorithms Library Implementation" --body "Implements Sprint 3 requirements for the Algorithms static library.

## Summary
- ✅ Algorithms library properly configured with CMake
- ✅ All algorithm tests migrated and updated
- ✅ Include paths corrected for modular structure
- ✅ Dependencies properly linked (Core, Qt6, Eigen3)
- ✅ App integration maintained without direct compilation
- ✅ All validation checks pass

## Files Changed
- Updated: src/algorithms/CMakeLists.txt
- Created: tests/algorithms/CMakeLists.txt
- Moved: tests/algorithms/test_*.cpp (3 files)
- Added: SPRINT3_IMPLEMENTATION_SUMMARY.md
- Added: validate_sprint3.py

## Testing
Run validation: \`python3 validate_sprint3.py\`
Build library: \`cmake --build build --target Algorithms\`
Run tests: \`ctest -R \"ICPRegistrationTests|LeastSquaresAlignmentTests|PointToPlaneICPTests\"\`

Ready for review and merge into feature/modular-restructure."

# Option 2: Manual PR creation
Write-Host "GitHub CLI not available. Please create PR manually:"
Write-Host "1. Go to: https://github.com/bramburn/cloud_registration/compare"
Write-Host "2. Set base branch: feature/modular-restructure"
Write-Host "3. Set compare branch: feature/modular-restructure/sprint3-algorithms-library"
Write-Host "4. Use the title and description from above"
```

### Step 5: Verification Commands
```powershell
# Verify branch structure
git log --oneline --graph -10

# Verify files are in correct locations
Get-ChildItem tests/algorithms/*.cpp
Get-ChildItem src/algorithms/CMakeLists.txt

# Run validation
python3 validate_sprint3.py

# Check for any uncommitted changes
git status
```

### Step 6: Post-PR Actions
After the PR is created and reviewed:

```powershell
# If changes are requested, make them on the same branch
git checkout feature/modular-restructure/sprint3-algorithms-library
# Make changes...
git add .
git commit -m "Address PR feedback: [specific changes]"
git push

# After PR is approved and merged
git checkout feature/modular-restructure
git pull origin feature/modular-restructure
git branch -d feature/modular-restructure/sprint3-algorithms-library  # Clean up local branch
```

## Important Notes

1. **Branch Naming**: Using the pattern `feature/modular-restructure/sprint3-algorithms-library` maintains clear hierarchy and traceability.

2. **Commit Message**: The commit message follows conventional commit format and clearly describes all changes made.

3. **PR Target**: The PR targets `feature/modular-restructure` branch, not `main`, as this is part of the larger modular restructuring effort.

4. **Validation**: Always run the validation script before creating the PR to ensure all requirements are met.

5. **Parallel Development**: Once this PR is merged, other sprint teams can work on their respective modules (Parsers, Rendering, UI) in parallel.

## Troubleshooting

If you encounter merge conflicts:
```powershell
# Pull latest changes from base branch
git checkout feature/modular-restructure
git pull origin feature/modular-restructure

# Merge into your sprint branch
git checkout feature/modular-restructure/sprint3-algorithms-library
git merge feature/modular-restructure

# Resolve conflicts manually, then:
git add .
git commit -m "Resolve merge conflicts with feature/modular-restructure"
git push
```

This workflow ensures clean integration of Sprint 3 changes while maintaining the parallel development structure outlined in the PRD.
