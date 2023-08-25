#ifndef ReAdmin_H
#define ReAdmin_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QObject>
#include <QVector>
#include <stdio.h>
#include <stdlib.h>
#include <QTimer>
#include <QQmlProperty>
#include <QSignalMapper>
#include "backend.h"

class FaChannel : public QObject
{
    Q_OBJECT

public:
    explicit FaChannel(int port, QObject *parent = 0);
    ~FaChannel();

signals:
    void errorConnection();
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

#endif // ReAdmin_H
