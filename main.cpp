#include <QtGui/QApplication>
#include "mt500.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MT500 w;
    w.show();
    
    return a.exec();
}
