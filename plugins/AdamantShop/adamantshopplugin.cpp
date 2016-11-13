#include "adamantshopplugin.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <core.h>

QDir AdamantShopPlugin::shopsPath() {
    return CoreService::dataPath("shops");
}

void AdamantShopPlugin::loadShops(const QStringList &currentLeagues) {
    _shops.clear();

    QStringList leagues = currentLeagues;

    // NOTE(rory): Load leagues
    auto list = shopsPath().entryInfoList({"*.shop"});
    for (QFileInfo info : list) {
        if (leagues.contains(info.baseName())) continue;
        leagues.append(info.baseName());
    }

    // NOTE(rory):
    for (const QString& league : leagues) {
        if (_shops.contains(league)) continue;
        bool disabled = !currentLeagues.contains(league);

        auto shop = loadShop(shopsPath().absoluteFilePath(QString("%1.shop").arg(league)));

        if (!shop)  {
            shop = new Shop(league);
            shop->setUnused();
        }
        shop->setDisabled(disabled);

        _shops.insert(shop->league(), shop);
        _viewer->addShop(shop);
    }
}

bool AdamantShopPlugin::addShop(Shop* shop) {
    if (shop == nullptr) return false;
    _shops.insert(shop->league(), shop);
    return saveShop(shop);
}

Shop* AdamantShopPlugin::loadShop(const QString &file) {
    QFile shopFile(file);
    if (shopFile.exists() && shopFile.open(QFile::ReadOnly)) {
        Shop* shop = new Shop();

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(shopFile.readAll(), &error);
        if (error.error == QJsonParseError::NoError && shop->load(doc.object())) {
            qDebug() << "Loaded shop: " << shop->league() << file;
        }
        else {
            qDebug() << "Failed to load shop: " << file << error.errorString();
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

bool AdamantShopPlugin::clearShop(Shop* shop) {
    const QString league = shop->league();
    if (league.isEmpty()) return false;
    QString path = shopsPath().absoluteFilePath(QString("%1.shop").arg(league));
    QFile shopFile(path);
    if (shopFile.exists()) {
        if (shopFile.remove()) {
            shop->clear();
            shop->setUnused();
            return true;
        }
    }
    return false;
}

bool AdamantShopPlugin::saveShop(const Shop* shop) const {
    if (shop->league().isEmpty()) return false;
    QJsonObject object;
    if (shop->save(object)) {
        QJsonDocument doc(object);
        QString path = shopsPath().absoluteFilePath(QString("%1.shop").arg(shop->league()));
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

    Core()->settings()->beginGroup("data");
    const QStringList leagues = Core()->settings()->value("leagues").toStringList();
    Core()->settings()->endGroup();

    {
        _viewer = new ShopViewer(this, _stashViewer);
        Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/cart.png"),
                                                       "Shops", "Manage shop threads.",
                                                       _viewer);
        _viewer->setLeagues(leagues);
    }

    if (false) {
        _templateViewer = new TemplateViewer(this);
        Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/embed.png"),
                                                       "Shop Templates", "Manage shop templates.",
                                                       _templateViewer);
    }




    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();

    loadShops(leagues);

    // TODO(rory): Remove this
    {
        Item* item = nullptr;

        QFile file("test.item");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            item = new Item(doc.object());
        }

        for (Shop* shop : _shops.values()) {
            shop->generateShopContent([&item](const QString &id) {
                Q_UNUSED(id);
                return item;
            });
        }
    }
}

void AdamantShopPlugin::setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
    Q_UNUSED(engine)
    Q_UNUSED(plugin)
    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();
}
