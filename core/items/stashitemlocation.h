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

    void update(const QJsonObject &tabData);

    LocationType location() const;
    QString header() const;
    QString hash() const;
    QString forumCode(const Item* item) const;
    bool operator<(const ItemLocation &other) const;
    bool operator==(const ItemLocation &other) const;

    QJsonObject toJson();
private:
    int tabIndex;
    QString _tabId;
    QString tabLabel;

    QString league;
    QColor _color;
};

#endif // STASHITEMLOCATION_H
