#include "fa_apache_se.h"

FaApacheSe::FaApacheSe(QString name, QObject *parent): QObject(parent){
    con_name = name; // for debug msg
    server = new QTcpServer;
    connect(server, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));

    mapper_data       = new QSignalMapper(this);
    mapper_error      = new QSignalMapper(this);
    mapper_live       = new QSignalMapper(this);
    mapper_watchdog   = new QSignalMapper(this);
    mapper_disconnect = new QSignalMapper(this);

    connect(mapper_data      , SIGNAL(mapped(int)),
            this             , SLOT(readyRead(int)));
    connect(mapper_error     , SIGNAL(mapped(int)),
            this             , SLOT(displayError(int)));
    connect(mapper_live      , SIGNAL(mapped(int)),
            this             , SLOT(liveTimeout(int)));
    connect(mapper_watchdog  , SIGNAL(mapped(int)),
            this             , SLOT(watchdogTimeout(int)));
    connect(mapper_disconnect, SIGNAL(mapped(int)),
            this             , SLOT(tcpDisconnected(int)));
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
    if( putInFree() )
    {
        return;
    }
    int new_con_id = cons.length();
    cons.push_back(NULL);

    QTcpSocket *con = server->nextPendingConnection();
    cons[new_con_id] = con;
    con->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    QTimer *live     = new QTimer;
    QTimer *watchdog = new QTimer;
    QString msg = "FaApacheSe::" + con_name;
    msg += " accept connection";
    ipv4.push_back(QHostAddress(con->peerAddress().toIPv4Address()));
    qDebug() << msg.toStdString().c_str() << new_con_id
             << ipv4[new_con_id].toString();

    // readyRead
    mapper_data->setMapping(con, new_con_id);
    connect(con, SIGNAL(readyRead()), mapper_data, SLOT(map()));

    // displayError
    mapper_error->setMapping(con, new_con_id);
    connect(con, SIGNAL(error(QAbstractSocket::SocketError)),
            mapper_error, SLOT(map()));

    // disconnected
    mapper_disconnect->setMapping(con, new_con_id);
    connect(con, SIGNAL(disconnected()),
            mapper_disconnect, SLOT(map()));

    // live
    mapper_live->setMapping(live, new_con_id);
    connect(live, SIGNAL(timeout()),
            mapper_live, SLOT(map()));

    // watchdog
    mapper_watchdog->setMapping(watchdog, new_con_id);
    connect(watchdog, SIGNAL(timeout()),
            mapper_watchdog, SLOT(map()));

    lives.push_back(live);
    watchdogs.push_back(watchdog);
    read_bufs.push_back(QByteArray());

    live->start(FA_LIVE);
    watchdog->start(FA_WATCHDOG);
    emit connected(new_con_id);
}

void FaApacheSe::displayError(int id)
 {
    QString msg = "FaApacheSe::" + con_name;
    msg += " Error";
    qDebug() << msg.toStdString().c_str()
             << id << cons[id]->errorString()
             << cons[id]->state()
             << ipv4[id].toString();

    lives[id]->stop();
    watchdogs[id]->stop();

    if( cons[id]->error()==QTcpSocket::RemoteHostClosedError )
    {
        return;
    }

    qDebug() << "FaApacheSe::displayError," << id;
}

void FaApacheSe::tcpDisconnected(int id)
{
    QString msg = "FaApacheSe::" + con_name;
    msg += " disconnected";
    qDebug() << msg.toStdString().c_str() << id
             << ipv4[id].toString();
    cons[id]->close();
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

// return id in array where connection is free
int FaApacheSe::putInFree()
{
    int len = cons.length();
    for( int i=0 ; i<len ; i++ )
    {
        if( cons[i]->isOpen()==0 )
        {
            mapper_data->removeMappings(cons[id]);
            mapper_error->removeMappings(cons[id]);
            mapper_disconnect->removeMappings(cons[id]);
            delete cons[id];
            QTcpSocket *con = server->nextPendingConnection();
            cons[i] = con;
            con->setSocketOption(
                        QAbstractSocket::LowDelayOption, 1);
            quint32 ip_32 = con->peerAddress().toIPv4Address();
            ipv4[i] = QHostAddress(ip_32);
            QString msg = "FaApacheSe::" + con_name;
            msg += " refereshing connection";
            qDebug() << msg.toStdString().c_str() << i
                     << ipv4[i].toString();

            mapper_data->setMapping(con, i);
            connect(con, SIGNAL(readyRead()), mapper_data, SLOT(map()));

            // displayError
            mapper_error->setMapping(con, i);
            connect(con, SIGNAL(error(QAbstractSocket::SocketError)),
                    mapper_error, SLOT(map()));

            // disconnected
            mapper_disconnect->setMapping(con, i);
            connect(con, SIGNAL(disconnected()),
                    mapper_disconnect, SLOT(map()));

            lives[i]->start(FA_LIVE);
            watchdogs[i]->start(FA_WATCHDOG);
            emit connected(i);

            return 1;
        }
        else
        {
            qDebug() << "conn is open" << i;
        }
    }

    return 0;
}
