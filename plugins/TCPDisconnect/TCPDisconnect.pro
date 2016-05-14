! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += TCPDISCONNECTPLUGIN_LIBRARY
QT += core network
QT -= gui

TARGET = com.adamant.plugin.tcpdisconnect

OTHER_FILES += tcpdisconnect.json

LIBS += -lIPHLPAPI -lWs2_32 -lpsapi
win32:QMAKE_LFLAGS += -shared

#RC_FILE = tcpdisconnect.rc

DISTFILES += \
    TCPDisconnect.exe.manifest \
    TCPDisconnect.rc

HEADERS += \
    tcpdisconnectplugin.h
