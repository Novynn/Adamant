#ifndef ITEMLOCATION_H
#define ITEMLOCATION_H

#include <core_global.h>
#include <QList>
#include <QJsonObject>
#include "item.h"
#include <QDebug>
#include <QPointF>
#include <QSize>
typedef QList<const Item*> ItemList;

class CORE_EXTERN ItemLocation {
    Q_GADGET
public:
    enum LocationType {
        StashLocation,
        CharacterLocation
    };

    enum State {
        Unknown,
        Loading,
        Loaded,
        LoadedFromDisk,
        Removed
    };

    virtual ~ItemLocation() {
        qDeleteAll(_items);
        _items.clear();
    }

    virtual LocationType location() const = 0;
    virtual QString header() const = 0;
    virtual QString hash() const = 0;
    virtual QString forumCode(const Item *item) const = 0;
    virtual bool operator<(const ItemLocation &other) const = 0;
    virtual bool operator==(const ItemLocation &other) const = 0;

    virtual QPointF itemPos(const Item* item) const {
        return QPointF(item->data("x").toFloat(), item->data("y").toFloat());
    }

    virtual QSize itemSize(const Item* item) const {
        return QSize(item->data("w").toInt(), item->data("h").toInt());
    }

    virtual void setItems(ItemList items, const QJsonObject &layout = QJsonObject()) {
        Q_UNUSED(layout);
        _items.clear();
        _items.append(items);
    }

    const ItemList items() const {
        return _items;
    }

    void setState(State state) {
        _state = state;
    }

    State state() const {
        return _state;
    }

    virtual QJsonObject toJson() const {
        QJsonObject result;
        QJsonArray items;
        for (const Item* item : _items) {
            items.append(item->toJson());
        }
        result.insert("items", items);
        //qDebug() << header() << "Saved to disk";
        return result;
    }

    virtual bool fromJson(const QJsonObject &object) {
        if (state() != ItemLocation::Unknown) return false; // Only load if in unknown state
        //qDebug() << header() << "Loaded from disk";
        qDeleteAll(_items);
        _items.clear();
        QJsonArray items = object.value("items").toArray();
        for (QJsonValue value : items) {
            _items << new Item(value.toObject());
        }
        return true;
    }

protected:
    explicit ItemLocation() : _state(Unknown) {}
    State _state;
    ItemList _items;
};



#endif // ITEMLOCATION_H
