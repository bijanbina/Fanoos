#ifndef FA_TRANSMITTER_H
#define FA_TRANSMITTER_H

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

class FaTransmitter : public QObject
{
    Q_OBJECT

public:
    explicit FaTransmitter(int port, QObject *parent = 0);
    ~FaTransmitter();

signals:

public slots:
    void acceptConnection();
    void readyRead(QString data, int id);
    void displayError(QAbstractSocket::SocketError socketError);
    void dataReady(QString data);
    void handleConnect(int id);
    void handleDisconnect(int id);

private:
    long bytesReceived;
    QTcpServer *server;
    QString message;
    QVector<FaConnection *> fa_cons;
};

#endif // FA_TRANSMITTER_H
