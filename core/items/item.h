#ifndef ITEM_H
#define ITEM_H

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QVariant>
#include <QJsonDocument>

struct ItemSocket {
    int group;
    QString attr;
};

class Item
{
public:
    Item(QJsonObject data);

    QVariant Data(QString key) const {
        return _data.value(key).toVariant();
    }

    QString Dump() const {
        return QJsonDocument(_data).toJson(QJsonDocument::Indented);
    }

private:
    QJsonObject _data;
};

#endif // ITEM_H
