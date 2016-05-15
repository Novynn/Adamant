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
    Q_PLUGIN_METADATA(IID "adamant.shop" FILE "adamantshop.json")
    Q_INTERFACES(AdamantPlugin)

public:
    AdamantShopPlugin()
        : _viewer(nullptr)
        , _stashViewer(nullptr) {
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

    StashViewer* _stashViewer;
public slots:
    void setupEngine(QScriptEngine* engine, QScriptValue* plugin);

    friend class ShopViewer;
};

#endif // ADAMANTSHOP_H