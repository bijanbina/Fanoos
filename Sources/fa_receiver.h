#ifndef FA_RECEIVER_H
#define FA_RECEIVER_H

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
#include "fa_connection.h"

class FaReceiver : public QObject
{
    Q_OBJECT

public:
    explicit FaReceiver(int port, QObject *parent = 0);
    ~FaReceiver();

signals:
    void dataReady(QString data);

public slots:
    void acceptConnection();
    void readyRead(QString data, int id);
    void displayError(QAbstractSocket::SocketError socketError);
    void handleDisconnect(int id);
    void handleConnect(int id);

private:
    long bytes_received; //useless
    QTcpServer *server;
    QString message;

    QVector<FaConnection *> fa_cons;
};

#endif // FA_RECEIVER_H
