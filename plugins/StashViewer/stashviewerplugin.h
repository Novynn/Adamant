#ifndef STASHVIEWERPLUGIN_H
#define STASHVIEWERPLUGIN_H

#include "adamantplugin.h"
#include <QDebug>
#include <QWidget>
#include "stashviewer.h"

#include <QDebug>
#include <session/psession.h>

#include <core.h>
#include <ui/ui.h>
#include <ui/mainwindow.h>

class StashViewerPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.stashviewer" FILE "stashviewer.json")
    Q_INTERFACES(AdamantPlugin)

public:
    Q_PROPERTY(StashViewer* Viewer MEMBER _viewer)

    StashViewerPlugin();
protected:
    void OnLoad();

private:
    StashViewer* _viewer;
};

#endif // STASHVIEWERPLUGIN_H
