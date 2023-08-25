#include "fa_channel.h"
#include <string.h>
#include <shellapi.h>

#define JOYSTICK_DELAY 100

FaChannel::FaChannel(int port, QObject *parent): QObject(parent)
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

FaChannel::~FaChannel()
{
    qDebug() << "Closing Server";
    server->close();
}

void FaChannel::acceptConnection()
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

void FaChannel::readyRead(int id)
{
    QByteArray data = cons[id]->readAll();
    QString msg = "rx data[";
    msg += QString::number(id);
    msg += "]:";
    qDebug() << msg << data;
}

void FaChannel::displayError(QAbstractSocket::SocketError socketError)
 {
     if (socketError == QTcpSocket::RemoteHostClosedError)
         return;

     qDebug() <<  QString("Error Happened");
}
