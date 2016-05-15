! include( ../plugin.pri ) {
    error( Couldn\'t find plugin.pri! )
}

DEFINES += GLOBALHOTKEYPLUGIN_LIBRARY \
           QXT_STATIC

QT += core
TARGET = adamant.globalhotkey

OTHER_FILES += globalhotkey.json
HEADERS += globalhotkeyplugin.h \
    qxt/qxtglobal.h \
    qxt/qxtglobalshortcut.h \
    qxt/qxtglobalshortcut_p.h

SOURCES += \
    qxt/qxtglobal.cpp \
    qxt/qxtglobalshortcut.cpp


unix:!macx:!android {
    SOURCES += qxt/qxtglobalshortcut_x11.cpp
}
win32 {
    SOURCES += qxt/qxtglobalshortcut_win.cpp
}
macx {
    SOURCES += qxt/qxtglobalshortcut_mac.cpp
}
