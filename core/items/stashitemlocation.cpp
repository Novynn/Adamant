#include "stashitemlocation.h"

StashItemLocation::StashItemLocation(const QString &league, const QJsonObject &tabData)
    : ItemLocation()
    , _type(Normal)
    , _updateInterval(0) {
    _tabId = tabData.value("id").toString();
    _league = league;
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

    QString type = tabData.value("type").toString().replace("Stash", "");
    _type = StashItemLocation::NameToType(type);
}

ItemLocation::LocationType StashItemLocation::location() const {
    return StashLocation;
}

QString StashItemLocation::header() const {
    return QString("%1").arg(_tabLabel);
}

QString StashItemLocation::forumCode(const Item &item) const {
    const QString format("[linkItem location=\"Stash%1\" league=\"%2\" x=\"%3\" y=\"%4\"]");
    return format.arg(_tabIndex + 1).arg(_league).arg(item.data("x").toInt()).arg(item.data("y").toInt());
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

void StashItemLocation::setItems(ItemList items, const QJsonValue &layout) {
    switch (_type) {
        case Currency:
        case Fragment: {
            _layout = layout.toObject();
        } break;
        case Essence: {
            if (layout.isArray()) {
                _layout = QJsonObject();
                auto arr = layout.toArray();
                for (int i = 0; i < arr.size(); i++) {
                    _layout.insert(QString::number(i), arr[i].toObject());
                }
            }
            else {
                _layout = layout.toObject();
            }
        } break;
    }

    ItemLocation::setItems(items, layout);
}

double StashItemLocation::scale() const {
    switch (_type) {
        case Currency:  return 63.6f/80.0f;
        case Essence:   return 63.6f/80.0f;
        case Quad:      return 0.5f;
        case Map:       return 71.0f/80.0f;
        case Fragment:
        default:        return 1.0f;
    }
}

QPointF StashItemLocation::itemPos(const Item &item) const {
    QPointF pos = ItemLocation::itemPos(item);
    switch (_type) {
        case Currency:
        case Fragment:
        case Essence: {
            if (!_layout.isEmpty()) {
                const QString key = QString::number((int)pos.x());
                const QJsonObject object = _layout.value(key).toObject();
                pos.setX(object.value("x").toDouble() / 47.4645);
                pos.setY(object.value("y").toDouble() / 47.4645);
            }
        } break;
        case Quad: {
            pos *= 0.5;
        } break;
    }
    return pos;
}

QSizeF StashItemLocation::itemSize(const Item &item) const {
    QPointF pos = ItemLocation::itemPos(item);
    QSizeF size = ItemLocation::itemSize(item);
    const double s = scale();
    switch (_type) {
        case Currency:
        case Fragment:
        case Essence: {
            if (!_layout.isEmpty()) {
                const QString key = QString::number((int)pos.x());
                const QJsonObject object = _layout.value(key).toObject();
                size.setWidth((double)object.value("w").toInt());
                size.setHeight((double)object.value("h").toInt());
            }
        } break;
    }
    size *= s;
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
    result.insert("update_interval", (qint64)_updateInterval);

    // TODO(rory): Save mapping data somehow ???
    result.insert("type", StashItemLocation::TypeToName(_type));

    result.insert("items", ItemLocation::toJson().value("items"));

    return result;
}

bool StashItemLocation::fromJson(const QJsonObject &object) {
    QString type = object.value("type").toString();
    _type = StashItemLocation::NameToType(type);
    _updateInterval = object.value("update_interval").toInt();
    _league = object.value("league").toString();

    // We only care about the items
    return ItemLocation::fromJson(object);
}
