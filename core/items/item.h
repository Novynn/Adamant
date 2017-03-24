#ifndef ITEM_H
#define ITEM_H

#include <core_global.h>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QVariant>
#include <QJsonDocument>
#include <QRegularExpression>

struct ItemSocket {
    int group;
    QString attr;
};

class ItemLocation;

class CORE_EXTERN Item
{
public:
    Item(ItemLocation* parent, QJsonObject data);

    QVariant data(QString key) const {
        return _data.value(key).toVariant();
    }

    QString dump() const {
        return QJsonDocument(_data).toJson(QJsonDocument::Indented);
    }

    QJsonObject toJson() const {
        return _data;
    }

    const ItemLocation* getParent() const {
        return _parent;
    }

private:
    const ItemLocation* _parent;
    QJsonObject _data;
};

#endif // ITEM_H
