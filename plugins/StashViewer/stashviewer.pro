! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += STASHVIEWER_LIBRARY

QT += core widgets gui network
TARGET = com.adamant.plugin.stashviewer

OTHER_FILES += stashviewer.json
HEADERS += stashviewerplugin.h \
    stashviewer.h \
    graphicitem.h \
    stashscene.h \
    dialogs/leaguedialog.h \
    characterviewer.h \
    dialogs/itemtooltip.h

FORMS += \
    stashviewer.ui \
    dialogs/leaguedialog.ui \
    characterviewer.ui \
    dialogs/itemtooltip.ui

SOURCES += \
    stashviewer.cpp \
    stashviewerplugin.cpp \
    graphicitem.cpp \
    stashscene.cpp \
    dialogs/leaguedialog.cpp \
    characterviewer.cpp \
    dialogs/itemtooltip.cpp

RESOURCES += \
    res.qrc


win32: LIBS += -L$$OUT_PWD/../../bin/ -ladamant
else:unix: LIBS += -L$$OUT_PWD/../../bin/ -ladamant

INCLUDEPATH += $$PWD/../../core
DEPENDPATH += $$PWD/../../core
