#!/usr/bin/env python3
"""
Create test E57 files for Sprint 2.1 codec testing - FIXED VERSION
"""

import struct
import os

def create_e57_with_bitpack_codec():
    """Create a simple E57 file with bitPackCodec for testing"""
    
    # E57 Header (48 bytes)
    signature = b"ASTM-E57"  # 8 bytes
    major_version = 1         # 4 bytes
    minor_version = 0         # 4 bytes
    file_physical_length = 0  # Will be set later, 8 bytes
    xml_offset = 48           # 8 bytes - right after header
    xml_length = 0            # Will be set later, 8 bytes
    page_size = 1024          # 8 bytes
    
    # Calculate binary offset first
    # Estimate XML size and align to page boundary
    estimated_xml_size = 1600  # Conservative estimate
    binary_offset = xml_offset + estimated_xml_size
    binary_offset = ((binary_offset + page_size - 1) // page_size) * page_size
    
    # XML content for bitPackCodec with proper structure
    xml_content = f'''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{{12345678-1234-1234-1234-123456789ABC}}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <e57LibraryVersion type="String">Sprint2.1-Test</e57LibraryVersion>
    <coordinateMetadata type="String">Test coordinate system</coordinateMetadata>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String">{{87654321-4321-4321-4321-210987654321}}</guid>
            <name type="String">Test Point Cloud</name>
            <description type="String">Test point cloud data for Sprint 2.1</description>
            <points type="CompressedVector" fileOffset="{binary_offset}" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector">
                    <vector type="CompressedVectorNode">
                        <bitPackCodec type="Structure"/>
                        <recordCount type="Integer">3</recordCount>
                        <binarySection type="String">test_binary_section</binarySection>
                    </vector>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>'''
    
    xml_bytes = xml_content.encode('utf-8')
    xml_length = len(xml_bytes)
    
    # Test point data (3 points, XYZ as single-precision floats)
    # Point 1: (1.0, 2.0, 3.0)
    # Point 2: (4.0, 5.0, 6.0)  
    # Point 3: (7.0, 8.0, 9.0)
    point_data = struct.pack('<9f', 
                            1.0, 2.0, 3.0,  # Point 1
                            4.0, 5.0, 6.0,  # Point 2
                            7.0, 8.0, 9.0)  # Point 3
    
    # Recalculate binary offset with actual XML size
    binary_offset = xml_offset + xml_length
    # Align to page boundary
    binary_offset = ((binary_offset + page_size - 1) // page_size) * page_size
    file_physical_length = binary_offset + len(point_data)
    
    # Update XML with correct binary offset
    xml_content = xml_content.replace(f'fileOffset="{xml_offset + estimated_xml_size}"', 
                                     f'fileOffset="{binary_offset}"')
    xml_bytes = xml_content.encode('utf-8')
    xml_length = len(xml_bytes)
    
    # Create the file
    filename = "test_data/e57_bitpack_codec_test_fixed.e57"
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    
    with open(filename, 'wb') as f:
        # Write header
        f.write(signature)
        f.write(struct.pack('<I', major_version))
        f.write(struct.pack('<I', minor_version))
        f.write(struct.pack('<Q', file_physical_length))
        f.write(struct.pack('<Q', xml_offset))
        f.write(struct.pack('<Q', xml_length))
        f.write(struct.pack('<Q', page_size))
        
        # Write XML
        f.write(xml_bytes)
        
        # Pad to binary offset
        current_pos = f.tell()
        padding_needed = binary_offset - current_pos
        if padding_needed > 0:
            f.write(b'\x00' * padding_needed)
        
        # Write point data
        f.write(point_data)
    
    print(f"Created test E57 file: {filename}")
    print(f"File size: {file_physical_length} bytes")
    print(f"XML offset: {xml_offset}, length: {xml_length}")
    print(f"Binary offset: {binary_offset}, length: {len(point_data)}")
    print("Test points: (1,2,3), (4,5,6), (7,8,9)")

def create_e57_with_unsupported_codec():
    """Create an E57 file with unsupported codec for testing error handling"""
    
    # Similar structure but with zLibCodec
    xml_content = '''<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <points type="CompressedVector" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector">
                    <vector type="CompressedVectorNode">
                        <zLibCodec type="Structure"/>
                        <recordCount type="Integer">3</recordCount>
                        <binarySection type="String">test_binary_section</binarySection>
                    </vector>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>'''
    
    filename = "test_data/e57_unsupported_codec_test_fixed.e57"
    
    # Create minimal file structure
    signature = b"ASTM-E57"
    xml_bytes = xml_content.encode('utf-8')
    
    with open(filename, 'wb') as f:
        # Write header
        f.write(signature)
        f.write(struct.pack('<I', 1))  # major version
        f.write(struct.pack('<I', 0))  # minor version
        f.write(struct.pack('<Q', 48 + len(xml_bytes)))  # file length
        f.write(struct.pack('<Q', 48))  # xml offset
        f.write(struct.pack('<Q', len(xml_bytes)))  # xml length
        f.write(struct.pack('<Q', 1024))  # page size
        
        # Write XML
        f.write(xml_bytes)
    
    print(f"Created unsupported codec test file: {filename}")

if __name__ == "__main__":
    create_e57_with_bitpack_codec()
    create_e57_with_unsupported_codec()
    print("Fixed test E57 files created successfully!")
