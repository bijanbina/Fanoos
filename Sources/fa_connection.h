#ifndef FA_CONNECTION_H
#define FA_CONNECTION_H

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

class FaConnection : public QObject
{
    Q_OBJECT

public:
    explicit FaConnection(QTcpSocket *con, QObject *parent = 0);
    ~FaConnection();

signals:
    void errorConnection();
    void clientDisconnected();
    void clientConnected();
    void clientReqSusspend();

public slots:
    void readyRead();
    void displayError(QAbstractSocket::SocketError socketError);
    void dataReady(QString data);
    void disconnect();
    void watchdogTimeout();
    void liveTimeout();

private:
    QTcpSocket *connection;
    QTimer *live;
    QTimer *watchdog;
};

#endif // FA_CONNECTION_H
