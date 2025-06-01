#!/usr/bin/env python3
"""
Script to create test E57 files for Sprint 1.2 testing.
Creates E57 files with CompressedVector structures containing uncompressed XYZ data.
"""

import struct
import os

def create_e57_header():
    """Create a basic E57 header."""
    header = bytearray()
    
    # File signature: "ASTM-E57"
    header.extend(b"ASTM-E57")
    
    # Major version (4 bytes, little-endian)
    header.extend(struct.pack('<I', 1))
    
    # Minor version (4 bytes, little-endian)
    header.extend(struct.pack('<I', 0))
    
    # File physical length (8 bytes, little-endian) - will be updated later
    header.extend(struct.pack('<Q', 0))
    
    # XML offset (8 bytes, little-endian) - starts after header
    xml_offset = 48  # Size of header
    header.extend(struct.pack('<Q', xml_offset))
    
    # XML length (8 bytes, little-endian) - will be calculated
    header.extend(struct.pack('<Q', 0))
    
    # Page size (8 bytes, little-endian)
    header.extend(struct.pack('<Q', 1024))
    
    return header, xml_offset

def create_compressed_vector_xml():
    """Create XML content for CompressedVector with uncompressed data."""
    xml_content = '''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String"><![CDATA[ASTM E57 3D Imaging Data File]]></formatName>
    <guid type="String"><![CDATA[{12345678-1234-5678-9ABC-123456789ABC}]]></guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <e57LibraryVersion type="String"><![CDATA[Test Library 1.0]]></e57LibraryVersion>
    <coordinateMetadata type="String"><![CDATA[Test coordinate metadata]]></coordinateMetadata>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String"><![CDATA[{87654321-4321-8765-CBA9-987654321CBA}]]></guid>
            <name type="String"><![CDATA[Test Point Cloud]]></name>
            <description type="String"><![CDATA[Test point cloud with CompressedVector]]></description>
            <points type="CompressedVector" fileOffset="''' + str(48 + 1200) + '''" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single" minimum="-1000.0" maximum="1000.0"/>
                    <cartesianY type="Float" precision="single" minimum="-1000.0" maximum="1000.0"/>
                    <cartesianZ type="Float" precision="single" minimum="-1000.0" maximum="1000.0"/>
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <vectorChild type="CompressedVectorNode" recordCount="3" fileOffset="''' + str(48 + 1200) + '''">
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
    return xml_content.encode('utf-8')

def create_malformed_xml():
    """Create malformed XML content for error testing."""
    xml_content = '''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String"><![CDATA[ASTM E57 3D Imaging Data File]]></formatName>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <points type="CompressedVector" fileOffset="''' + str(48 + 800) + '''" recordCount="invalid_count">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <!-- Missing cartesianY and cartesianZ -->
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <!-- Missing CompressedVectorNode -->
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>'''
    return xml_content.encode('utf-8')

def create_point_data():
    """Create binary point data (3 points: (1,2,3), (4,5,6), (7,8,9))."""
    points = [
        1.0, 2.0, 3.0,  # First point
        4.0, 5.0, 6.0,  # Second point
        7.0, 8.0, 9.0   # Third point
    ]
    
    data = bytearray()
    for point in points:
        data.extend(struct.pack('<f', point))  # Little-endian single precision float
    
    return data

def create_e57_file(filename, xml_content, point_data):
    """Create a complete E57 file."""
    header, xml_offset = create_e57_header()
    
    # Calculate XML length
    xml_length = len(xml_content)
    
    # Calculate binary data offset (after XML, aligned to 8-byte boundary)
    binary_offset = xml_offset + xml_length
    if binary_offset % 8 != 0:
        binary_offset += 8 - (binary_offset % 8)
    
    # Calculate total file length
    file_length = binary_offset + len(point_data)
    
    # Update header with correct lengths
    # File physical length at offset 16
    header[16:24] = struct.pack('<Q', file_length)
    # XML length at offset 32
    header[32:40] = struct.pack('<Q', xml_length)
    
    # Write the file
    with open(filename, 'wb') as f:
        # Write header
        f.write(header)
        
        # Write XML content
        f.write(xml_content)
        
        # Pad to binary offset
        current_pos = f.tell()
        if current_pos < binary_offset:
            f.write(b'\x00' * (binary_offset - current_pos))
        
        # Write binary point data
        f.write(point_data)
    
    print(f"Created {filename} ({file_length} bytes)")
    print(f"  Header: 48 bytes")
    print(f"  XML: {xml_length} bytes at offset {xml_offset}")
    print(f"  Binary data: {len(point_data)} bytes at offset {binary_offset}")

def main():
    """Create test E57 files for Sprint 1.2."""
    os.makedirs('test_data', exist_ok=True)
    
    # Create valid E57 file with CompressedVector and uncompressed data
    print("Creating compressedvector_uncompressed_data.e57...")
    xml_content = create_compressed_vector_xml()
    point_data = create_point_data()
    create_e57_file('test_data/compressedvector_uncompressed_data.e57', xml_content, point_data)
    
    # Create malformed E57 file for error testing
    print("\nCreating malformed_compressedvector.e57...")
    malformed_xml = create_malformed_xml()
    create_e57_file('test_data/malformed_compressedvector.e57', malformed_xml, point_data)
    
    # Create the existing test file for compatibility
    print("\nCreating test_real_points.e57...")
    create_e57_file('test_data/test_real_points.e57', xml_content, point_data)
    
    print("\nTest E57 files created successfully!")

if __name__ == '__main__':
    main()
