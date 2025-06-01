#!/usr/bin/env python3
"""
Script to create test E57 files for Sprint 1.2 testing.
This creates minimal E57 files with CompressedVector structures for testing.
"""

import struct
import os

def create_e57_header():
    """Create a basic E57 header"""
    header = bytearray()
    
    # File signature: "ASTM-E57" (8 bytes)
    header.extend(b"ASTM-E57")
    
    # Major version (4 bytes, little-endian)
    header.extend(struct.pack('<I', 1))
    
    # Minor version (4 bytes, little-endian)  
    header.extend(struct.pack('<I', 0))
    
    # File physical length (8 bytes, little-endian) - will be updated
    header.extend(struct.pack('<Q', 0))
    
    # XML offset (8 bytes, little-endian) - starts after header
    xml_offset = 48  # Header size
    header.extend(struct.pack('<Q', xml_offset))
    
    # XML length (8 bytes, little-endian) - will be calculated
    header.extend(struct.pack('<Q', 0))
    
    # Page size (8 bytes, little-endian)
    header.extend(struct.pack('<Q', 1024))
    
    return header

def create_compressed_vector_xml():
    """Create XML with CompressedVector structure"""
    xml = '''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{12345678-1234-1234-1234-123456789012}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <e57LibraryVersion type="String">Sprint1.2-Test</e57LibraryVersion>
    <coordinateMetadata type="String">Test coordinate system</coordinateMetadata>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String">{87654321-4321-4321-4321-210987654321}</guid>
            <name type="String">Test CompressedVector Scan</name>
            <description type="String">Test scan with CompressedVector structure</description>
            <points type="CompressedVector" fileOffset="0" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <vectorChild type="CompressedVectorNode" recordCount="3" fileOffset="2048">
                        <prototype type="Structure">
                            <cartesianX type="Float" precision="single"/>
                            <cartesianY type="Float" precision="single"/>
                            <cartesianZ type="Float" precision="single"/>
                        </prototype>
                    </vectorChild>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>'''
    return xml.encode('utf-8')

def create_malformed_compressed_vector_xml():
    """Create XML with malformed CompressedVector structure"""
    xml = '''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{12345678-1234-1234-1234-123456789012}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String">{87654321-4321-4321-4321-210987654321}</guid>
            <name type="String">Test Malformed CompressedVector</name>
            <points type="CompressedVector" fileOffset="0">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <vectorChild type="CompressedVectorNode">
                        <!-- Missing recordCount attribute -->
                        <prototype type="Structure">
                            <cartesianX type="Float" precision="single"/>
                            <cartesianY type="Float" precision="single"/>
                            <!-- Missing cartesianZ -->
                        </prototype>
                    </vectorChild>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>'''
    return xml.encode('utf-8')

def create_point_data():
    """Create binary point data (3 points: (1,2,3), (4,5,6), (7,8,9))"""
    points = [
        (1.0, 2.0, 3.0),
        (4.0, 5.0, 6.0),
        (7.0, 8.0, 9.0)
    ]
    
    data = bytearray()
    for x, y, z in points:
        data.extend(struct.pack('<fff', x, y, z))
    
    return data

def create_e57_file(filename, xml_data, point_data):
    """Create a complete E57 file"""
    header = create_e57_header()
    
    # Calculate offsets and sizes
    xml_offset = len(header)
    xml_length = len(xml_data)
    binary_offset = xml_offset + xml_length
    
    # Pad to align binary data
    padding_needed = (1024 - (binary_offset % 1024)) % 1024
    padding = b'\x00' * padding_needed
    binary_offset += padding_needed
    
    total_length = binary_offset + len(point_data)
    
    # Update header with correct values
    struct.pack_into('<Q', header, 24, total_length)  # File physical length
    struct.pack_into('<Q', header, 40, xml_length)    # XML length
    
    # Write file
    with open(filename, 'wb') as f:
        f.write(header)
        f.write(xml_data)
        f.write(padding)
        f.write(point_data)
    
    print(f"Created {filename} ({total_length} bytes)")
    print(f"  Header: {len(header)} bytes")
    print(f"  XML: {xml_length} bytes at offset {xml_offset}")
    print(f"  Binary: {len(point_data)} bytes at offset {binary_offset}")

def main():
    """Create test E57 files"""
    os.makedirs('test_data', exist_ok=True)
    
    # Create CompressedVector test file
    xml_data = create_compressed_vector_xml()
    point_data = create_point_data()
    create_e57_file('test_data/compressedvector_uncompressed_data.e57', xml_data, point_data)
    
    # Create malformed CompressedVector test file
    malformed_xml = create_malformed_compressed_vector_xml()
    create_e57_file('test_data/malformed_compressedvector.e57', malformed_xml, point_data)
    
    print("\nTest files created successfully!")
    print("Use these files to test Sprint 1.2 CompressedVector parsing functionality.")

if __name__ == '__main__':
    main()
