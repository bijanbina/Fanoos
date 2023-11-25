#ifndef BACKEND_H
#define BACKEND_H

#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include "fa_config.h"

#ifdef _WIN32
    #include <windows.h>
#endif

struct screen_pos
{
    int x;
    int y;
};

int getIntCommand(const char *command);
QString getStrCommand(QString command);

screen_pos getPrimaryScreen();
void changeLaguageBack();

void updateScreenInfo(QObject *item);

int isItemVisible(QObject *item);

#endif // BACKEND_H
