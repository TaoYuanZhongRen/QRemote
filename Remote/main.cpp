#include "Remote.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Remote w;
    w.show();
    return a.exec();
}
