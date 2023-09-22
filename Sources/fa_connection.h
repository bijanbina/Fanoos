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
    explicit FaConnection(QTcpSocket *con, int con_id,
                          QObject *parent = 0);
    ~FaConnection();

    void write(QString data);

signals:
    void clientDisconnected(int id);
    void clientConnected(int id);
    void clientReadyRead(QString data, int id);

public slots:
    void readyRead();
    void displayError(QAbstractSocket::SocketError socketError);
    void handleDisconnect();
    void watchdogTimeout();
    void liveTimeout();

private:
    QTcpSocket *connection;
    QTimer *live;
    QTimer *watchdog;
    int id;
};

#endif // FA_CONNECTION_H
