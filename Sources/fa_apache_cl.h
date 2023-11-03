#ifndef FA_APACHE_CL_H
#define FA_APACHE_CL_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QObject>
#include <QVector>
#include <stdio.h>
#include <stdlib.h>
#include <QTimer>
#include "backend.h"

class FaApacheCl : public QObject
{
    Q_OBJECT

public:
    explicit FaApacheCl(QObject *parent = 0);
    ~FaApacheCl();

    void write(QString data);
    void start(QString ip, int port);

signals:
    void disconnected();
    void clientConnected();
    void dataReady(QString data);

public slots:
    void connected();
    void readyRead();
    void displayError(QAbstractSocket::SocketError socketError);
    void disconnect();
    void watchdogTimeout();
    void liveTimeout();

private:
    QTcpSocket *connection;
    QTimer *live;
    QTimer *watchdog;

    QString c_ip;
    int c_port;
};

#endif // FA_APACHE_CL_H
