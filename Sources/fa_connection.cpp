#include "fa_connection.h"

#define JOYSTICK_DELAY 100

FaConnection::FaConnection(QTcpSocket *con,
               QObject *parent): QObject(parent)
{
    con->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connection = con;

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

FaConnection::~FaConnection()
{
    ;
}

void FaConnection::displayError(QAbstractSocket::SocketError socketError)
 {
     if (socketError == QTcpSocket::RemoteHostClosedError)
         return;

     qDebug() <<  QString("Error Happened");
}

void FaConnection::write(QString data)
{
    QByteArray data_b(data.toStdString().c_str());
    connection->write(data_b);
}

void FaConnection::disconnected()
{
    watchdog->stop();
    live->stop();
    connection->close();
}

void FaConnection::watchdogTimeout()
{
    if( connection->isOpen() )
    {
        qDebug() << "Remote: connection dropped:"
                 << connection->state();
        disconnected();
    }
    else
    {
        qDebug() << "Remote: watchdog, tcpClient is closed";
    }
}

void FaConnection::liveTimeout()
{
    if( connection->isOpen() )
    {
        if( connection->state()==QAbstractSocket::ConnectedState )
        {
            connection->write("Live");
            connection->waitForBytesWritten(50);
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
    watchdog->start(RE_WATCHDOG);
}
