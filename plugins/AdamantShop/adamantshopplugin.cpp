#include "adamantshopplugin.h"
#include <QHBoxLayout>
#include <QPushButton>
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
        if (_shops.contains(shop->name())) {
            qWarning() << "Duplicate shop name detected: " << shop->name();
            delete shop;
            continue;
        }
        _shops.insert(shop->name(), shop);
        _viewer->addShop(shop);
    }
}

bool AdamantShopPlugin::addShop(Shop* shop) {
    if (shop == nullptr) return false;
    _shops.insert(shop->name(), shop);
    return saveShop(shop);
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

bool AdamantShopPlugin::deleteShop(const Shop* shop) {
    if (shop->name().isEmpty()) return false;
    QString path = shopsPath().absoluteFilePath(QString("%1.shop").arg(shop->name()));
    QFile shopFile(path);
    if (shopFile.exists()) {
        if (shopFile.remove()) {
            _shops.remove(shop->name());
            delete shop;
            return true;
        }
    }
    return false;
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
    StashViewerPlugin* plugin = dynamic_cast<StashViewerPlugin*>(Core()->getPluginManager()->getPluginByIID("adamant.stashviewer"));
    if (plugin == nullptr) {
        // Error, requires StashViewerPlugin
        return;
    }
    _stashViewer = plugin->getStashViewer();
    if (_stashViewer == nullptr) {
        // Couldn't fetch StashViewer
        return;
    }
    _viewer = new ShopViewer(this, _stashViewer);
    Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/cart.png"),
                                                   "Shops", "Manage shop threads.",
                                                   _viewer);
    Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/embed.png"),
                                                   "Shop Templates", "Manage shop templates.",
                                                   new QWidget());
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
        _viewer->addShop(shop);
    }
}

void AdamantShopPlugin::setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
    Q_UNUSED(engine)
    Q_UNUSED(plugin)
    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();
}
