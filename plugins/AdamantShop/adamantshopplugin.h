#ifndef ADAMANTSHOP_H
#define ADAMANTSHOP_H

#include "adamantplugin.h"
class PluginManager;
#include <core.h>
#include <pluginmanager.h>
#include <stashviewerplugin.h>
#include <stashviewer.h>
#include <shopviewer.h>
#include <templateviewer.h>
#include <shop/shop.h>

class AdamantShopPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "adamant.shop" FILE "adamantshop.json")
    Q_INTERFACES(AdamantPlugin)

public:
    AdamantShopPlugin()
        : _viewer(nullptr)
        , _templateViewer(nullptr)
        , _stashViewer(nullptr) {
    }
    static QDir shopsPath();

    Shop* loadShop(const QString& file);
    Q_INVOKABLE void loadShops();

    bool addShop(Shop* shop);

    bool saveShop(const Shop* shop) const;
    Q_INVOKABLE void saveShops() const;

    Q_INVOKABLE bool deleteShop(const Shop* shop);

    Q_INVOKABLE QStringList getShopNames() {
        return _shops.uniqueKeys();
    }

    Q_INVOKABLE ShopList getShops() {
        return _shops;
    }

    Q_INVOKABLE bool hasShop(const QString &shopName) {
        return _shops.contains(shopName);
    }

    Q_INVOKABLE Shop* getShop(const QString &shopName) {
        return _shops.value(shopName, nullptr);
    }

public slots:
    void OnLoad();
private:
    ShopViewer* _viewer;
    TemplateViewer* _templateViewer;
    ShopList _shops;

    StashViewer* _stashViewer;
public slots:
    void setupEngine(QScriptEngine* engine, QScriptValue* plugin);

    friend class ShopViewer;
};

#endif // ADAMANTSHOP_H
