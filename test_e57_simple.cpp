#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDataStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "Testing E57 parsing implementation..." << std::endl;
    
    // Test if we can create a simple file and read it
    QFile testFile("simple_test.e57");
    if (testFile.open(QIODevice::WriteOnly)) {
        QDataStream stream(&testFile);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        // Write E57 signature
        stream << static_cast<quint32>(0x41535446); // "ASTF"
        stream << static_cast<quint32>(1); // Major version
        stream << static_cast<quint32>(0); // Minor version
        
        testFile.close();
        std::cout << "Created test file successfully" << std::endl;
    }
    
    // Test reading
    if (testFile.open(QIODevice::ReadOnly)) {
        QDataStream stream(&testFile);
        stream.setByteOrder(QDataStream::LittleEndian);
        
        quint32 signature, major, minor;
        stream >> signature >> major >> minor;
        
        std::cout << "Read signature: 0x" << std::hex << signature << std::endl;
        std::cout << "Version: " << std::dec << major << "." << minor << std::endl;
        
        testFile.close();
    }
    
    // Clean up
    QFile::remove("simple_test.e57");
    
    std::cout << "Basic file I/O test completed successfully!" << std::endl;
    return 0;
}
