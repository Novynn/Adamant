#include "adamant.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QtMsgType>("QtMsgType");

    Adamant app;
    if (app.start()) {
        a.exec();
    }

    return 0;
}
