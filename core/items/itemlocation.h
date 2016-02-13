#ifndef ITEMLOCATION_H
#define ITEMLOCATION_H

#include <core_global.h>
#include <QList>
#include "item.h"
typedef QList<const Item*> ItemList;

class CORE_EXTERN ItemLocation {
public:
    enum LocationType {
        StashLocation,
        CharacterLocation
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

    virtual void addItems(ItemList items) = 0;
    const ItemList items() const {
        return _items;
    }

protected:
    explicit ItemLocation() {}

    ItemList _items;
};



#endif // ITEMLOCATION_H
