! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += ADAMANTSHOP_LIBRARY

adamantshop.depends = stashviewer core

QT += core gui widgets network
TARGET = com.adamant.plugin.adamantshop

OTHER_FILES += adamantshop.json
HEADERS += adamantshopplugin.h

win32: LIBS += -L$$OUT_PWD/../../bin/ -ladamant
else:unix: LIBS += -L$$OUT_PWD/../../bin/ -ladamant

INCLUDEPATH += $$PWD/../../core
DEPENDPATH += $$PWD/../../core

win32: LIBS += -L$$OUT_PWD/../../bin/plugins/ -lcom.adamant.plugin.stashviewer
else:unix: LIBS += -L$$OUT_PWD/../../bin/plugins/ -lcom.adamant.plugin.stashviewer

INCLUDEPATH += $$PWD/../../plugins/StashViewer
DEPENDPATH += $$PWD/../../plugins/StashViewer
