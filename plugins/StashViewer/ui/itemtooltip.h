#ifndef ITEMTOOLTIP_H
#define ITEMTOOLTIP_H

#include <QWidget>
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
    friend class ItemRenderer;
};

#endif // ITEMTOOLTIP_H
