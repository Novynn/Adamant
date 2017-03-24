#include "characteritemlocation.h"

QMap<QString, QPointF> CharacterItemLocation::InventoryLayout;

CharacterItemLocation::CharacterItemLocation(const QJsonObject& data)
    : ItemLocation()
{
    QJsonObject characterData = data.value("character").toObject();
    QJsonArray itemData = data.value("items").toArray();
    _name = characterData.value("name").toString();
    _league = characterData.value("league").toString();
    _class = characterData.value("class").toString();
    _level = characterData.value("level").toInt();

    for (QJsonValue itemVal : itemData) {
        auto item = QSharedPointer<Item>::create(this, itemVal.toObject());
        _items.append(item);
    }

    if (InventoryLayout.isEmpty()) {
        InventoryLayout.insert("Weapon", QPointF(0,0));
        InventoryLayout.insert("Offhand", QPointF(2,0));
        InventoryLayout.insert("Weapon2", QPointF(8,0));
        InventoryLayout.insert("Offhand2", QPointF(10,0));
        InventoryLayout.insert("Helm", QPointF(5,0));
        InventoryLayout.insert("BodyArmour", QPointF(5,2));
        InventoryLayout.insert("Belt", QPointF(5,5));
        InventoryLayout.insert("Gloves", QPointF(3,4));
        InventoryLayout.insert("Boots", QPointF(7,4));
        InventoryLayout.insert("Ring", QPointF(4,3));
        InventoryLayout.insert("Ring2", QPointF(7,3));
        InventoryLayout.insert("Amulet", QPointF(7,2));
        InventoryLayout.insert("Flask", QPointF(3.5,6));
    }
}


ItemLocation::LocationType CharacterItemLocation::location() const {
    return CharacterLocation;
}

QString CharacterItemLocation::header() const {
    return _name;
}

QString CharacterItemLocation::forumCode(const Item &item) const {
    const QString format("[linkItem location=\"%1\" character=\"%2\" x=\"%3\" y=\"%4\"]");
    return format.arg(item.data("inventoryId").toString()).arg(_name).arg(item.data("x").toInt()).arg(item.data("y").toInt());
}

QString CharacterItemLocation::hash() const {
    QString format("character:%1");
    return format.arg(_name);
}

bool CharacterItemLocation::operator<(const ItemLocation &other) const {
    if (location() != other.location())
        return location() < other.location();
    return _name < dynamic_cast<const CharacterItemLocation&>(other)._name;
}

bool CharacterItemLocation::operator==(const ItemLocation &other) const {
    if (location() != other.location())
        return false;
    return _name == dynamic_cast<const CharacterItemLocation&>(other)._name;
}

QJsonObject CharacterItemLocation::toJson() const {
    return QJsonObject();
}

QPointF CharacterItemLocation::itemPos(const Item &item) const {
    QPointF pos = ItemLocation::itemPos(item);
    QString inventoryId = item.data("inventoryId").toString();

    if (InventoryLayout.contains(inventoryId))
        pos += InventoryLayout.value(inventoryId);
    else if (inventoryId == "MainInventory")
        pos += QPointF(0, 9);

    return pos;
}

QSize CharacterItemLocation::itemSize(const Item &item) const {
    QSize size = ItemLocation::itemSize(item);
    QString inventoryId = item.data("inventoryId").toString();

    if ((inventoryId.startsWith("Weapon") || inventoryId.startsWith("Offhand")) &&
        InventoryLayout.contains(inventoryId)) {
        size.setWidth(2);
        size.setHeight(4);
    }

    return size;
}
