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

    QColor tabColor() const {
        return _color;
    }

    LocationType location() const final;
    QString header() const final;
    QString hash() const final;
    QString forumCode(const QString &league, const Item* item) const final;
    bool operator<(const ItemLocation &other) const final;
    bool operator==(const ItemLocation &other) const final;
    void addItems(ItemList items) final;
private:
    int tabIndex;
    QString tabLabel;

    QString league;
    QColor _color;
};

#endif // STASHITEMLOCATION_H
