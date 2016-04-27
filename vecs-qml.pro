TEMPLATE = app

QT += qml quick bluetooth
CONFIG += c++11

SOURCES += main.cpp \
    vecscontroller.cpp \
    vecsdevice.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    vecscontroller.h \
    vecsdevice.h

DISTFILES += \
    BorDelayButton.qml \
    BorCheckButton.qml \
    BorToolButton.qml \
    helper.js


