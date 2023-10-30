#include "fa_connection.h"

#define JOYSTICK_DELAY 100

FaConnection::FaConnection(QTcpSocket *con, int con_id,
               QObject *parent): QObject(parent)
{
    id = con_id;
    con->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connection = con;

    connect(connection, SIGNAL(readyRead()),
            this, SLOT(readyRead()));
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(disconnected()),
            this, SLOT(handleDisconnect()));

    live = new QTimer;
    watchdog = new QTimer;

    connect(live, SIGNAL(timeout()),
            this, SLOT(liveTimeout()));
    connect(watchdog, SIGNAL(timeout()),
            this, SLOT(watchdogTimeout()));

    live->start(FA_LIVE);
    watchdog->start(FA_WATCHDOG);

    emit clientConnected(id);
}

FaConnection::~FaConnection()
{
    if( connection )
    {
        if( connection->isOpen() )
        {
            connection->close();
        }
    }
}

void FaConnection::displayError(QAbstractSocket::SocketError socketError)
 {
     if( socketError==QTcpSocket::RemoteHostClosedError )
     {
         return;
     }

     qDebug() << QString("FaConnection::Error Happened");
}

void FaConnection::write(QString data)
{
    if( connection )
    {
        if( connection->isOpen() )
        {
            live->start(FA_LIVE);//don't send live
            QByteArray data_b(data.toStdString().c_str());
            connection->write(data_b);
            connection->waitForBytesWritten(50);
            live->start(FA_LIVE);//don't send live
        }
    }
}

void FaConnection::handleDisconnect()
{
    watchdog->stop();
    live->stop();
    connection->close();
    emit clientDisconnected(id);
}

// client lost, drop connection and reconnect
void FaConnection::watchdogTimeout()
{
    if( connection->isOpen() )
    {
        qDebug() << "Remote: connection dropped:"
                 << connection->state();
        handleDisconnect();
    }
    else
    {
        qDebug() << "Remote: watchdog, tcpClient is closed";
    }
}

// keep client alive
void FaConnection::liveTimeout()
{
    if( connection->isOpen() )
    {
        if( connection->state()==QAbstractSocket::ConnectedState )
        {
            int byte_count = connection->write("Live");
            connection->waitForBytesWritten(50);
            if( byte_count!=4 )
            {
                qDebug() << "Client: live, byte_count:" << byte_count;
            }
        }
        else
        {
            qDebug() << "Remote: live, not connected, State:"
                     << connection->state();
        }
    }
    else
    {
        qDebug() << "Remote: live, tcpClient is closed";
    }
}

void FaConnection::readyRead()
{
    QByteArray data = connection->readAll();

    if( data.length()==4 )
    {
        watchdog->start(RE_WATCHDOG);
    }
    else if( data.contains("Live") )
    {
        qDebug() << "Server: Misterious Live" << data;
        watchdog->start(RE_WATCHDOG);
        data.replace("Live", "");
    }
    else
    {
        qDebug() << "Server: Single watchdog failure," << data;
    }

    emit clientReadyRead(data, id);
}
