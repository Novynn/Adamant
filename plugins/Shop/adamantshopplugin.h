#ifndef ADAMANTSHOP_H
#define ADAMANTSHOP_H

#include "adamantplugin.h"
class PluginManager;
#include <core.h>
#include <pluginmanager.h>
#include <stashviewerplugin.h>

class AdamantShopPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.adamantshop" FILE "adamantshop.json")
    Q_INTERFACES(AdamantPlugin)

public slots:
    void OnLoad() {
        AdamantPlugin* plugin = Core()->getPluginManager()->getPluginByIID("com.adamant.plugin.stashviewer");
        StashViewerPlugin* sPlugin = dynamic_cast<StashViewerPlugin*>(plugin);

        Core()->interface()->window()->registerPage(QIcon(":/icons/dark/cart.png"), "Shops", "Manage shop threads.",
                                    new QWidget());

        if (sPlugin) {

        }
        qDebug() << "Hello?" << plugin << sPlugin;
    }
};

#endif // ADAMANTSHOP_H
