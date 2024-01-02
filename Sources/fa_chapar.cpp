#include "fa_chapar.h"

FaChapar::FaChapar(QObject *parent): QObject(parent)
{
    receiver    = new FaApacheSe("rx");
    transmitter = new FaApacheSe("tx");
    connect(receiver, SIGNAL(dataReady(int, QString)),
            this, SLOT(dataReady(int, QString)));

    receiver->bind(5000);
    transmitter->bind(5001);
}

void FaChapar::dataReady(int id, QString data)
{
    (void)id; //to suppress unused warning
    int len = transmitter->cons.size();
    for( int i=0 ; i<len ; i++ )
    {
        transmitter->write(i, data);
    }
}

FaChapar::~FaChapar()
{
}
