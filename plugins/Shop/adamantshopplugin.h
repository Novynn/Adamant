#ifndef ADAMANTSHOP_H
#define ADAMANTSHOP_H

#include "adamantplugin.h"
class PluginManager;
#include <core.h>
#include <pluginmanager.h>
#include <stashviewerplugin.h>
#include <stashviewer.h>
#include <shopviewer.h>
#include <shop/shop.h>

class AdamantShopPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.adamantshop" FILE "adamantshop.json")
    Q_INTERFACES(AdamantPlugin)

public:
    AdamantShopPlugin()
        : _viewer(new ShopViewer(this)) {
    }
    static QDir shopsPath();

    Shop* loadShop(const QString& file);
    Q_INVOKABLE void loadShops();

    bool saveShop(const Shop* shop) const;
    Q_INVOKABLE void saveShops() const;

    Q_INVOKABLE ShopList getShops() {
        return _shops;
    }

public slots:
    void OnLoad();
private:
    ShopViewer* _viewer;
    ShopList _shops;

    // AdamantPlugin interface
public slots:
    void SetupEngine(QScriptEngine* engine, QScriptValue* plugin);
};

#endif // ADAMANTSHOP_H
