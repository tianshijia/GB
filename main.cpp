#include "bgtest.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BGTEST w;
    w.show();

    return a.exec();
}
