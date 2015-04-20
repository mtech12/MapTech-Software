#-------------------------------------------------
#
# Project created by QtCreator 2012-10-23T23:02:21
#
#-------------------------------------------------

QT       += core \
            gui \
            network \
            qt3support \
            sql

TARGET = MT500
TEMPLATE = app


SOURCES += main.cpp\
        mt500.cpp \
    databasemodule.cpp

HEADERS  += mt500.h \
    logger.h \
    databasemodule.h

FORMS    += mt500.ui

CONFIG   += serialport
