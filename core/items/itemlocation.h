#ifndef ITEMLOCATION_H
#define ITEMLOCATION_H

#include <core_global.h>
#include <QList>
class Item;
typedef QList<const Item*> ItemList;

class CORE_EXTERN ItemLocation {
public:
    enum LocationType {
        StashLocation,
        CharacterLocation
    };

    virtual LocationType Location() const = 0;
    virtual QString Header() const = 0;
    virtual QString Hash() const = 0;
    virtual QString ForumCode(const QString &league, const Item *item) const = 0;
    virtual bool operator<(const ItemLocation &other) const = 0;
    virtual bool operator==(const ItemLocation &other) const = 0;

    virtual void AddItems(ItemList items) = 0;
    const ItemList Items() const {
        return _items;
    }

protected:
    explicit ItemLocation() {}

    ItemList _items;
};



#endif // ITEMLOCATION_H
