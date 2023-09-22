#include "fa_receiver.h"

FaReceiver::FaReceiver(int port, QObject *parent): QObject(parent)
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

FaReceiver::~FaReceiver()
{
    qDebug() << "Closing Server";
    server->close();
}

void FaReceiver::acceptConnection()
{
    int new_con_id = fa_cons.length();
    qDebug() << "Accepted connection "
             << new_con_id;

    QTcpSocket *con = server->nextPendingConnection();
    FaConnection *connection = new FaConnection(con, new_con_id);
    connect(connection, SIGNAL(clientReadyRead(QString, int)),
            this, SLOT(readyRead(QString, int)));
    connect(connection, SIGNAL(clientConnected(int)),
            this, SLOT(handleConnect(int)));
    connect(connection, SIGNAL(clientDisconnected(int)),
            this, SLOT(handleDisconnect(int)));
    fa_cons.push_back(connection);
}

void FaReceiver::readyRead(QString data, int id)
{
    QString data_str = data;
    emit dataReady(data_str);
}

void FaReceiver::displayError(QAbstractSocket::SocketError socketError)
 {
     if( socketError==QTcpSocket::RemoteHostClosedError )
     {
         return;
     }

     qDebug() <<  QString("Error Happened");
}

void FaReceiver::handleDisconnect(int id)
{

}

void FaReceiver::handleConnect(int id)
{

}
