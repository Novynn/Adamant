#include "itemtooltip.h"

ItemTooltip::ItemTooltip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemTooltip)
{
    ui->setupUi(this);
    QFont font = this->font();
    font.setStyleStrategy(QFont::PreferAntialias);
    setFont(font);
}

ItemTooltip::~ItemTooltip()
{
    delete ui;
}
