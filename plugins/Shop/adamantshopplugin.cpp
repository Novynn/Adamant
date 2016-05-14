#include "adamantshopplugin.h"
#include <core.h>

QDir AdamantShopPlugin::shopsPath() {
    return CoreService::dataPath("shops");
}

void AdamantShopPlugin::loadShops() {
    _shops.clear();
    auto list = shopsPath().entryInfoList({"*.shop"});
    for (QFileInfo info : list) {
        auto shop = loadShop(info.absoluteFilePath());
        if (shop == nullptr) continue;
        _shops << shop;
    }
    _viewer->setShops(_shops);
}

Shop* AdamantShopPlugin::loadShop(const QString &file) {
    QFile shopFile(file);
    if (shopFile.exists() && shopFile.open(QFile::ReadOnly)) {
        Shop* shop = new Shop();

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(shopFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError && shop->load(doc.object())) {
            qDebug() << "Loaded shop: " << shop->name() << file;
        }
        else {
            qDebug() << "Failed to load shop: " << file;
            shop->deleteLater();
            shop = nullptr;
        }
        shopFile.close();

        if (shop != nullptr) {
            saveShop(shop);
        }
        return shop;
    }

    return nullptr;
}

void AdamantShopPlugin::saveShops() const {
    for (const Shop* shop : _shops) {
        saveShop(shop);
    }

}

bool AdamantShopPlugin::saveShop(const Shop* shop) const {
    if (shop->name().isEmpty()) return false;
    QJsonObject object;
    if (shop->save(object)) {
        QJsonDocument doc(object);
        QString path = shopsPath().absoluteFilePath(QString("%1.shop").arg(shop->name()));
        QFile shopFile(path);
        if (shopFile.open(QFile::WriteOnly)) {
            shopFile.write(doc.toJson());
            shopFile.close();
            return true;
        }
    }


    return false;
}

void AdamantShopPlugin::OnLoad() {
    AdamantPlugin* plugin = Core()->getPluginManager()->getPluginByIID("com.adamant.plugin.stashviewer");
    StashViewerPlugin* sPlugin = dynamic_cast<StashViewerPlugin*>(plugin);
    if (!sPlugin) {
        // Error, requires StashViewerPlugins

        return;
    }
    //        StashViewer* viewer = sPlugin->property("stashViewer").value<StashViewer*>();

    Core()->getInterface()->window()->registerPage(QIcon(":/icons/dark/cart.png"),
                                                   "Shops", "Manage shop threads.",
                                                   _viewer);

    Core()->settings()->beginGroup("data");
    const QStringList leagues = Core()->settings()->value("leagues").toStringList();
    Core()->settings()->endGroup();
    _viewer->setLeagues(leagues);

    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();

    loadShops();
    if (_shops.empty()) {
        Shop* shop = new Shop("Testirino", "Standard");
        saveShop(shop);
    }
}

void AdamantShopPlugin::SetupEngine(QScriptEngine* engine, QScriptValue* plugin) {
    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();
    qScriptRegisterSequenceMetaType<ShopList>(engine);
}
