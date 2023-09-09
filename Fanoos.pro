TEMPLATE = app

QT += core network

CONFIG += console

SOURCES += Sources/main.cpp \
           Sources/backend.cpp \
           Sources/fa_chapar.cpp \
           Sources/fa_connection.cpp \
           Sources/fa_receiver.cpp \
           Sources/fa_transmitter.cpp

HEADERS += Sources/backend.h \
           Sources/fa_chapar.h \
           Sources/fa_config.h \
           Sources/fa_connection.h \
           Sources/fa_receiver.h \
           Sources/fa_transmitter.h

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj
UI_DIR = Build/.ui
