#include "shop.h"

Shop::Shop(QObject *parent)
    : Shop(QString(), parent) {
}


Shop::Shop(const QString &league, QObject *parent)
    : QObject(parent)
    , _league(league)
    , _created(QDateTime::currentDateTime())
    , _disabled(false)
    , _unused(false) {
}

bool Shop::save(QJsonObject& object) const {
    object["league"] = _league;
    object["template"] = _template;
    object["created"] = dateToInt(_created);

    QJsonArray threads;
    for (const ShopThread &thread : _threads.values()) {
        threads << QJsonObject({
                                   QPair<QString, QString>("id", thread._id),
                                   QPair<QString, qint64>("updated", dateToInt(thread._updated)),
                                   QPair<QString, qint64>("bumped", dateToInt(thread._bumped))
                               });
    }
    object["threads"] = threads;

    QJsonObject items;
    for (const QString &id : _items.uniqueKeys()) {
        auto item = _items.value(id);
        QJsonObject itemObject;
        itemObject["data"] = item._data;
        itemObject["inherit"] = item._inherit;
        itemObject["updated"] = dateToInt(item._updated);;
        items[id] = itemObject;
    }
    object["items"] = items;

    QJsonObject tabs;
    for (const QString &id : _tabs.uniqueKeys()) {
        auto tab = _tabs.value(id);
        QJsonObject tabObject;
        tabObject["data"] = tab._data;
        tabObject["updated"] = dateToInt(tab._updated);
        tabs[id] = tabObject;
    }
    object["tabs"] = tabs;
    return true;
}

bool Shop::load(const QJsonObject& object) {
    _league = object.value("league").toString();
    _template = object.value("template").toString();
    _created = intToDate(object.value("created").toVariant().toLongLong());

    QJsonArray threads = object.value("threads").toArray();
    for (const QJsonValue threadValue : threads) {
        auto threadObject = threadValue.toObject();
        ShopThread thread;
        thread._id      = threadObject.value("id").toString();
        thread._updated = intToDate(threadObject.value("updated").toVariant().toLongLong());
        thread._bumped  = intToDate(threadObject.value("bumped").toVariant().toLongLong());
        _threads.insert(thread._id, thread);
    }

    QJsonObject items = object.value("items").toObject();
    for (const QString &id : items.keys()) {
        auto itemObject = items.value(id).toObject();
        ShopItem item;
        item._id = itemObject.value("id").toString();
        item._data = itemObject.value("data").toString();
        item._updated = intToDate(itemObject.value("updated").toVariant().toLongLong());
        item._inherit = itemObject.value("inherit").toBool();
        _items.insert(id, item);
    }

    QJsonObject tabs = object.value("tabs").toObject();
    for (const QString &id : tabs.keys()) {
        auto tabObject = tabs.value(id).toObject();
        ShopTab tab;
        tab._id = tabObject.value("id").toString();
        tab._data = tabObject.value("data").toString();
        tab._updated = intToDate(tabObject.value("updated").toVariant().toLongLong());
        _tabs.insert(id, tab);
    }
    return true;
}
