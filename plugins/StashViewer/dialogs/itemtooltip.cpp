#include "itemtooltip.h"

ItemTooltip::ItemTooltip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemTooltip)
{
    ui->setupUi(this);
}

ItemTooltip::~ItemTooltip()
{
    delete ui;
}
