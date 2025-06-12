#!/usr/bin/env python3
"""
Sprint 3 Validation Script
Validates that all Sprint 3 tasks have been completed correctly.
"""

import os
import sys
from pathlib import Path

def check_file_exists(filepath, description):
    """Check if a file exists and print status."""
    if os.path.exists(filepath):
        print(f"✓ {description}: {filepath}")
        return True
    else:
        print(f"✗ {description}: {filepath} (MISSING)")
        return False

def check_file_contains(filepath, search_text, description):
    """Check if a file contains specific text."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            if search_text in content:
                print(f"✓ {description}")
                return True
            else:
                print(f"✗ {description} (NOT FOUND)")
                return False
    except Exception as e:
        print(f"✗ {description} (ERROR: {e})")
        return False

def main():
    print("=== Sprint 3: Algorithms Library Implementation Validation ===\n")
    
    all_checks_passed = True
    
    # 1. Check algorithm source files exist
    print("1. Checking algorithm source files...")
    algorithm_files = [
        ("src/algorithms/ICPRegistration.h", "ICP Registration header"),
        ("src/algorithms/ICPRegistration.cpp", "ICP Registration implementation"),
        ("src/algorithms/LeastSquaresAlignment.h", "Least Squares Alignment header"),
        ("src/algorithms/LeastSquaresAlignment.cpp", "Least Squares Alignment implementation"),
        ("src/algorithms/PointToPlaneICP.h", "Point-to-Plane ICP header"),
        ("src/algorithms/PointToPlaneICP.cpp", "Point-to-Plane ICP implementation"),
    ]
    
    for filepath, description in algorithm_files:
        if not check_file_exists(filepath, description):
            all_checks_passed = False
    
    # 2. Check algorithms CMakeLists.txt
    print("\n2. Checking algorithms CMakeLists.txt...")
    cmake_file = "src/algorithms/CMakeLists.txt"
    if check_file_exists(cmake_file, "Algorithms CMakeLists.txt"):
        # Check for required content
        checks = [
            ("add_library(Algorithms STATIC", "Algorithms library definition"),
            ("target_link_libraries(Algorithms PUBLIC", "Library dependencies"),
            ("Eigen3::Eigen", "Eigen3 dependency"),
            ("Qt6::Core", "Qt6 Core dependency"),
            ("Qt6::Gui", "Qt6 Gui dependency"),
        ]
        
        for search_text, description in checks:
            if not check_file_contains(cmake_file, search_text, f"  - {description}"):
                all_checks_passed = False
    else:
        all_checks_passed = False
    
    # 3. Check test files have been moved
    print("\n3. Checking test files migration...")
    test_files = [
        ("tests/algorithms/test_icp_registration.cpp", "ICP Registration test"),
        ("tests/algorithms/test_least_squares_alignment.cpp", "Least Squares Alignment test"),
        ("tests/algorithms/test_point_to_plane_icp.cpp", "Point-to-Plane ICP test"),
    ]
    
    for filepath, description in test_files:
        if not check_file_exists(filepath, description):
            all_checks_passed = False
    
    # 4. Check old test files have been removed
    print("\n4. Checking old test files removal...")
    old_test_files = [
        ("tests/test_icp_registration.cpp", "Old ICP Registration test"),
        ("tests/sprint4/test_least_squares_alignment.cpp", "Old Least Squares Alignment test"),
        ("tests/test_point_to_plane_icp.cpp", "Old Point-to-Plane ICP test"),
    ]
    
    for filepath, description in old_test_files:
        if os.path.exists(filepath):
            print(f"✗ {description}: {filepath} (SHOULD BE REMOVED)")
            all_checks_passed = False
        else:
            print(f"✓ {description}: {filepath} (correctly removed)")
    
    # 5. Check algorithms test CMakeLists.txt
    print("\n5. Checking algorithms test CMakeLists.txt...")
    test_cmake_file = "tests/algorithms/CMakeLists.txt"
    if check_file_exists(test_cmake_file, "Algorithms test CMakeLists.txt"):
        # Check for required content
        checks = [
            ("add_executable(ICPRegistrationTests", "ICP Registration test executable"),
            ("add_executable(LeastSquaresAlignmentTests", "Least Squares Alignment test executable"),
            ("add_executable(PointToPlaneICPTests", "Point-to-Plane ICP test executable"),
            ("target_link_libraries(ICPRegistrationTests PRIVATE", "ICP test dependencies"),
            ("Algorithms", "Algorithms library linkage"),
            ("GTest::gtest_main", "Google Test linkage"),
        ]
        
        for search_text, description in checks:
            if not check_file_contains(test_cmake_file, search_text, f"  - {description}"):
                all_checks_passed = False
    else:
        all_checks_passed = False
    
    # 6. Check include paths in test files
    print("\n6. Checking include paths in test files...")
    include_checks = [
        ("tests/algorithms/test_icp_registration.cpp", "../../src/algorithms/ICPRegistration.h", "ICP test include path"),
        ("tests/algorithms/test_least_squares_alignment.cpp", "../../src/algorithms/LeastSquaresAlignment.h", "LSA test include path"),
        ("tests/algorithms/test_point_to_plane_icp.cpp", "../../src/algorithms/PointToPlaneICP.h", "P2P test include path"),
    ]
    
    for filepath, search_text, description in include_checks:
        if not check_file_contains(filepath, search_text, f"  - {description}"):
            all_checks_passed = False
    
    # 7. Check app CMakeLists.txt doesn't include algorithm files
    print("\n7. Checking app CMakeLists.txt...")
    app_cmake_file = "src/app/CMakeLists.txt"
    if check_file_exists(app_cmake_file, "App CMakeLists.txt"):
        # Check that it links to Algorithms library
        if not check_file_contains(app_cmake_file, "Algorithms", "  - Links to Algorithms library"):
            all_checks_passed = False
        
        # Check that it doesn't directly compile algorithm files
        bad_includes = [
            ("ICPRegistration.cpp", "Should not directly compile ICP Registration"),
            ("LeastSquaresAlignment.cpp", "Should not directly compile Least Squares Alignment"),
            ("PointToPlaneICP.cpp", "Should not directly compile Point-to-Plane ICP"),
        ]
        
        for search_text, description in bad_includes:
            if check_file_contains(app_cmake_file, search_text, f"  - {description}"):
                print(f"✗ App CMakeLists.txt should not directly compile {search_text}")
                all_checks_passed = False
            else:
                print(f"✓ App CMakeLists.txt correctly excludes {search_text}")
    else:
        all_checks_passed = False
    
    # Summary
    print("\n" + "="*60)
    if all_checks_passed:
        print("🎉 ALL SPRINT 3 VALIDATION CHECKS PASSED!")
        print("\nThe Algorithms library has been successfully implemented:")
        print("- ✓ Algorithm files are properly located")
        print("- ✓ CMake configuration is correct")
        print("- ✓ Tests have been migrated and configured")
        print("- ✓ Include paths are updated")
        print("- ✓ App no longer directly compiles algorithm files")
        return 0
    else:
        print("❌ SOME VALIDATION CHECKS FAILED!")
        print("\nPlease review the failed checks above and fix the issues.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
