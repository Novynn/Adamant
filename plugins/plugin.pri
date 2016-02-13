TEMPLATE     = lib
CONFIG      += qt plugin debug_and_release

DESTDIR      = ../../bin/plugins
INCLUDEPATH += ../../core/interfaces/ \
               ../../core/
HEADERS     += ../../core/interfaces/adamantplugin.h

CONFIG      += c++14

QT          += script


