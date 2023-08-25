TEMPLATE = app

QT += qml quick dbus core websockets

CONFIG += console

win32:QT += gamepad

SOURCES += Sources/main.cpp \
           Sources/backend.cpp \
           Sources/fa_channel.cpp

HEADERS += Sources/backend.h \
           Sources/fa_channel.h \
           Sources/fa_config.h

RESOURCES += Qml/ui.qrc \
             Resources/images.qrc \
             Resources/fonts.qrc

OTHER_FILES += Qml/*.qml


# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH += Qml/

DISTFILES += \
    Rebound.exe.manifest

MOC_DIR = Build/.moc
RCC_DIR = Build/.rcc
OBJECTS_DIR = Build/.obj
UI_DIR = Build/.ui

HEADERS += \
    Sources/re_meta_oc.h

