#include "mainwindow.h"

#include <QApplication>
//#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    try{
    MainWindow w;
    w.show();
//    }
//    catch (const std::exception& e) {
//        QMessageBox messageBox;
//        messageBox.critical(0,"Error",e.what());
//        messageBox.setFixedSize(500,200);
//    }
    return a.exec();
}
