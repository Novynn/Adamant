#ifndef ITEMLOCATION_H
#define ITEMLOCATION_H

#include <core_global.h>
#include <QList>
#include <QJsonObject>
#include "item.h"
#include <QDebug>
#include <QPointF>
#include <QSize>
#include <QSharedPointer>

typedef QList<QSharedPointer<const Item>> ItemList;

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
        _items.clear();
    }

    virtual LocationType location() const = 0;
    virtual QString header() const = 0;
    virtual QString hash() const = 0;
    virtual QString forumCode(const Item &item) const = 0;
    virtual bool operator<(const ItemLocation &other) const = 0;
    virtual bool operator==(const ItemLocation &other) const = 0;

    virtual QPointF itemPos(const Item &item) const {
        return QPointF(item.data("x").toFloat(), item.data("y").toFloat());
    }

    virtual QSizeF itemSize(const Item &item) const {
        return QSizeF(item.data("w").toInt(), item.data("h").toInt());
    }

    virtual void setItems(ItemList items, const QJsonValue &layout = QJsonValue()) {
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
        for (QSharedPointer<const Item> item : _items) {
            items.append(item->toJson());
        }
        result.insert("items", items);
        //qDebug() << header() << "Saved to disk";
        return result;
    }

    virtual bool fromJson(const QJsonObject &object) {
        if (state() != ItemLocation::Unknown) return false; // Only load if in unknown state
        //qDebug() << header() << "Loaded from disk";
        _items.clear();
        QJsonArray items = object.value("items").toArray();
        for (QJsonValue value : items) {
            _items << QSharedPointer<Item>::create(this, value.toObject());
        }
        return true;
    }

protected:
    explicit ItemLocation() : _state(Unknown) {}
    State _state;
    ItemList _items;
    QString _league;
};



#endif // ITEMLOCATION_H
