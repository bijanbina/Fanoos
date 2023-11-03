#ifndef FA_CHAPAR_H
#define FA_CHAPAR_H

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
#include "fa_transmitter.h"
#include "fa_apache_se.h"

class FaChapar : public QObject
{
    Q_OBJECT

public:
    explicit FaChapar(QObject *parent = 0);
    ~FaChapar();

public slots:
    void dataReady(int id, QString data);

private:
    FaApacheSe *receiver;
    FaApacheSe *transmitter;
};

#endif // FA_CHAPAR_H
