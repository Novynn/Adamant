! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += TCPDISCONNECTPLUGIN_LIBRARY
QT += core

TARGET = adamant.tcpdisconnect

OTHER_FILES += tcpdisconnect.json

LIBS += -lIPHLPAPI -lWs2_32 -lpsapi
QMAKE_LFLAGS += -shared

DISTFILES += \
    TCPDisconnect.exe.manifest \
    TCPDisconnect.rc

HEADERS += \
    tcpdisconnectplugin.h
