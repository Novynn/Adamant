#include "stashitemlocation.h"

StashItemLocation::StashItemLocation(const QJsonObject &tabData)
    : ItemLocation()
    , _type(Normal) {
    _tabId = tabData.value("id").toString();
    update(tabData);
}

QString StashItemLocation::TypeToName(StashItemLocation::Type type) {
    return GetTypeEnum().valueToKey(type);
}

StashItemLocation::Type StashItemLocation::NameToType(const QString& typeName) {
    bool ok;
    Type type = (Type)GetTypeEnum().keyToValue(typeName.toLatin1().constData(), &ok);
    return ok ? type : Normal;
}

QMetaEnum StashItemLocation::GetTypeEnum() {
    return QMetaEnum::fromType<StashItemLocation::Type>();
}

StashItemLocation::Type StashItemLocation::type() const {
    return _type;
}

void StashItemLocation::update(const QJsonObject& tabData)
{
    _tabIndex = tabData.value("i").toInt();
    _tabLabel = tabData.value("n").toString();
    QJsonObject color = tabData.value("colour").toObject();
    _color = QColor::fromRgb(color.value("r").toInt(),
                             color.value("g").toInt(),
                             color.value("b").toInt());
}

ItemLocation::LocationType StashItemLocation::location() const {
    return StashLocation;
}

QString StashItemLocation::header() const {
    return QString("%1").arg(_tabLabel);
}

QString StashItemLocation::forumCode(const Item *item) const {
    QString format("[linkItem location=\"Stash%1\" league=\"%2\" x=\"%3\" y=\"%4\"]");
    return format.arg(_tabIndex + 1).arg(_league).arg(item->data("x").toInt()).arg(item->data("y").toInt());
}

QString StashItemLocation::hash() const {
    return _tabId;
}

bool StashItemLocation::operator<(const ItemLocation &other) const {
    if (location() != other.location())
        return location() < other.location();
    return _tabIndex < dynamic_cast<const StashItemLocation&>(other)._tabIndex;
}

bool StashItemLocation::operator==(const ItemLocation &other) const {
    if (location() != other.location())
        return false;
    return _tabId == dynamic_cast<const StashItemLocation&>(other)._tabId;
}

void StashItemLocation::setItems(ItemList items, const QJsonObject &layout) {
    if (layout.isEmpty()) {
        _type = Normal;
    }
    else {
        _type = Currency;
    }
    _layout = layout;
    ItemLocation::setItems(items, layout);
}

QPointF StashItemLocation::itemPos(const Item* item) const {
    QPointF pos = ItemLocation::itemPos(item);
    switch (_type) {
        case Currency: {
            if (!_layout.isEmpty()) {
                QString key = QString::number((int)pos.x());
                QJsonObject object = _layout.value(key).toObject();
                // Why are these absolute values??? :(
                pos.setX(object.value("x").toDouble() / 47.4645);
                pos.setY(object.value("y").toDouble() / 47.4645);
            }
        } break;
        default: {
        } break;
    }
    return pos;
}

QSize StashItemLocation::itemSize(const Item* item) const {
    QPointF pos = ItemLocation::itemPos(item);
    QSize size = ItemLocation::itemSize(item);
    switch (_type) {
        case Currency: {
            if (!_layout.isEmpty()) {
                QString key = QString::number((int)pos.x());
                QJsonObject object = _layout.value(key).toObject();
                // Why are these absolute values??? :(
                size.setWidth(object.value("w").toInt());
                size.setHeight(object.value("h").toInt());
            }
        } break;
        default: {
        } break;
    }
    return size;
}

int StashItemLocation::tabIndex() const {
    return _tabIndex;
}

QJsonObject StashItemLocation::toJson() {
    QJsonObject result;
    result.insert("index", _tabIndex);
    result.insert("id", _tabId);
    result.insert("label", _tabLabel);
    result.insert("league", _league);
    result.insert("color", (qint64) _color.rgb());

    // TODO(rory): Save mapping data somehow ???
    result.insert("type", StashItemLocation::TypeToName(_type));

    result.insert("items", ItemLocation::toJson().value("items"));

    return result;
}

bool StashItemLocation::fromJson(const QJsonObject &object) {
    QString type = object.value("type").toString();
    _type = StashItemLocation::NameToType(type);

    // We only care about the items
    return ItemLocation::fromJson(object);
}
