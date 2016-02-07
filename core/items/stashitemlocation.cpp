#include "stashitemlocation.h"

StashItemLocation::StashItemLocation(const QJsonObject &tabData)
    : ItemLocation() {
    tabIndex = tabData.value("i").toInt();
    tabLabel = tabData.value("n").toString();
    QJsonObject color = tabData.value("colour").toObject();
    _color = QColor::fromRgb(color.value("r").toInt(),
                             color.value("g").toInt(),
                             color.value("b").toInt());
}

ItemLocation::LocationType StashItemLocation::location() const {
    return StashLocation;
}

QString StashItemLocation::header() const {
    return QString("#%1: %2").arg(tabIndex).arg(tabLabel);
}

QString StashItemLocation::forumCode(const QString &league, const Item *item) const {
    Q_UNUSED(item)
    // TODO(rory): implement item location gathering
    QString format("[linkItem location=\"Stash%1\" league=\"%2\" x=\"%3\" y=\"%4\"]");
    return format.arg(tabIndex + 1).arg(league).arg(0).arg(0);
}

QString StashItemLocation::hash() const {
    QString format("stash:%1:%2");
    return format.arg(tabIndex).arg(tabLabel);
}

bool StashItemLocation::operator<(const ItemLocation &other) const {
    if (location() != other.location())
        return location() < other.location();
    return tabIndex < dynamic_cast<const StashItemLocation&>(other).tabIndex;
}

bool StashItemLocation::operator==(const ItemLocation &other) const {
    if (location() != other.location())
        return false;
    return tabIndex == dynamic_cast<const StashItemLocation&>(other).tabIndex;
}

void StashItemLocation::addItems(ItemList items) {
    _items.append(items);
}
