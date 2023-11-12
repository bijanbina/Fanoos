#include "fa_chapar.h"

FaChapar::FaChapar(QObject *parent): QObject(parent)
{
    //init
    receiver = new FaApacheSe;
    transmitter = new FaApacheSe;
    connect(receiver, SIGNAL(dataReady(int, QString)),
            this, SLOT(dataReady(int, QString)));

    receiver->bind(5000);
    transmitter->bind(5001);
}

void FaChapar::dataReady(int id, QString data)
{
    int len = transmitter->cons.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( transmitter->cons[i]->isOpen() )
        {
            transmitter->write(i, data);
        }
    }
}

FaChapar::~FaChapar()
{
    qDebug() << "Closing Server";
}
