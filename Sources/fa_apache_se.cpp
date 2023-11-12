#include "fa_apache_se.h"

FaApacheSe::FaApacheSe(QObject *parent): QObject(parent)
{
    server = new QTcpServer;
    connect(server, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    mapper_data       = new QSignalMapper(this);
    mapper_disconnect = new QSignalMapper(this);
    mapper_live       = new QSignalMapper(this);
    mapper_watchdog   = new QSignalMapper(this);
}

FaApacheSe::~FaApacheSe()
{
    int len = cons.size();
    for( int i=0 ; i<len ; i++ )
    {
        if( cons[id]==NULL )
        {
            continue;
        }
        if( cons[id]->isOpen() )
        {
            cons[id]->close();
        }
    }
}

void FaApacheSe::bind(int port)
{
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

void FaApacheSe::acceptConnection()
{
    int new_con_id = cons.length();
    qDebug() << "Accepted connection" << new_con_id;

    QTcpSocket *con = server->nextPendingConnection();
    con->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    QTimer *live     = new QTimer;
    QTimer *watchdog = new QTimer;

    // readyRead
    connect(con, SIGNAL(readyRead()), mapper_data, SLOT(map()));
    mapper_data->setMapping(con, new_con_id);
    connect(mapper_data, SIGNAL(mapped(int)),
             this, SLOT(readyRead(int)));

    // displayError
    connect(con, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    // disconnected
    connect(con, SIGNAL(disconnected()),
            mapper_disconnect, SLOT(map()));
    mapper_disconnect->setMapping(con, new_con_id);
    connect(mapper_disconnect, SIGNAL(mapped(int)),
            this, SLOT(disconnected(int)));

    // live
    connect(live, SIGNAL(timeout()),
            mapper_live, SLOT(map()));
    mapper_live->setMapping(live, new_con_id);
    connect(mapper_live, SIGNAL(mapped(int)),
            this, SLOT(liveTimeout(int)));

    // watchdog
    connect(watchdog, SIGNAL(timeout()),
            mapper_watchdog, SLOT(map()));
    mapper_watchdog->setMapping(watchdog, new_con_id);
    connect(mapper_watchdog, SIGNAL(mapped(int)),
            this, SLOT(watchdogTimeout(int)));

    lives.push_back(live);
    watchdogs.push_back(watchdog);
    cons.push_back(con);

    live->start(FA_LIVE);
    watchdog->start(FA_WATCHDOG);
    emit connected(new_con_id);
}

void FaApacheSe::displayError(QAbstractSocket::SocketError
                              socketError)
 {
     if( socketError==QTcpSocket::RemoteHostClosedError )
     {
         return;
     }

     qDebug() << QString("FaApacheSe::Error Happened");
}

void FaApacheSe::write(int id, QString data)
{
    if( id>=cons.size() )
    {
        qDebug() << "Error 301: FaApacheSe::write,"
                 << "connection not found:" << id;
        return;
    }
    if( cons[id]->isOpen()==0 )
    {
        qDebug() << "Error 302: FaApacheSe::write,"
                 << "connection not open:" << id << data;
        return;
    }

    lives[id]->start(FA_LIVE);//don't send live
    QByteArray data_b(data.toStdString().c_str());
    cons[id]->write(data_b);
    cons[id]->waitForBytesWritten(50);
    lives[id]->start(FA_LIVE);//don't send live
}

void FaApacheSe::tcpDisconnected(int id)
{
    watchdogs[id]->stop();
    lives[id]->stop();
    cons[id]->close();
    emit disconnected(id);
}

// client lost, drop connection and reconnect
void FaApacheSe::watchdogTimeout(int id)
{
    if( cons[id]->isOpen() )
    {
        qDebug() << "Remote: connection dropped:"
                 << cons[id]->state();
        disconnected(id);
    }
    else
    {
        qDebug() << "Remote: watchdog, tcpClient is closed";
    }
}

// keep client alive
void FaApacheSe::liveTimeout(int id)
{
    if( cons[id]->isOpen() )
    {
        if( cons[id]->state()==QAbstractSocket::ConnectedState )
        {
            cons[id]->write(FA_LIVE_PACKET);
        }
        else
        {
            qDebug() << "FaApacheSe::liveTimeout: not connected, State:"
                     << cons[id]->state();
        }
    }
    else
    {
        qDebug() << "FaApacheSe::liveTimeout: tcpClient is closed";
    }
}

void FaApacheSe::readyRead(int id)
{
    QByteArray data = cons[id]->readAll();

    watchdogs[id]->start(RE_WATCHDOG);

    if( data==FA_LIVE_PACKET )
    {
        return;
    }
    else if( data.contains(FA_LIVE_PACKET) )
    {
        data.replace(FA_LIVE_PACKET, "");
    }

    emit dataReady(id, data);
}
