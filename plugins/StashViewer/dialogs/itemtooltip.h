#ifndef ITEMTOOLTIP_H
#define ITEMTOOLTIP_H

#include <QWidget>

#include <items/item.h>
#include "ui_itemtooltip.h"

namespace Ui {
class ItemTooltip;
}

class ItemTooltip : public QWidget
{
    Q_OBJECT

public:
    explicit ItemTooltip(QWidget *parent = 0);
    ~ItemTooltip();

private:
    Ui::ItemTooltip *ui;

    // NEVER DO THIS IN C++ OMG
    // TODO(rory): REMOVE!!!
    friend class GraphicItem;
};

#endif // ITEMTOOLTIP_H
