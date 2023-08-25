#include "fa_transmitter.h"

#define JOYSTICK_DELAY 100

FaTransmitter::FaTransmitter(int port,
               QObject *parent): QObject(parent)
{
    //init
    server = new QTcpServer;
    signalMapper = new QSignalMapper(this);
    connect(server, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    //server->listen(QHostAddress("127.0.0.1"), RE_PORT);
    if( server->listen(QHostAddress::Any, port) )
    {
        qDebug() << "created on port " << port;
    }
    else
    {
        qDebug() << "Server failed";
        qDebug() << "Error message is:" << server->errorString();
    }
}

FaTransmitter::~FaTransmitter()
{
    qDebug() << "Closing Server";
    server->close();
}

void FaTransmitter::acceptConnection()
{
    qDebug() << "Accepted connection "
             << cons.length();

    QTcpSocket *con = server->nextPendingConnection();
    con->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    cons.push_back(con);

    int id = cons.length() - 1;
    connect(con, SIGNAL(readyRead()), signalMapper, SLOT(map()));
    signalMapper->setMapping(con, id);
    connect(signalMapper, SIGNAL(mapped(int)),
             this, SLOT(readyRead(int)));
    connect(con, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    emit clientConnected();
}

void FaTransmitter::readyRead(int id)
{
    id;
}

void FaTransmitter::displayError(QAbstractSocket::SocketError socketError)
 {
     if (socketError == QTcpSocket::RemoteHostClosedError)
         return;

     qDebug() <<  QString("Error Happened");
}

void FaTransmitter::dataReady(QString data)
{
    QByteArray data_b(data.toStdString().c_str());
    int len = cons.length();

    for( int i = 0; i<len; i++ )
    {
        cons[i]->write(data_b);
    }

    QString msg = "broadcast:";
    qDebug() << msg << data;
}
