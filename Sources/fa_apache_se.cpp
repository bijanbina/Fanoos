#include "fa_apache_se.h"

FaApacheSe::FaApacheSe(QObject *parent): QObject(parent)
{
    server = new QTcpServer;
    connect(server, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    mapper_data       = new QSignalMapper(this);
    mapper_disconnect = new QSignalMapper(this);
    mapper_error      = new QSignalMapper(this);
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
    cons.push_back(NULL);
    qDebug() << "FaApacheSe::acceptConnection" << new_con_id;

    QTcpSocket *con = server->nextPendingConnection();
    cons[new_con_id] = con;
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
            mapper_error, SLOT(map()));
    mapper_data->setMapping(con, new_con_id);
    connect(mapper_error, SIGNAL(mapped(int)),
             this, SLOT(displayError(int)));

    // disconnected
    connect(con, SIGNAL(disconnected()),
            mapper_disconnect, SLOT(map()));
    mapper_disconnect->setMapping(con, new_con_id);
    connect(mapper_disconnect, SIGNAL(mapped(int)),
            this, SLOT(tcpDisconnected(int)));

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
    read_bufs.push_back(QByteArray());

    live->start(FA_LIVE);
    watchdog->start(FA_WATCHDOG);
    emit connected(new_con_id);
}

void FaApacheSe::displayError(int id)
 {
    qDebug() << "FaApacheSe::Error" << cons[id]->errorString();

    cons[id]->close();
    lives[id]->stop();
    watchdogs[id]->stop();
    if( cons[id]->error()==QTcpSocket::RemoteHostClosedError )
    {
        return;
    }
}

void FaApacheSe::write(int id, QString data)
{
    if( id>=cons.size() )
    {
        qDebug() << "Error 301: FaApacheSe::write,"
                 << "connection not found:" << id;
        return;
    }
    if( cons[id]->isOpen()==0 ||
        cons[id]->state()!=QAbstractSocket::ConnectedState )
    {
        qDebug() << "Error 302: FaApacheSe::write,"
                 << "connection not open:" << id << data;
        return;
    }

    lives[id]->start(FA_LIVE);//don't send live
    QByteArray data_b(data.toStdString().c_str());
    data_b.prepend(FA_START_PACKET);
    data_b.append(FA_END_PACKET);
    cons[id]->write(data_b);
    cons[id]->waitForBytesWritten(50);
    lives[id]->start(FA_LIVE);//don't send live
}

void FaApacheSe::tcpDisconnected(int id)
{
    watchdogs[id]->stop();
    lives[id]->stop();
}

// client lost, drop connection and reconnect
void FaApacheSe::watchdogTimeout(int id)
{
    if( cons[id]->isOpen() )
    {
        qDebug() << "Remote: connection dropped:"
                 << cons[id]->state();
        watchdogs[id]->stop();
        lives[id]->stop();
        cons[id]->close();

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
            write(id, FA_LIVE_PACKET);
        }
        else
        {
            qDebug() << "FaApacheSe::liveTimeout: not connected, State:"
                     << cons[id]->state();
            lives[id]->stop();
        }
    }
    else
    {
        qDebug() << "ReApacheCl::liveTimeout: socket is closed";
        lives[id]->stop();
    }
}

void FaApacheSe::readyRead(int id)
{
    read_bufs[id] += cons[id]->readAll();
    QByteArray data = processBuffer(id);

    watchdogs[id]->start(FA_WATCHDOG);

    if( data==FA_LIVE_PACKET )
    {
        return;
    }
    else if( data.contains(FA_LIVE_PACKET) )
    {
        data.replace(FA_LIVE_PACKET, "");
    }

    if( data.isEmpty() )
    {
        return;
    }

    emit dataReady(id, data);
}

QByteArray FaApacheSe::processBuffer(int id)
{
    if( read_bufs[id].contains(FA_START_PACKET)==0 )
    {
        return "";
    }
    if( read_bufs[id].contains(FA_END_PACKET)==0 )
    {
        return "";
    }
    int start_index = read_bufs[id].indexOf(FA_START_PACKET);
    start_index += strlen(FA_START_PACKET);
    read_bufs[id].remove(0, start_index);

    int end_index = read_bufs[id].indexOf(FA_END_PACKET);
    QByteArray data = read_bufs[id].mid(0, end_index);

    end_index += strlen(FA_END_PACKET);
    read_bufs[id].remove(0, end_index);

    return data;
}
