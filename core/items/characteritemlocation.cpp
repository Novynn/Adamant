#include "characteritemlocation.h"

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
        auto item = new Item(itemVal.toObject());
        _items.append(item);
    }
}


ItemLocation::LocationType CharacterItemLocation::location() const {
    return CharacterLocation;
}

QString CharacterItemLocation::header() const {
    return _name;
}

QString CharacterItemLocation::forumCode(const Item *item) const {
    Q_UNUSED(item)
    return "";
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
