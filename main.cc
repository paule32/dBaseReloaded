#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // ---------------------------------
    // IMPORTANT: set plugin path's ...
    // ---------------------------------  
    setenv("QT_QPA_PLATFORM_PLUGIN_PATH",
    "./plugins", 1);
    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
