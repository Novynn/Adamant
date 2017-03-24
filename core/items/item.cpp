#include "item.h"
#include "itemlocation.h"

Item::Item(ItemLocation *parent, QJsonObject data)
    : _parent(parent)
    , _data(data) {
}
