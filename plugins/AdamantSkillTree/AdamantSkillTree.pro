! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += ADAMANTSKILLTREE_LIBRARY

adamantshop.depends = core

QT += core gui widgets network printsupport
TARGET = adamant.skilltree

OTHER_FILES += adamantskilltree.json
HEADERS += adamantskilltreeplugin.h \
    skilltreeviewer.h \
    node.h \
    group.h

win32: LIBS += -L$$OUT_PWD/../../bin/ -ladamant
else:unix: LIBS += -L$$OUT_PWD/../../bin/ -ladamant

INCLUDEPATH += $$PWD/../../core
DEPENDPATH += $$PWD/../../core

SOURCES += \
    adamantskilltreeplugin.cpp \
    skilltreeviewer.cpp \
    node.cpp \
    group.cpp

FORMS += \
    skilltreeviewer.ui

RESOURCES += \
    res.qrc
