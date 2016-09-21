! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += STASHVIEWER_LIBRARY

QT += core widgets gui network
TARGET = adamant.stashviewer

OTHER_FILES += stashviewer.json
HEADERS += stashviewerplugin.h \
    stashviewer.h \
    graphicitem.h \
    stashscene.h \
    characterviewer.h \
    graphicitemfactory.h \
    ui/characterwidget.h \
    stash/stashviewdata.h \
    stashviewer_global.h \
    ui/stashlistwidgetitem.h \
    itemrenderer.h \
    ui/itemtooltip.h

FORMS += \
    stashviewer.ui \
    dialogs/leaguedialog.ui \
    characterviewer.ui \
    ui/characterwidget.ui \
    ui/itemtooltip.ui

SOURCES += \
    stashviewer.cpp \
    stashviewerplugin.cpp \
    graphicitem.cpp \
    stashscene.cpp \
    characterviewer.cpp \
    ui/characterwidget.cpp \
    ui/itemtooltip.cpp

RESOURCES += \
    res.qrc


win32: LIBS += -L$$OUT_PWD/../../bin/ -ladamant
else:unix: LIBS += -L$$OUT_PWD/../../bin/ -ladamant

INCLUDEPATH += $$PWD/../../core
DEPENDPATH += $$PWD/../../core
