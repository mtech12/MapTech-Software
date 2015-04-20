#-------------------------------------------------
#
# Project created by QtCreator 2012-10-23T23:02:21
#
#-------------------------------------------------

QT       += core gui network qt3support

TARGET = MT500
TEMPLATE = app


SOURCES += main.cpp\
        mt500.cpp

HEADERS  += mt500.h \
    logger.h

FORMS    += mt500.ui

CONFIG   += serialport
