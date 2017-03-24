#ifndef STASHITEMLOCATION_H
#define STASHITEMLOCATION_H

#include "itemlocation.h"
#include <QMetaObject>
#include <QMetaEnum>
#include <QColor>
#include <QJsonObject>
#include <QString>

class CORE_EXTERN StashItemLocation : public ItemLocation
{
    Q_GADGET
public:
    StashItemLocation(const QString &league, const QJsonObject &tabData);

    QColor tabColor() const {
        return _color;
    }

    enum Type {
        Normal,
        Premium,
        Currency
    };
    Q_ENUM(Type)

    static QString TypeToName(Type type);
    static Type NameToType(const QString &typeName);
    static QMetaEnum GetTypeEnum();

    Type type() const;

    void update(const QJsonObject &tabData);

    LocationType location() const;
    QString header() const;
    QString hash() const;
    QString forumCode(const Item &item) const;
    bool operator<(const ItemLocation &other) const;
    bool operator==(const ItemLocation &other) const;

    void setItems(ItemList items, const QJsonObject &layout);
    QJsonObject toJson();
    bool fromJson(const QJsonObject& object);
    QPointF itemPos(const Item &item) const;
    QSize itemSize(const Item &item) const;
    int tabIndex() const;

    int getAutoUpdateInterval() const {
        return _updateInterval;
    }

    void setAutoUpdateInterval(int interval) {
        _updateInterval = interval;
    }

private:
    int _tabIndex;
    QString _tabId;
    QString _tabLabel;

    QColor _color;
    Type _type;
    QJsonObject _layout;

    int _updateInterval;
};

#endif // STASHITEMLOCATION_H
