#include "adamant.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Adamant app;
    if (app.start()) {
        a.exec();
    }

    return 0;
}
