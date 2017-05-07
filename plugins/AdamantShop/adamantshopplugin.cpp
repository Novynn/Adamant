#include "adamantshopplugin.h"
#include <session/session.h>
#include <session/forum/forumrequest.h>
#include <QHBoxLayout>
#include <QPushButton>
#include <core.h>
#include "items/itemmanager.h"

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

QMap<QString, QString> AdamantShopPlugin::previewShop(const Shop* shop) {
    auto manager = Core()->getItemManager();
    auto resolver = [manager, shop](const QString &id) -> QSharedPointer<const Item> {
        // TODO(rory): Optimize this, using index on public item id -> Item*
        //             Also need a better method of storing item data
        auto stashes = manager->getStashTabs(shop->league());
        for (const StashItemLocation* stash : stashes) {
            for (QSharedPointer<const Item> item : stash->items()) {
                if (item->data("id").toString() == id) {
                    return item;
                }
            }
        }

        return QSharedPointer<const Item>();
    };

    QVariantHash data;
    data["item"] = QVariant::fromValue<Mustache::QtVariantContext::fn_t>([&resolver](const QString& val, Mustache::Renderer* renderer, Mustache::Context* context) -> QString {
        Q_UNUSED(renderer);
        Q_UNUSED(context);
        QSharedPointer<const Item> item = resolver(val);
        if (item.isNull() || !item->getParent())
            return "[[NOITEM]]";
        return item->getParent()->forumCode(*item);
    });

    data["allitems"] = QVariant::fromValue<Mustache::QtVariantContext::fn_t>([manager, shop](const QString& val, Mustache::Renderer* renderer, Mustache::Context* context) -> QString {
        Q_UNUSED(val);
        Q_UNUSED(renderer);
        Q_UNUSED(context);
        auto itemIds = shop->getItemIds();
        auto stashes = manager->getStashTabs(shop->league());

        QHash<QString, QSharedPointer<const Item>> items;

        // NOTE(rory): Resolve item ids to current items
        for (const StashItemLocation* stash : stashes) {
            for (QSharedPointer<const Item> item : stash->items()) {
                const QString id = item->data("id").toString();
                if (itemIds.contains(id)) {
                    items.insert(id, item);
                }
            }
        }

        QString result;

        for (const QString &id : itemIds) {
            QSharedPointer<const Item> item = items.value(id);
            if (!item) {
                result += "???";
                continue;
            }

            result += item->getParent()->forumCode(*item);
            result += shop->getItemData(id).getData();
        }

        return result;
    });

    return shop->generateShopContent("Welcome to my shop!\n", "{{#allitems}}{{/allitems}}", "\nThanks for visiting!", data);
}

void AdamantShopPlugin::updateShop(const Shop* shop) {
    Q_UNUSED(shop);

    //        ForumSubmission* s = new ForumSubmission();
    //        s->threadId = threadId;
    //        s->data.insert("content", content);
    //        _submissions.append(s);
    //        Core()->forum()->beginRequest(s);
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

    QStringList leagues;
    for (const QString &league : Core()->session()->leagues()) {
        // TODO(rory): Use flags returned with leagues rather than this check
        if (league.contains("SSF")) continue;
        leagues << league;
    }

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



    connect(Core()->forum(), &Session::ForumRequest::requestReady, this, [this](const ForumSubmission* submission) {
        if (!_submissions.contains(submission)) return;
    });

    connect(Core()->forum(), &Session::ForumRequest::requestError, this, [this](const ForumSubmission* submission, const QString &error) {
        if (!_submissions.contains(submission)) return;
        qDebug() << "Error!" << submission->threadId << error;
        _submissions.removeOne(submission);
        delete submission;
    });

    connect(Core()->forum(), &Session::ForumRequest::requestFinished, this, [this](const ForumSubmission* submission) {
        if (!_submissions.contains(submission)) return;
        qDebug() << "Finished!" << submission->threadId;
        _submissions.removeOne(submission);
        delete submission;
    });

    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();

    loadShops(leagues);
}

void AdamantShopPlugin::setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
    Q_UNUSED(engine)
    Q_UNUSED(plugin)
    qRegisterMetaType<Shop*>();
    qRegisterMetaType<ShopList>();
}
