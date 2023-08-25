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

class FaTransmitter : public QObject
{
    Q_OBJECT

public:
    explicit FaTransmitter(int port, QObject *parent = 0);
    ~FaTransmitter();

signals:
    void errorConnection();
    void clientDisconnected();
    void clientConnected();
    void clientReqSusspend();

public slots:
    void acceptConnection();
    void readyRead(int id);
    void displayError(QAbstractSocket::SocketError socketError);
    void dataReady(QString data);

private:
    long bytesReceived;
    QTcpServer *server;
    QString message;
    QVector<QTcpSocket *> cons; //connections
    QSignalMapper* signalMapper;
};

#endif // FA_TRANSMITTER_H
