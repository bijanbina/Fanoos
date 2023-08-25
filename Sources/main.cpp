#include <QGuiApplication>
#include "fa_chapar.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    FaChapar *chapar = new FaChapar();

    return app.exec();
}
