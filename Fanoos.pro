TEMPLATE = app

QT += core network

CONFIG += console

SOURCES += Sources/main.cpp \
           Sources/backend.cpp \
           Sources/fa_apache_cl.cpp \
           Sources/fa_apache_se.cpp \
           Sources/fa_chapar.cpp

HEADERS += Sources/backend.h \
           Sources/fa_apache_cl.h \
           Sources/fa_apache_se.h \
           Sources/fa_chapar.h \
           Sources/fa_config.h

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj
UI_DIR = Build/.ui
