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

class FaReceiver : public QObject
{
    Q_OBJECT

public:
    explicit FaReceiver(int port, QObject *parent = 0);
    ~FaReceiver();

signals:
    void dataReady(QString data);
    void clientDisconnected();
    void clientConnected();
    void clientReqSusspend();

public slots:
    void acceptConnection();
    void readyRead(int id);
    void displayError(QAbstractSocket::SocketError socketError);

private:
    long bytesReceived;
    QTcpServer *server;
    QString message;
    QVector<QTcpSocket *> cons; //connections
    QSignalMapper* signalMapper;
};

#endif // FA_RECEIVER_H
