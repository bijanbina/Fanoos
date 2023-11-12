#ifndef FA_APACHE_SE_H
#define FA_APACHE_SE_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QObject>
#include <QVector>
#include <stdio.h>
#include <stdlib.h>
#include <QTimer>
#include <QSignalMapper>
#include "backend.h"

#define FA_LIVE_PACKET "<Live>\n"

class FaApacheSe : public QObject
{
    Q_OBJECT

public:
    explicit FaApacheSe(QObject *parent = 0);
    ~FaApacheSe();

    void write(int id, QString data);
    void bind(int port);

    QVector<QTcpSocket *> cons;

signals:
    void disconnected(int id);
    void connected(int id);
    void dataReady(int id, QString data);

public slots:
    void readyRead(int id);
    void acceptConnection();
    void displayError(QAbstractSocket::SocketError socketError);
    void tcpDisconnected(int id);
    void watchdogTimeout(int id);
    void liveTimeout(int id);

private:
    QSignalMapper *mapper_data;
    QSignalMapper *mapper_disconnect;
    QSignalMapper *mapper_live;
    QSignalMapper *mapper_watchdog;
    QTcpServer *server;
    QVector<QTimer *> lives;
    QVector<QTimer *> watchdogs;
    int id;
};

#endif // FA_APACHE_SE_H
