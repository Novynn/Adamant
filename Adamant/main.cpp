#include "adamant.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Adamant app;
    app.start();

    return a.exec();
}
