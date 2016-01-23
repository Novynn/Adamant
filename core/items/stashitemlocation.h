#ifndef STASHITEMLOCATION_H
#define STASHITEMLOCATION_H

#include "itemlocation.h"
#include <QColor>
#include <QJsonObject>
#include <QString>

class StashItemLocation : public ItemLocation
{
public:
    StashItemLocation(const QJsonObject &tabData);

    QColor TabColor() const {
        return _color;
    }

    LocationType Location() const final;
    QString Header() const final;
    QString Hash() const final;
    QString ForumCode(const QString &league, const Item* item) const final;
    bool operator<(const ItemLocation &other) const final;
    bool operator==(const ItemLocation &other) const final;
    void AddItems(ItemList items) final;
private:
    int tabIndex;
    QString tabLabel;

    QString league;
    QColor _color;
};

#endif // STASHITEMLOCATION_H
