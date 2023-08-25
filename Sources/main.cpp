#include <QGuiApplication>
#include "fa_channel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    FaChannel *channel = new FaChannel(5000);

    return app.exec();
}
