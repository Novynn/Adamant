#ifndef ADAMANTSHOP_H
#define ADAMANTSHOP_H

#include "adamantplugin.h"
class PluginManager;
#include <core.h>
#include <pluginmanager.h>
#include <stashviewerplugin.h>

#include <shopviewer.h>

class AdamantShopPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.adamantshop" FILE "adamantshop.json")
    Q_INTERFACES(AdamantPlugin)

public:
    AdamantShopPlugin()
        : _viewer(new ShopViewer()) {

    }

public slots:
    void OnLoad() {
        AdamantPlugin* plugin = Core()->getPluginManager()->getPluginByIID("com.adamant.plugin.stashviewer");
        StashViewerPlugin* sPlugin = dynamic_cast<StashViewerPlugin*>(plugin);

        Core()->interface()->window()->registerPage(QIcon(":/icons/dark/cart.png"),
                                                    "Shops", "Manage shop threads.",
                                                    _viewer);

//        qDebug() << "Hello?" << plugin << sPlugin;
    }
private:
    ShopViewer* _viewer;
};

#endif // ADAMANTSHOP_H
