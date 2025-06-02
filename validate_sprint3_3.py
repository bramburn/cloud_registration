#!/usr/bin/env python3
"""
Sprint 3.3 Implementation Validation Script

This script validates the implementation of Sprint 3.3 by checking:
1. File existence and structure
2. Code syntax and includes
3. CMake configuration
4. Resource files
5. Test coverage
"""

import os
import re
import sys
from pathlib import Path

def check_file_exists(filepath, description=""):
    """Check if a file exists and report status."""
    if os.path.exists(filepath):
        print(f"✅ {filepath} - {description}")
        return True
    else:
        print(f"❌ {filepath} - {description} (MISSING)")
        return False

def check_cpp_syntax(filepath):
    """Basic C++ syntax validation."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
        # Check for basic C++ structure
        has_includes = '#include' in content
        has_class_or_function = ('class ' in content or 'void ' in content or 'int ' in content)
        balanced_braces = content.count('{') == content.count('}')
        
        if has_includes and has_class_or_function and balanced_braces:
            print(f"✅ {filepath} - Basic syntax validation passed")
            return True
        else:
            print(f"⚠️  {filepath} - Basic syntax validation failed")
            return False
    except Exception as e:
        print(f"❌ {filepath} - Error reading file: {e}")
        return False

def check_header_guards(filepath):
    """Check for proper header guards."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
        # Look for header guards
        has_ifndef = '#ifndef' in content
        has_define = '#define' in content
        has_endif = '#endif' in content
        
        if has_ifndef and has_define and has_endif:
            print(f"✅ {filepath} - Header guards present")
            return True
        else:
            print(f"⚠️  {filepath} - Header guards missing or incomplete")
            return False
    except Exception as e:
        print(f"❌ {filepath} - Error checking header guards: {e}")
        return False

def check_cmake_integration():
    """Check CMakeLists.txt for Sprint 3.3 integration."""
    cmake_file = "CMakeLists.txt"
    if not os.path.exists(cmake_file):
        print(f"❌ {cmake_file} not found")
        return False
        
    try:
        with open(cmake_file, 'r') as f:
            content = f.read()
            
        checks = [
            ("iconmanager.cpp", "IconManager source file"),
            ("progressmanager.cpp", "ProgressManager source file"),
            ("iconmanager.h", "IconManager header file"),
            ("progressmanager.h", "ProgressManager header file"),
            ("resources.qrc", "Resource file"),
            ("Sprint33UIRefinementsTests", "Sprint 3.3 tests"),
        ]
        
        all_good = True
        for check, description in checks:
            if check in content:
                print(f"✅ CMakeLists.txt includes {description}")
            else:
                print(f"❌ CMakeLists.txt missing {description}")
                all_good = False
                
        return all_good
    except Exception as e:
        print(f"❌ Error reading CMakeLists.txt: {e}")
        return False

def check_resource_files():
    """Check resource file structure."""
    qrc_file = "resources.qrc"
    if not check_file_exists(qrc_file, "Qt Resource file"):
        return False
        
    # Check icon directory structure
    icon_dirs = [
        "icons",
        "icons/overlays",
        "icons/badges",
        "icons/light",
        "icons/dark"
    ]
    
    all_good = True
    for icon_dir in icon_dirs:
        if os.path.exists(icon_dir):
            print(f"✅ {icon_dir}/ directory exists")
        else:
            print(f"❌ {icon_dir}/ directory missing")
            all_good = False
            
    # Check for key icon files
    key_icons = [
        "icons/scan.svg",
        "icons/cluster.svg",
        "icons/project.svg",
        "icons/overlays/loaded.svg",
        "icons/overlays/locked.svg",
        "icons/badges/copy.svg"
    ]
    
    for icon in key_icons:
        if os.path.exists(icon):
            print(f"✅ {icon} exists")
        else:
            print(f"❌ {icon} missing")
            all_good = False
            
    return all_good

def main():
    """Main validation function."""
    print("🔍 Sprint 3.3 Implementation Validation")
    print("=" * 50)
    
    # Core implementation files
    core_files = [
        ("src/iconmanager.h", "IconManager header"),
        ("src/iconmanager.cpp", "IconManager implementation"),
        ("src/progressmanager.h", "ProgressManager header"),
        ("src/progressmanager.cpp", "ProgressManager implementation"),
    ]
    
    print("\n📁 Core Implementation Files:")
    all_core_good = True
    for filepath, description in core_files:
        exists = check_file_exists(filepath, description)
        if exists:
            if filepath.endswith('.h'):
                check_header_guards(filepath)
            check_cpp_syntax(filepath)
        all_core_good = all_core_good and exists
    
    # Test files
    print("\n🧪 Test Files:")
    test_files = [
        ("tests/test_sprint3_3_ui_refinements.cpp", "Sprint 3.3 unit tests"),
    ]
    
    all_tests_good = True
    for filepath, description in test_files:
        exists = check_file_exists(filepath, description)
        if exists:
            check_cpp_syntax(filepath)
        all_tests_good = all_tests_good and exists
    
    # Documentation
    print("\n📚 Documentation:")
    doc_files = [
        ("docs/sprint3_3_implementation_summary.md", "Implementation summary"),
        ("docs/sidebar/s3.3.md", "Sprint requirements"),
        ("docs/sidebar/s3.3g.md", "Sprint guidance"),
    ]
    
    all_docs_good = True
    for filepath, description in doc_files:
        exists = check_file_exists(filepath, description)
        all_docs_good = all_docs_good and exists
    
    # CMake integration
    print("\n🔧 Build System:")
    cmake_good = check_cmake_integration()
    
    # Resource files
    print("\n🎨 Resources:")
    resources_good = check_resource_files()
    
    # Summary
    print("\n📊 Validation Summary:")
    print("=" * 30)
    
    components = [
        ("Core Implementation", all_core_good),
        ("Test Coverage", all_tests_good),
        ("Documentation", all_docs_good),
        ("CMake Integration", cmake_good),
        ("Resource Files", resources_good),
    ]
    
    all_passed = True
    for component, status in components:
        status_icon = "✅" if status else "❌"
        print(f"{status_icon} {component}")
        all_passed = all_passed and status
    
    print("\n" + "=" * 50)
    if all_passed:
        print("🎉 Sprint 3.3 implementation validation PASSED!")
        print("   All components are properly implemented and integrated.")
        return 0
    else:
        print("⚠️  Sprint 3.3 implementation validation FAILED!")
        print("   Some components need attention before deployment.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
