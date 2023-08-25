#include <QCoreApplication>
#include "fa_chapar.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    FaChapar *chapar = new FaChapar();

    return app.exec();
}
