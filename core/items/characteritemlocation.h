#ifndef CHARACTERITEMLOCATION_H
#define CHARACTERITEMLOCATION_H

#include "itemlocation.h"
#include <QJsonObject>
#include <QString>

class CharacterItemLocation : public ItemLocation
{
public:
    CharacterItemLocation(const QJsonObject &data);

    LocationType location() const final;
    QString header() const final;
    QString hash() const final;
    QString forumCode(const Item* item) const final;
    bool operator<(const ItemLocation &other) const final;
    bool operator==(const ItemLocation &other) const final;
    void addItems(ItemList items) final;
private:
    QString _name;
    QString _league;
    QString _class;
    int _level;
};

#endif // CHARACTERITEMLOCATION_H
