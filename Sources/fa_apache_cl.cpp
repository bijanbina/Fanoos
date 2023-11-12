#include "fa_apache_cl.h"

FaApacheCl::FaApacheCl(QObject *parent): QObject(parent)
{
    connection = new QTcpSocket;
    connection->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    connect(connection, SIGNAL(connected()),
            this, SLOT(connected()));
    connect(connection, SIGNAL(readyRead()),
            this, SLOT(readyRead()));
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(disconnected()),
            this, SLOT(disconnected()));

    live = new QTimer;
    watchdog = new QTimer;

    connect(live, SIGNAL(timeout()),
            this, SLOT(liveTimeout()));
    connect(watchdog, SIGNAL(timeout()),
            this, SLOT(watchdogTimeout()));
}

FaApacheCl::~FaApacheCl()
{
    if( connection->isOpen() )
    {
        connection->close();
    }
    delete connection;
}

void FaApacheCl::start(QString ip, int port)
{
    c_ip = ip;
    c_port = port;
    connection->connectToHost(QHostAddress(c_ip), c_port);
}

void FaApacheCl::connected()
{
    live->start(FA_LIVE);
    watchdog->start(FA_WATCHDOG);

    emit clientConnected();
}

void FaApacheCl::displayError(QAbstractSocket::SocketError socketError)
 {
     if( socketError==QTcpSocket::RemoteHostClosedError )
     {
         return;
     }

     qDebug() << QString("FaApacheCl::Error Happened");
}

void FaApacheCl::write(QString data)
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

void FaApacheCl::disconnect()
{
    connection->connectToHost(QHostAddress(c_ip), c_port);
}

// client lost, drop connection and reconnect
void FaApacheCl::watchdogTimeout()
{
    if( connection->isOpen() )
    {
        qDebug() << "Remote: connection dropped:"
                 << connection->state();
        watchdog->stop();
        live->stop();

        // in disconnect it will try to reconnect
        connection->close();
    }
    else
    {
        qDebug() << "Remote: watchdog, tcpClient is closed";
    }
}

// keep client alive
void FaApacheCl::liveTimeout()
{
    if( connection->isOpen() )
    {
        if( connection->state()==QAbstractSocket::ConnectedState )
        {
            connection->write(FA_LIVE_PACKET);
        }
        else
        {
            qDebug() << "FaApacheCl::liveTimeout: not connected, State:"
                     << connection->state();
        }
    }
    else
    {
        qDebug() << "FaApacheCl::liveTimeout: tcpClient is closed";
    }
}

void FaApacheCl::readyRead()
{
    QByteArray data = connection->readAll();
    watchdog->start(RE_WATCHDOG);

    if( data==FA_LIVE_PACKET )
    {
        return;
    }
    else if( data.contains(FA_LIVE_PACKET) )
    {
        data.replace(FA_LIVE_PACKET, "");
    }

    emit dataReady(data);
}
