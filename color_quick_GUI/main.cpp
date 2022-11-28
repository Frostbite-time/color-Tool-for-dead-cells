#include "color_quick_GUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    color_quick_GUI w;
    w.show();
    return a.exec();
}
