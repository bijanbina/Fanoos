#include "fa_transmitter.h"

#define JOYSTICK_DELAY 100

FaTransmitter::FaTransmitter(int port,
               QObject *parent): QObject(parent)
{
    //init
    server = new QTcpServer;
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
    int new_con_id = fa_cons.length();
    qDebug() << "Accepted connection "
             << new_con_id;

    QTcpSocket *con = server->nextPendingConnection();
    FaConnection *connection = new FaConnection(con, new_con_id);
    connect(connection, SIGNAL(clientReadyRead(QString, int)),
            this, SLOT(readyRead(QString, int)));
    connect(connection, SIGNAL(clientDisconnected(int)),
            this, SLOT(handleDisconnect(int)));
    connect(connection, SIGNAL(clientConnected(int)),
            this, SLOT(handleConnect(int)));
    fa_cons.push_back(connection);
}

void FaTransmitter::readyRead(QString data, int id)
{
    ;
}

void FaTransmitter::displayError(QAbstractSocket::SocketError socketError)
 {
     if( socketError==QTcpSocket::RemoteHostClosedError )
     {
         return;
     }

     qDebug() <<  QString("Error Happened");
}

void FaTransmitter::dataReady(QString data)
{
    QByteArray data_b(data.toStdString().c_str());
    int len = fa_cons.length();

    for( int i = 0; i<len; i++ )
    {
        fa_cons[i]->write(QString(data_b));
    }

    QString msg = "broadcast:";
    qDebug() << msg << data;
}

void FaTransmitter::handleDisconnect(int id)
{
    ;
}

void FaTransmitter::handleConnect(int id)
{
    ;
}
