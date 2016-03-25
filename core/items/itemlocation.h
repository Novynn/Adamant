#ifndef ITEMLOCATION_H
#define ITEMLOCATION_H

#include <core_global.h>
#include <QList>
#include <QJsonObject>
#include "item.h"
typedef QList<const Item*> ItemList;

class CORE_EXTERN ItemLocation {
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

    virtual void setItems(ItemList items) {
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
        return result;
    }

protected:
    explicit ItemLocation() : _state(Unknown) {}
    State _state;
    ItemList _items;
};



#endif // ITEMLOCATION_H
