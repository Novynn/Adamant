#include "shopwidget.h"
#include "ui_shopwidget.h"

ShopWidget::ShopWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShopWidget)
{
    ui->setupUi(this);
}

ShopWidget::~ShopWidget()
{
    delete ui;
}
