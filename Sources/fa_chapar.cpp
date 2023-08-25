#include "fa_chapar.h"

FaChapar::FaChapar(QObject *parent): QObject(parent)
{
    //init
    receiver = new FaReceiver(5000);
    transmitter = new FaTransmitter(5001);
    connect(receiver, SIGNAL(dataReady(QString)),
            transmitter, SLOT(dataReady(QString)));
}

FaChapar::~FaChapar()
{
    qDebug() << "Closing Server";
}

