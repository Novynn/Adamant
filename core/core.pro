DESTDIR       = ../bin

TEMPLATE = lib
CONFIG += qt c++14
QT += core network gui widgets script scripttools
TARGET = adamant

DEFINES += CORE_INTERNAL

HEADERS += \
    core.h \
    interfaces/adamantplugin.h \
    ui/commandbutton.h \
    ui/mainwindow.h \
    ui/ui.h \
    session/session.h \
    scripting/scriptsandbox.h \
    adamantplugininfo.h \
    pluginmanager.h \
    core_global.h \
    session/imagecache.h \
    items/itemmanager.h \
    items/itemlocation.h \
    items/stashitemlocation.h \
    items/item.h \
    ui/pages/pluginpage.h \
    ui/widgets/pluginitem.h \
    ui/widgets/pluginitemdelegate.h \
    session/sessionrequest.h \
    items/characteritemlocation.h \
    ui/widgets/custompage.h \
    session/forum/forumrequest.h \
    oauthhandler.h

SOURCES += \
    core.cpp \
    ui/mainwindow.cpp \
    ui/ui.cpp \
    session/session.cpp \
    scripting/scriptsandbox.cpp \
    pluginmanager.cpp \
    session/imagecache.cpp \
    items/itemmanager.cpp \
    items/stashitemlocation.cpp \
    items/item.cpp \
    ui/pages/pluginpage.cpp \
    ui/widgets/pluginitem.cpp \
    ui/widgets/pluginitemdelegate.cpp \
    session/sessionrequest.cpp \
    items/characteritemlocation.cpp \
    ui/widgets/custompage.cpp \
    session/forum/forumrequest.cpp \
    oauthhandler.cpp

RESOURCES += \
    ui/res/res.qrc

FORMS += \
    ui/mainwindow.ui \
    ui/pages/pluginpage.ui \
    ui/widgets/pluginitem.ui
