#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T17:41:09
#
#-------------------------------------------------

QT       += core network widgets

TARGET = Adamant
TEMPLATE = app

RC_FILE = resources.rc

DESTDIR = ../bin

unix|win32: LIBS += -L$$OUT_PWD/../bin -ladamant

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core


CONFIG += qt c++14

HEADERS  += adamant.h \
    loggingsystem.h \
    version.h

SOURCES += main.cpp\
    adamant.cpp \
    loggingsystem.cpp

VERSION = 0.0.0.1
