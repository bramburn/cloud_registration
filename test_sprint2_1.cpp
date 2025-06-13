// Test file to verify Sprint 2.1 implementation
#include <QApplication>
#include <QDebug>

// Test includes to verify all headers compile
#include "app/MainPresenter.h"
#include "ui/AlignmentControlPanel.h"
#include "registration/TargetManager.h"
#include "registration/AlignmentEngine.h"
#include "interfaces/IMainView.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Sprint 2.1 Implementation Test";
    qDebug() << "Testing header includes...";
    
    // Test that all classes can be instantiated
    try {
        AlignmentEngine engine;
        TargetManager targetManager;
        AlignmentControlPanel panel;
        
        qDebug() << "All classes instantiated successfully";
        qDebug() << "Sprint 2.1 implementation appears to be working";
        
        return 0;
    } catch (...) {
        qDebug() << "Error instantiating classes";
        return 1;
    }
}
