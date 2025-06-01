#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include "../../src/e57parserlib.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Sprint 2.1 Manual Test ===";
    
    // Test 1: BitPack codec identification (explicit)
    qDebug() << "\n--- Test 1: BitPack Codec Identification (Explicit) ---";
    
    QString testXml1 = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <bitPackCodec/>
                </vector>
            </codecs>
        </points>
    )";
    
    QDomDocument doc1;
    if (doc1.setContent(testXml1)) {
        E57Parser parser1;
        QDomElement pointsElement1 = doc1.documentElement();
        bool result1 = parser1.parseData3D(pointsElement1);
        
        qDebug() << "Test 1 Result:" << (result1 ? "PASS" : "FAIL");
        if (!result1) {
            qDebug() << "Error:" << parser1.getLastError();
        }
    }
    
    // Test 2: BitPack codec identification (default)
    qDebug() << "\n--- Test 2: BitPack Codec Identification (Default) ---";
    
    QString testXml2 = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <!-- Empty vector = default bitPackCodec -->
                </vector>
            </codecs>
        </points>
    )";
    
    QDomDocument doc2;
    if (doc2.setContent(testXml2)) {
        E57Parser parser2;
        QDomElement pointsElement2 = doc2.documentElement();
        bool result2 = parser2.parseData3D(pointsElement2);
        
        qDebug() << "Test 2 Result:" << (result2 ? "PASS" : "FAIL");
        if (!result2) {
            qDebug() << "Error:" << parser2.getLastError();
        }
    }
    
    // Test 3: Unsupported codec rejection
    qDebug() << "\n--- Test 3: Unsupported Codec Rejection ---";
    
    QString testXml3 = R"(
        <points type="CompressedVector" recordCount="100">
            <prototype>
                <cartesianX type="Float" precision="single"/>
                <cartesianY type="Float" precision="single"/>
                <cartesianZ type="Float" precision="single"/>
            </prototype>
            <codecs>
                <vector>
                    <zLibCodec/>
                </vector>
            </codecs>
        </points>
    )";
    
    QDomDocument doc3;
    if (doc3.setContent(testXml3)) {
        E57Parser parser3;
        QDomElement pointsElement3 = doc3.documentElement();
        bool result3 = parser3.parseData3D(pointsElement3);
        
        qDebug() << "Test 3 Result:" << (!result3 ? "PASS" : "FAIL") << "(Should fail)";
        if (!result3) {
            qDebug() << "Expected error:" << parser3.getLastError();
        }
    }
    
    // Test 4: Field descriptor parsing
    qDebug() << "\n--- Test 4: Field Descriptor Parsing ---";
    
    QString testXml4 = R"(
        <points type="CompressedVector" recordCount="50">
            <prototype>
                <cartesianX type="Float" precision="single" minimum="-10.0" maximum="10.0"/>
                <cartesianY type="Float" precision="double" minimum="-5.0" maximum="5.0"/>
                <cartesianZ type="ScaledInteger" precision="16" scale="0.001" offset="100.0"/>
            </prototype>
            <codecs>
                <vector>
                    <bitPackCodec/>
                </vector>
            </codecs>
        </points>
    )";
    
    QDomDocument doc4;
    if (doc4.setContent(testXml4)) {
        E57Parser parser4;
        QDomElement pointsElement4 = doc4.documentElement();
        bool result4 = parser4.parseData3D(pointsElement4);
        
        qDebug() << "Test 4 Result:" << (result4 ? "PASS" : "FAIL");
        if (!result4) {
            qDebug() << "Error:" << parser4.getLastError();
        }
    }
    
    qDebug() << "\n=== Sprint 2.1 Manual Test Complete ===";
    
    return 0;
}
