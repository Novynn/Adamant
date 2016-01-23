TEMPLATE      = lib
CONFIG       += qt plugin debug_and_release #link_prl
# CONFIG       -= embed_manifest_dll

INCLUDEPATH += ../../core/interfaces/ \
               ../../core/

CONFIG += c++14

DESTDIR       = ../../bin/plugins

HEADERS += ../../core/interfaces/adamantplugin.h

QT += script scripttools
