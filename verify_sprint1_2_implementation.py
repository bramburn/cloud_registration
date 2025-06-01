#!/usr/bin/env python3
"""
Sprint 1.2 Implementation Verification Script
Checks that all required files and integrations are in place.
"""

import os
import sys
import re
from pathlib import Path

def check_file_exists(filepath, description=""):
    """Check if a file exists and report status."""
    if os.path.exists(filepath):
        print(f"✅ {filepath} {description}")
        return True
    else:
        print(f"❌ {filepath} {description} - MISSING")
        return False

def check_file_contains(filepath, pattern, description=""):
    """Check if a file contains a specific pattern."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            if re.search(pattern, content, re.MULTILINE):
                print(f"✅ {filepath} contains {description}")
                return True
            else:
                print(f"❌ {filepath} missing {description}")
                return False
    except Exception as e:
        print(f"❌ Error reading {filepath}: {e}")
        return False

def main():
    print("🔍 Sprint 1.2 Implementation Verification")
    print("=" * 50)
    
    all_checks_passed = True
    
    # Check core Sprint 1.2 files
    print("\n📁 Core Files Check:")
    core_files = [
        ("src/sqlitemanager.h", "SQLite manager header"),
        ("src/sqlitemanager.cpp", "SQLite manager implementation"),
        ("src/scanimportmanager.h", "Scan import manager header"),
        ("src/scanimportmanager.cpp", "Scan import manager implementation"),
        ("src/scanimportdialog.h", "Scan import dialog header"),
        ("src/scanimportdialog.cpp", "Scan import dialog implementation"),
        ("src/projecttreemodel.h", "Project tree model header"),
        ("src/projecttreemodel.cpp", "Project tree model implementation"),
    ]
    
    for filepath, description in core_files:
        if not check_file_exists(filepath, description):
            all_checks_passed = False
    
    # Check CMakeLists.txt integration
    print("\n🔧 CMakeLists.txt Integration:")
    cmake_checks = [
        (r"src/sqlitemanager\.cpp", "SQLiteManager source"),
        (r"src/scanimportmanager\.cpp", "ScanImportManager source"),
        (r"src/scanimportdialog\.cpp", "ScanImportDialog source"),
        (r"src/projecttreemodel\.cpp", "ProjectTreeModel source"),
        (r"Qt6::Sql", "Qt6 SQL dependency"),
    ]
    
    for pattern, description in cmake_checks:
        if not check_file_contains("CMakeLists.txt", pattern, description):
            all_checks_passed = False
    
    # Check ProjectManager enhancements
    print("\n🏗️ ProjectManager Integration:")
    pm_checks = [
        (r"class SQLiteManager", "SQLiteManager forward declaration"),
        (r"class ScanImportManager", "ScanImportManager forward declaration"),
        (r"struct ScanInfo", "ScanInfo struct definition"),
        (r"SQLiteManager.*m_sqliteManager", "SQLiteManager member"),
        (r"ScanImportManager.*m_scanImportManager", "ScanImportManager member"),
        (r"getDatabasePath", "getDatabasePath method"),
        (r"getScansSubfolder", "getScansSubfolder method"),
    ]
    
    for pattern, description in pm_checks:
        if not check_file_contains("src/projectmanager.h", pattern, description):
            all_checks_passed = False
    
    # Check MainWindow enhancements
    print("\n🖥️ MainWindow Integration:")
    mw_checks = [
        (r"class ScanImportDialog", "ScanImportDialog forward declaration"),
        (r"class SQLiteManager", "SQLiteManager forward declaration"),
        (r"m_importScansAction", "Import scans action"),
        (r"onImportScans", "Import scans slot"),
        (r"onScansImported", "Scans imported slot"),
        (r"showImportGuidance", "Import guidance method"),
    ]
    
    # Check header file for forward declarations
    header_checks = [
        (r"class ScanImportDialog", "ScanImportDialog forward declaration"),
        (r"class SQLiteManager", "SQLiteManager forward declaration"),
        (r"m_importScansAction", "Import scans action"),
        (r"onImportScans", "Import scans slot"),
        (r"onScansImported", "Scans imported slot"),
        (r"showImportGuidance", "Import guidance method"),
    ]

    # Check implementation file for includes and usage
    impl_checks = [
        (r"#include.*scanimportdialog", "ScanImportDialog include"),
        (r"#include.*sqlitemanager", "SQLiteManager include"),
        (r"onImportScans", "Import scans implementation"),
        (r"onScansImported", "Scans imported implementation"),
        (r"showImportGuidance", "Import guidance implementation"),
    ]

    for pattern, description in header_checks:
        if not check_file_contains("src/mainwindow.h", pattern, description):
            all_checks_passed = False

    for pattern, description in impl_checks:
        if not check_file_contains("src/mainwindow.cpp", pattern, description):
            all_checks_passed = False
    
    # Check SidebarWidget enhancements
    print("\n📋 SidebarWidget Integration:")
    sw_checks = [
        (r"ProjectTreeModel", "ProjectTreeModel usage"),
        (r"setSQLiteManager", "setSQLiteManager method"),
        (r"refreshFromDatabase", "refreshFromDatabase method"),
        (r"addScan", "addScan method"),
    ]
    
    for pattern, description in sw_checks:
        if not check_file_contains("src/sidebarwidget.h", pattern, description):
            all_checks_passed = False
    
    # Check database schema
    print("\n🗄️ Database Schema:")
    schema_checks = [
        (r"CREATE TABLE.*scans", "Scans table creation"),
        (r"scan_id.*PRIMARY KEY", "Primary key definition"),
        (r"import_type.*CHECK", "Import type constraint"),
        (r"UNIQUE.*file_path_relative", "Unique file path constraint"),
    ]
    
    for pattern, description in schema_checks:
        if not check_file_contains("src/sqlitemanager.cpp", pattern, description):
            all_checks_passed = False
    
    # Check test files
    print("\n🧪 Test Files:")
    test_files = [
        ("test_sprint1_2.ps1", "PowerShell test script"),
        ("SPRINT_1_2_IMPLEMENTATION_SUMMARY.md", "Implementation summary"),
    ]
    
    for filepath, description in test_files:
        if not check_file_exists(filepath, description):
            all_checks_passed = False
    
    # Summary
    print("\n" + "=" * 50)
    if all_checks_passed:
        print("🎉 All Sprint 1.2 implementation checks PASSED!")
        print("\n✅ Ready for testing and deployment")
        print("\nNext steps:")
        print("1. Build the project: cmake .. && make")
        print("2. Run manual tests with the application")
        print("3. Test scan import functionality")
        print("4. Verify database integration")
    else:
        print("❌ Some Sprint 1.2 implementation checks FAILED!")
        print("\n🔧 Please review and fix the missing components")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
